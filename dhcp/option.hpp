#pragma once

#include <cstdint>
#include <optional>

struct dhcp_option {
    uint8_t type;
    uint8_t len;

    uint8_t data[256];
};

class dhcp_optionreader {
    public:
    dhcp_optionreader(uint8_t* data, int len);

    std::optional<dhcp_option> read();

    private:
    uint8_t* m_data;
    int      m_len;
    int      m_off = 0;

    int nextlen();
};

class dhcp_optionwriter {
    public:
    dhcp_optionwriter(uint8_t* data);

    int write(uint8_t type, uint8_t* buffer, int len);

    template <typename T>
    int write(uint8_t type, T* ptr, int len) {
        return write(type, (uint8_t*) ptr, len);
    }

    template <typename T>
    int write(uint8_t type, T value) {
        return write(type, (uint8_t*) &value, sizeof(value));
    }

    int length();

    private:
    uint8_t* m_data;
    int      m_off = 0;
};