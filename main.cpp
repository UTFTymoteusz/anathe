#include "dhcp/leaser.hpp"
#include "dhcp/server.hpp"

#include <stdio.h>
#include <stdlib.h>

int main() {
#if _WIN32
    WSADATA wsaData;

    int iResult;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        exit(EXIT_FAILURE);
    }
#endif

    DHCPServer server = DHCPServer();
    DHCPLeaser leaser = DHCPLeaser("static.cfg", "leases");

    leaser.print_leases(stdout);

    if (server.start() != 0) {
        perror("server.start failed");
        exit(EXIT_FAILURE);
    }

    while (true) {
        auto request_try = server.recv();
        if (!request_try.has_value())
            break;

        auto request = request_try.value();

        strncpy(request.domain, "menel", sizeof(request.domain));

        request.dns.push_back(ipv4_addr(1, 1, 1, 1));
        request.dns.push_back(ipv4_addr(1, 0, 0, 1));

        request.lease_time = 86400;

        bool static_addr = false;

        auto addr   = leaser.get(request.client_hw);
        static_addr = addr != ipv4_addr();

        if (!static_addr)
            addr = request.client_addr ? request.client_addr : leaser.get_new_dynamic();

        printf("Giving address: %i.%i.%i.%i\n", addr[0], addr[1], addr[2], addr[3]);

        if (request.discover) {
            request.client_addr = addr;
            request.mask        = ipv4_addr(255, 255, 255, 0);
            request.router_addr = ipv4_addr(192, 168, 0, 1);

            server.offer(request);
        }
        else if (request.request) {
            if (static_addr && request.client_addr != addr) {
                server.nack(request);
                continue;
            }

            auto owner = leaser.ownerof(request.client_addr);

            if (owner && owner != request.client_hw) {
                server.nack(request);
                continue;
            }

            request.client_addr = addr;
            request.mask        = ipv4_addr(255, 255, 255, 0);
            request.router_addr = ipv4_addr(192, 168, 0, 1);

            if (!static_addr)
                leaser.lease(request.client_hw, request.client_addr);

            server.ack(request);
        }
    }

    return 0;
}