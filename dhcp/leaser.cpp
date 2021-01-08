#include "leaser.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>
#include <string>

DHCPLeaser::DHCPLeaser(const char* statics_path, const char* leases_path) {
    m_statics_path = new char[strlen(statics_path) + 1];
    strcpy(m_statics_path, statics_path);

    m_leases_path = new char[strlen(leases_path) + 1];
    strcpy(m_leases_path, leases_path);

    read_statics();
    read_leases();
}

void DHCPLeaser::print_leases(FILE* file) {
    fprintf(file, "Static leases:\n");

    for (auto& lease : m_statics)
        fprintf(file, "  %02x:%02x:%02x:%02x:%02x:%02x - %i.%i.%i.%i\n", lease.mac[0], lease.mac[1],
                lease.mac[2], lease.mac[3], lease.mac[4], lease.mac[5], lease.ipv4[0],
                lease.ipv4[1], lease.ipv4[2], lease.ipv4[3]);

    fprintf(file, "\n");
    fprintf(file, "Dynamic leases:\n");

    for (auto& lease : m_leases)
        fprintf(file, "  %02x:%02x:%02x:%02x:%02x:%02x - %i.%i.%i.%i\n", lease.mac[0], lease.mac[1],
                lease.mac[2], lease.mac[3], lease.mac[4], lease.mac[5], lease.ipv4[0],
                lease.ipv4[1], lease.ipv4[2], lease.ipv4[3]);

    fprintf(file, "\n");
}

ipv4_addr DHCPLeaser::get(mac_addr mac) {
    auto static_addr = get_static(mac);
    if (static_addr)
        return static_addr;

    auto leased_addr = get_leased(mac);
    if (leased_addr)
        return leased_addr;

    return {0, 0, 0, 0};
}

ipv4_addr DHCPLeaser::get_new_dynamic() {
    for (int i = 100; i < 254; i++) {
        auto addr = ipv4_addr(192, 168, 0, i);

        if (!ownerof(addr))
            return addr;
    }

    return {0, 0, 0, 0};
}

void DHCPLeaser::lease(mac_addr mac, ipv4_addr ipv4) {
    for (auto& lease : m_leases) {
        if (lease.mac == mac) {
            lease.ipv4 = ipv4;
            save_leases();

            return;
        }
    }

    m_leases.push_back({
        .mac       = mac,
        .ipv4      = ipv4,
        .expire_at = std::time(0) + 3600,
    });

    save_leases();
}

mac_addr DHCPLeaser::ownerof(ipv4_addr ipv4) {
    read_statics();

    for (auto& static_lease : m_statics) {
        if (static_lease.ipv4 == ipv4)
            return static_lease.mac;
    }

    for (auto& dynamic_lease : m_leases) {
        if (dynamic_lease.ipv4 == ipv4)
            return dynamic_lease.mac;
    }

    return {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
}

ipv4_addr DHCPLeaser::get_static(mac_addr mac) {
    read_statics();

    for (auto& static_lease : m_statics) {
        if (static_lease.mac == mac)
            return static_lease.ipv4;
    }

    return {0, 0, 0, 0};
}

ipv4_addr DHCPLeaser::get_leased(mac_addr mac) {
    update_leases();

    for (auto& dynamic_lease : m_leases) {
        if (dynamic_lease.mac == mac)
            return dynamic_lease.ipv4;
    }

    return {0, 0, 0, 0};
}

void DHCPLeaser::read_statics() {
    std::ifstream file;

    auto read_mac = [](std::istringstream& stream) {
        uint8_t bytes[6];

        for (int i = 0; i < sizeof(bytes); i++) {
            int v;

            stream >> std::hex >> v;
            stream.get();

            bytes[i] = v;

            if (i != sizeof(bytes) - 1 && stream.eof())
                return mac_addr(0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
        }

        return mac_addr(bytes);
    };

    auto read_ipv4 = [](std::istringstream& stream) {
        uint8_t bytes[4];

        for (int i = 0; i < sizeof(bytes); i++) {
            int v;

            stream >> std::dec >> v;
            stream.get();

            bytes[i] = v;

            if (i != sizeof(bytes) - 1 && stream.eof())
                return ipv4_addr(0, 0, 0, 0);
        }

        return ipv4_addr(bytes);
    };

    file.open(m_statics_path);

    m_statics.clear();

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream line_str(line);

        addr_lease static_lease = {
            .mac  = read_mac(line_str),
            .ipv4 = read_ipv4(line_str),
        };

        if (!static_lease.mac)
            continue;

        m_statics.push_back(static_lease);
    }

    file.close();
}

void DHCPLeaser::read_leases() {
    std::ifstream file;

    file.open(m_leases_path);
    m_leases.clear();

    file.seekg(0, std::ios::end);
    int count = file.tellg() / sizeof(addr_lease);

    file.seekg(0, std::ios::beg);

    for (int i = 0; i < count; i++) {
        addr_lease lease;

        file.read((char*) &lease, sizeof(lease));
        m_leases.push_back(lease);
    }

    file.close();
}

void DHCPLeaser::update_leases() {
    bool updated = false;

    for (int i = 0; i < m_leases.size(); i++) {
        auto lease = m_leases[i];

        if (std::time(0) > lease.expire_at) {
            m_leases.erase(m_leases.begin() + i);
            updated = true;
            i--;
        }
    }

    if (updated)
        save_leases();
}

void DHCPLeaser::save_leases() {
    std::ofstream file;

    file.open(m_leases_path);

    for (auto& lease : m_leases)
        file.write((char*) &lease, sizeof(lease));

    file.close();
}