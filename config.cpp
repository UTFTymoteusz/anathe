#include "config.hpp"

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
        if (!(line.find_first_not_of(' ') != std::string::npos))
            continue;

        std::istringstream line_str(line);

        char key[64];
        char value[64];

        line_str >> key;
        line_str >> value;

        if (strcmp(key, "router") == 0) {
            router_addr = parse_ipv4(value);
        }
        else if (strcmp(key, "mask") == 0) {
            mask = parse_ipv4(value);
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
        else
            fprintf(stderr, "config: Unknown option: %s - %s\n", key, value);
    }

    file.close();
}