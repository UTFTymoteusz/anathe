#pragma once

#include "net.hpp"

#include <cstdint>
#include <vector>

struct dhcp_request {
    bool discover;
    bool request;

    uint32_t transct_id;

    ipv4_addr              client_addr;
    ipv4_addr              mask;
    ipv4_addr              router_addr;
    std::vector<ipv4_addr> dns;
    std::vector<ipv4_addr> ntp;
    uint32_t               lease_time;

    uint8_t options[64];

    char domain[64];
    char hostname[64];

    sockaddr_in reply_addr;
    mac_addr    client_hw;

    ipv4_addr server_addr;

    bool option(uint8_t type) {
        for (int i = 0; i < sizeof(options); i++)
            if (options[i] == type)
                return true;

        return false;
    }
};