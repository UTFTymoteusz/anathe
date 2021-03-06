#pragma once

#include "endian.hpp"

#include <stdint.h>
#include <string.h>

struct ipv4_addr {
    uint8_t bytes[4];

    ipv4_addr() {
        memset(bytes, 0, 4);
    }

    ipv4_addr(const uint8_t ipv4[4]) {
        memcpy(bytes, ipv4, 4);
    }

    constexpr ipv4_addr(uint8_t a, uint8_t b, uint8_t c, uint8_t d) : bytes{a, b, c, d} {}

    constexpr ipv4_addr(uint32_t v)
        : bytes{(uint8_t)(v >> 24), (uint8_t)(v >> 16), (uint8_t)(v >> 8), (uint8_t)(v >> 0)} {}

    uint8_t& operator[](int index) {
        return bytes[index];
    }

    bool operator==(const ipv4_addr& b) {
        return memcmp(bytes, b.bytes, 4) == 0;
    }

    bool operator!=(const ipv4_addr& b) {
        return memcmp(bytes, b.bytes, 4) != 0;
    }

    bool operator<(const ipv4_addr& b) {
        return memcmp(bytes, b.bytes, 4) < 0;
    }

    bool operator>(const ipv4_addr& b) {
        return memcmp(bytes, b.bytes, 4) > 0;
    }

    bool isSubnettedWith(ipv4_addr destination, ipv4_addr mask) {
        return (*this & mask) == (destination & mask);
    }

    operator uint32_t() {
        return from_big_endian<uint32_t>(*((uint32_t*) bytes));
    }
} __attribute__((packed));

// Make these more efficient pls

inline ipv4_addr operator|(ipv4_addr a, ipv4_addr b) {
    return to_big_endian(*((uint32_t*) a.bytes) | *((uint32_t*) b.bytes));
}

inline ipv4_addr operator&(ipv4_addr a, ipv4_addr b) {
    return to_big_endian(*((uint32_t*) a.bytes) & *((uint32_t*) b.bytes));
}

inline ipv4_addr operator^(ipv4_addr a, ipv4_addr b) {
    return to_big_endian(*((uint32_t*) a.bytes) ^ *((uint32_t*) b.bytes));
}

inline ipv4_addr operator~(ipv4_addr a) {
    return to_big_endian(~(*((uint32_t*) a.bytes)));
}