#pragma once

#include "net.hpp"

#include <algorithm>
#include <optional>

#define BOOT_REQUEST 1
#define BOOT_REPLY 2

#define DHCP_DISCOVER 1
#define DHCP_OFFER 2
#define DHCP_REQUEST 3
#define DHCP_ACK 5
#define DHCP_NACK 6

struct dhcp_packet {
    uint8_t msg_type;

    uint8_t hw_type;
    uint8_t hw_len;

    uint8_t hops;

    big_endian<uint32_t> transct_id;
    big_endian<uint16_t> seconds;

    big_endian<uint16_t> flags;

    ipv4_addr client_own_addr;
    ipv4_addr client_addr;
    ipv4_addr next_addr;
    ipv4_addr relay_addr;

    mac_addr client_hw;
    uint8_t  client_hw_padding[10];

    char server_hostname[64];
    char boot_file[128];

    char cookie[4];
};

struct dhcp_option {
    uint8_t type;
    uint8_t len;

    uint8_t data[256];
};

class dhcp_optionreader {
    public:
    dhcp_optionreader(uint8_t* data, int len) {
        m_data = data;
        m_len  = len;
    }

    std::optional<dhcp_option> read() {
        dhcp_option option;

        int offset = m_off;

        int len = nextlen();
        if (len == 0)
            return {};

        option.type = m_data[offset + 0];
        option.len  = m_data[offset + 1];

        memcpy(&option.data, &m_data[offset + 2], std::min(len, (int) sizeof(option.data)));
        return option;
    }

    private:
    uint8_t* m_data;
    int      m_len;

    int m_off = 0;

    int nextlen() {
        while (m_off < m_len) {
            switch (m_data[m_off]) {
            case 0:
                m_off++;
                break;
            case 255:
                return 0;
            default:
                int len = m_data[m_off + 1] + 2;
                m_off += len;
                return len;
            }
        }

        return 0;
    }
};

class dhcp_optionwriter {
    public:
    dhcp_optionwriter(uint8_t* data) {
        m_data = data;
    }

    int write(uint8_t type, uint8_t* buffer, int len) {
        m_data[m_off + 0] = type;

        if (type == 0 || type == 255) {
            m_off++;
            return 1;
        }

        m_data[m_off + 1] = len;

        memcpy(&m_data[m_off + 2], buffer, len);

        m_off += len + 2;

        return len + 2;
    }

    int length() {
        return m_off;
    }

    private:
    uint8_t* m_data;
    int      m_off = 0;
};