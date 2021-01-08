#pragma once

#include <cstdint>

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

    uint8_t options[];
};