#include "dhcp/leaser.hpp"
#include "dhcp/server.hpp"

#include <stdio.h>
#include <stdlib.h>

DHCPServer server = DHCPServer();
DHCPLeaser leaser = DHCPLeaser("static.cfg", "leases");

void handle(dhcp_request& request);

int main() {
    net_init();

    leaser.print_leases(stdout);

    if (server.start() != 0) {
        perror("server.start failed");
        exit(EXIT_FAILURE);
    }

    while (true) {
        auto request_try = server.recv();
        if (!request_try.has_value())
            break;

        handle(request_try.value());
    }

    return 0;
}

void handle(dhcp_request& request) {
    strncpy(request.domain, "menel", sizeof(request.domain));

    request.dns.push_back(ipv4_addr(1, 1, 1, 1));
    request.dns.push_back(ipv4_addr(1, 0, 0, 1));

    request.lease_time = 86400;

    auto addr      = leaser.get(request.client_hw);
    bool is_static = addr != ipv4_addr();

    if (!is_static)
        addr = request.client_addr ? request.client_addr : leaser.get_new();

    printf("Giving address: %i.%i.%i.%i\n", addr[0], addr[1], addr[2], addr[3]);

    if (request.discover) {
        request.client_addr = addr;
        request.mask        = ipv4_addr(255, 255, 255, 0);
        request.router_addr = ipv4_addr(192, 168, 0, 1);

        server.offer(request);
    }
    else if (request.request) {
        if (is_static && request.client_addr != addr) {
            server.nack(request);
            return;
        }

        auto owner = leaser.ownerof(request.client_addr);
        if (owner && owner != request.client_hw) {
            server.nack(request);
            return;
        }

        request.client_addr = addr;
        request.mask        = ipv4_addr(255, 255, 255, 0);
        request.router_addr = ipv4_addr(192, 168, 0, 1);

        if (!is_static)
            leaser.lease(request.client_hw, request.client_addr);

        server.ack(request);
    }
}