#pragma once

#include "net.hpp"

#include <vector>

class Config {
    public:
    ipv4_addr router_addr;
    ipv4_addr mask;

    std::vector<ipv4_addr> dns_servers;
    char*                  dns_suffix;

    uint32_t lease_time;

    ipv4_addr dynamic_start;
    ipv4_addr dynamic_end;

    void read(const char* path);
};