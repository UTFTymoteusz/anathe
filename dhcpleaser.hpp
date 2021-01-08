#pragma once

#include "ipv4.hpp"
#include "mac.hpp"
#include <sys/types.h>

#include <optional>
#include <vector>

struct addr_lease {
    mac_addr  mac;
    ipv4_addr ipv4;

    time_t expire_at;
};

class DHCPLeaser {
    public:
    DHCPLeaser(const char* statics_path, const char* leases_path = "dhcpleases");

    ipv4_addr get(mac_addr mac);
    ipv4_addr get_new_dynamic();

    void lease(mac_addr mac, ipv4_addr ipv4);

    mac_addr ownerof(ipv4_addr ipv4);

    private:
    char* m_statics_path;
    char* m_leases_path;

    std::vector<addr_lease> m_statics;
    std::vector<addr_lease> m_leases;

    void read_statics();
    void read_leases();

    void update_leases();

    ipv4_addr get_static(mac_addr mac);
    ipv4_addr get_leased(mac_addr mac);

    void save_leases();
};