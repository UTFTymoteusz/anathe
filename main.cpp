#include "config.hpp"
#include "dhcp/leaser.hpp"
#include "dhcp/server.hpp"

#include <cstdio>
#include <cstdlib>

Config config;

DHCPServer server = DHCPServer();
DHCPLeaser leaser = DHCPLeaser("static.cfg", "leases");

void handle(dhcp_request& request);

int main() {
    net_init();

    if (server.start() != 0) {
        perror("server.start failed");
        exit(EXIT_FAILURE);
    }

    leaser.print_leases(stdout);
    config.read("dhcp.cfg");

    while (true) {
        auto request_try = server.recv();
        if (!request_try.has_value())
            break;

        handle(request_try.value());
    }

    return 0;
}

void handle(dhcp_request& request) {
    if (config.dns_suffix)
        strncpy(request.domain, config.dns_suffix, sizeof(request.domain));

    if (config.boot_hostname)
        strncpy(request.boot_hostname, config.boot_hostname, sizeof(request.boot_hostname));

    if (config.boot_filename)
        strncpy(request.boot_filename, config.boot_filename, sizeof(request.boot_filename));

    if (request.release) {
        auto rl_addr = request.client_addr;
        leaser.release(request.client_hw, request.client_addr);

        printf("Released address: %i.%i.%i.%i\n", rl_addr[0], rl_addr[1], rl_addr[2], rl_addr[3]);
        return;
    }

    request.dns        = config.dns_servers;
    request.ntp        = config.ntp_servers;
    request.lease_time = config.lease_time;

    auto addr      = leaser.get(request.client_hw);
    bool is_static = addr != ipv4_addr();

    if (!is_static)
        addr = request.client_addr ? request.client_addr
                                   : leaser.get_new(config.dynamic_start, config.dynamic_end);

    printf("Giving address: %i.%i.%i.%i\n", addr[0], addr[1], addr[2], addr[3]);

    if (request.discover) {
        request.client_addr = addr;
        request.mask        = config.mask;
        request.router_addr = config.router_addr;
        request.next_addr   = config.next_addr;

        server.offer(request);
    }
    else if (request.request) {
        if (is_static) {
            if (request.client_addr != addr) {
                server.nack(request);
                return;
            }
        }
        else {
            if (addr < config.dynamic_start || addr > config.dynamic_end) {
                server.nack(request);
                return;
            }
        }

        auto owner = leaser.ownerof(request.client_addr);
        if (owner && owner != request.client_hw) {
            server.nack(request);
            return;
        }

        request.client_addr = addr;
        request.mask        = config.mask;
        request.router_addr = config.router_addr;
        request.next_addr   = config.next_addr;

        if (!is_static)
            leaser.lease(request.client_hw, request.client_addr, config.lease_time);

        server.ack(request);
    }
}