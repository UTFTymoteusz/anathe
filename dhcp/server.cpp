#include "server.hpp"

#include "net.hpp"
#include "option.hpp"
#include "packet.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <unistd.h>

DHCPServer::DHCPServer() {}

int DHCPServer::start(int port) {
    sockaddr_in srv_addr_in;

    m_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_sock == -1) {
        perror("socket() failed");
        return -1;
    }

    int be = 1;
    setsockopt(m_sock, SOL_SOCKET, SO_BROADCAST, (char*) &be, sizeof(be));

    srv_addr_in.sin_family      = AF_INET;
    srv_addr_in.sin_addr.s_addr = INADDR_ANY;
    srv_addr_in.sin_port        = htons(port);

    int bind_try = bind(m_sock, (const sockaddr*) &srv_addr_in, sizeof(srv_addr_in));
    if (bind_try != 0) {
        perror("bind() failed");
        return bind_try;
    }

    m_srv_addr = local_addr();
    printf("  Running from %i.%i.%i.%i\n", m_srv_addr[0], m_srv_addr[1], m_srv_addr[2],
           m_srv_addr[3]);

    return 0;
}

std::optional<dhcp_request> DHCPServer::recv() {
    struct sockaddr_in cln_addr;

    uint8_t   buffer[2048];
    socklen_t addr_len = sizeof(cln_addr);

    while (true) {
        int len =
            recvfrom(m_sock, (char*) buffer, sizeof(buffer), 0, (sockaddr*) &cln_addr, &addr_len);
        if (len == 0)
            break;

        auto* packet = (dhcp_packet*) buffer;
        if (packet->msg_type != BOOT_REQUEST)
            continue;

        auto reader = dhcp_optionreader(&buffer[sizeof(dhcp_packet)], len - sizeof(dhcp_packet));

        auto option = reader.read();
        if (!option.has_value())
            continue;

        if (option->type != 53)
            continue;

        if (option->data[0] != DHCP_DISCOVER && option->data[0] != DHCP_REQUEST)
            continue;

        if (cln_addr.sin_addr.s_addr == INADDR_ANY)
            cln_addr.sin_addr.s_addr = INADDR_BROADCAST;

        printf("%02x:%02x:%02x:%02x:%02x:%02x: %s\n", packet->client_hw[0], packet->client_hw[1],
               packet->client_hw[2], packet->client_hw[3], packet->client_hw[4],
               packet->client_hw[5],
               option->data[0] == DHCP_DISCOVER ? "DHCP_DISCOVER" : "DHCP_REQUEST");

        dhcp_request request = {};

        request.discover = option->data[0] == DHCP_DISCOVER;
        request.request  = option->data[0] == DHCP_REQUEST;

        request.transct_id  = packet->transct_id;
        request.source_addr = cln_addr;
        request.server_addr = m_srv_addr;
        request.client_addr = packet->client_own_addr;
        request.client_hw   = packet->client_hw;

        read_options(request, reader);

        return request;
    }

    return {};
}

void prepare(dhcp_packet* packet, dhcp_request& request) {
    packet->msg_type = BOOT_REPLY;

    packet->hw_type = 0x01;
    packet->hw_len  = 6;

    packet->hops = 0;

    packet->transct_id = request.transct_id;
    packet->seconds    = 0;
    packet->flags      = 0x8000;

    packet->client_own_addr = {};
    packet->client_addr     = request.client_addr;
    packet->next_addr       = request.next_addr;
    packet->relay_addr      = {};

    packet->client_hw = request.client_hw;
    memset(packet->client_hw_padding, 0, sizeof(packet->client_hw_padding));

    strncpy(packet->server_hostname, request.boot_hostname, sizeof(packet->server_hostname));
    strncpy(packet->boot_file, request.boot_filename, sizeof(packet->boot_file));

    packet->cookie[0] = 0x63;
    packet->cookie[1] = 0x82;
    packet->cookie[2] = 0x53;
    packet->cookie[3] = 0x63;
}

