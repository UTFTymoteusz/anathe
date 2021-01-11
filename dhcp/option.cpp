#include "option.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>

dhcp_optionreader::dhcp_optionreader(uint8_t* data, int len) {
    m_data = data;
    m_len  = len;
}

std::optional<dhcp_option> dhcp_optionreader::read() {
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

int dhcp_optionreader::nextlen() {
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

dhcp_optionwriter::dhcp_optionwriter(uint8_t* data) {
    m_data = data;
}

int dhcp_optionwriter::write(uint8_t type) {
    m_data[m_off++] = type;
    return 1;
}

int dhcp_optionwriter::write(uint8_t type, uint8_t* buffer, int len) {
    if (type == 0 || type == 255) {
        return write(type);
    }

    m_data[m_off + 0] = type;
    m_data[m_off + 1] = len;

    memcpy(&m_data[m_off + 2], buffer, len);

    m_off += len + 2;

    return len + 2;
}

int dhcp_optionwriter::length() {
    return m_off;
}