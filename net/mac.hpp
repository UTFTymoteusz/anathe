#pragma once

#include "endian.hpp"

#include <stdint.h>
#include <string.h>

struct mac_addr {
    uint8_t bytes[6];

    mac_addr() {}

    mac_addr(const uint8_t mac[6]) {
        memcpy(bytes, mac, 6);
    }

    constexpr mac_addr(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f)
        : bytes{a, b, c, d, e, f} {}

    bool isBroadcast() {
        return bytes[0] & 0x01;
    }

    uint8_t& operator[](int index) {
        return bytes[index];
    }

    bool operator==(const mac_addr& b) {
        return memcmp(bytes, b.bytes, 6) == 0;
    }

    bool operator!=(const mac_addr& b) {
        return memcmp(bytes, b.bytes, 6) != 0;
    }

    operator bool() {
        int sum = 0;
        for (int i = 0; i < sizeof(bytes); i++)
            sum += bytes[i];

        return sum;
    }
} __attribute__((packed));