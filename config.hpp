#pragma once

#include "net.hpp"

#include <vector>

class Config {
    public:
    ipv4_addr router_addr;
    ipv4_addr mask;
    ipv4_addr next_addr;

    std::vector<ipv4_addr> dns_servers;
    char*                  dns_suffix;

    std::vector<ipv4_addr> ntp_servers;

    uint32_t lease_time;

    ipv4_addr dynamic_start;
    ipv4_addr dynamic_end;

    char* boot_hostname;
    char* boot_filename;

    void read(const char* path);
};