void write_options(dhcp_optionwriter& writer, dhcp_request& request) {
    if (request.option(3) && request.router_addr)
        writer.write(3, request.router_addr);

    if (request.option(1) && request.mask)
        writer.write(1, request.mask);

    if (request.option(6) && !request.dns.empty())
        writer.write(6, request.dns.data(), request.dns.size() * sizeof(ipv4_addr));

    if (request.option(42) && !request.ntp.empty())
        writer.write(42, request.ntp.data(), request.ntp.size() * sizeof(ipv4_addr));

    if (request.option(15) && *request.domain)
        writer.write(15, request.domain, strlen(request.domain));

    if (request.option(12) && *request.hostname)
        writer.write(12, request.hostname, strlen(request.hostname));

    if (request.option(28) && request.router_addr && request.mask)
        writer.write(28, request.router_addr | ~request.mask);

    if (request.server_addr)
        writer.write(54, request.server_addr);

    if (request.lease_time)
        writer.write(51, htonl(request.lease_time));
}

void DHCPServer::offer(dhcp_request& request) {
    uint8_t buffer[1024];
    auto    packet = (dhcp_packet*) &buffer;

    prepare(packet, request);

    auto writer = dhcp_optionwriter(packet->options);

    writer.write<uint8_t>(53, DHCP_OFFER);
    write_options(writer, request);
    writer.write(255);

    sendto(m_sock, (char*) buffer, sizeof(dhcp_packet) + writer.length(), 0,
           (const sockaddr*) &request.source_addr, sizeof(request.source_addr));
}

void DHCPServer::ack(dhcp_request& request) {
    uint8_t buffer[1024];
    auto    packet = (dhcp_packet*) &buffer;

    prepare(packet, request);

    auto writer = dhcp_optionwriter(packet->options);

    writer.write<uint8_t>(53, DHCP_ACK);
    write_options(writer, request);
    writer.write(255);

    sendto(m_sock, (char*) buffer, sizeof(dhcp_packet) + writer.length(), 0,
           (const sockaddr*) &request.source_addr, sizeof(request.source_addr));
}

void DHCPServer::nack(dhcp_request& request) {
    uint8_t buffer[1024];
    auto    packet = (dhcp_packet*) &buffer;

    prepare(packet, request);

    auto writer = dhcp_optionwriter(packet->options);

    writer.write<uint8_t>(53, DHCP_NACK);
    writer.write(255);

    sendto(m_sock, (char*) buffer, sizeof(dhcp_packet) + writer.length(), 0,
           (const sockaddr*) &request.source_addr, sizeof(request.source_addr));
}

// privatebongs
void DHCPServer::read_options(dhcp_request& request, dhcp_optionreader& reader) {
    while (true) {
        auto option = reader.read();
        if (!option.has_value())
            break;

        switch (option->type) {
        case 61: {
            printf("Option: (61) Client identifier\n");
        } break;
        case 50: {
            printf("Option: (50) Requested IP Address\n");

            auto addr           = (ipv4_addr) option->data;
            request.client_addr = addr;

            printf("  Requested IP Address: %i.%i.%i.%i\n", addr[0], addr[1], addr[2], addr[3]);
        } break;
        case 55: {
            printf("Option: (55) Parameter Request List\n");

            memcpy(request.options, option->data, option->len);
        } break;
        case 12: {
            char buffer[128];

            strncpy(request.hostname, (const char*) option->data,
                    std::min((size_t) option->len, sizeof(buffer)));

            printf("Option: (12) Hostname\n");
            printf("  Hostname: %s\n", request.hostname);
        } break;
        default: {
            printf("Option: (%i) Unknown\n", option->type);
        } break;
        }
    }
}

ipv4_addr DHCPServer::local_addr() {
    int addr_sock = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in addr;
    socklen_t   addr_len = sizeof(addr);

    addr.sin_family = AF_INET;
    addr.sin_port   = htons(76);

    inet_pton(AF_INET, "2.1.3.7", &addr.sin_addr);

    connect(addr_sock, (sockaddr*) &addr, sizeof(addr));
    getsockname(addr_sock, (sockaddr*) &addr, &addr_len);

    close(addr_sock);

    return ipv4_addr(ntohl(addr.sin_addr.s_addr));
}