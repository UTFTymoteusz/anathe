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
    uint32_t               lease_time;

    char domain[64];
    char hostname[64];

    sockaddr_in reply_addr;
    mac_addr    client_hw;

    ipv4_addr server_addr;
};