#include "config.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

void Config::read(const char* path) {
    auto parse_ipv4 = [](const char* str) {
        struct sockaddr_in sa;
        inet_pton(AF_INET, str, &sa.sin_addr);

        return ipv4_addr(ntohl(sa.sin_addr.s_addr));
    };

    std::ifstream file;
    file.open(path);

    if (!file.good()) {
        fprintf(stderr, "config: Failed to open %s\n", path);
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Gotta check if the line is whitespace
        if (std::all_of(line.begin(), line.end(), isspace))
            continue;

        std::istringstream line_str(line);

        char key[64];
        char value[256];

        line_str >> key;
        line_str >> value;

        if (strcmp(key, "router") == 0) {
            router_addr = parse_ipv4(value);
        }
        else if (strcmp(key, "mask") == 0) {
            mask = parse_ipv4(value);
        }
        else if (strcmp(key, "next") == 0) {
            next_addr = parse_ipv4(value);
        }
        else if (strcmp(key, "dnssuffix") == 0) {
            int len    = strlen(value);
            dns_suffix = new char[len + 1];

            strncpy(dns_suffix, value, len + 1);
        }
        else if (strcmp(key, "dnsserver") == 0) {
            dns_servers.push_back(parse_ipv4(value));
        }
        else if (strcmp(key, "dynamicstart") == 0) {
            dynamic_start = parse_ipv4(value);
        }
        else if (strcmp(key, "dynamicend") == 0) {
            dynamic_end = parse_ipv4(value);
        }
        else if (strcmp(key, "leasetime") == 0) {
            lease_time = atoi(value);
        }
        else if (strcmp(key, "ntpserver") == 0) {
            ntp_servers.push_back(parse_ipv4(value));
        }
        else if (strcmp(key, "boothostname") == 0) {
            int len       = strlen(value);
            boot_hostname = new char[len + 1];

            strncpy(boot_hostname, value, len + 1);
        }
        else if (strcmp(key, "bootfilename") == 0) {
            int len       = strlen(value);
            boot_filename = new char[len + 1];

            strncpy(boot_filename, value, len + 1);
        }
        else {
            fprintf(stderr, "config: Unknown option: %s - %s\n", key, value);
        }
    }

    file.close();
}