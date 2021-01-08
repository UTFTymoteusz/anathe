#pragma once

#include "dhcprequest.hpp"

#include <optional>

struct dhcp_request;

class DHCPServer {
    public:
    DHCPServer();

    int start(int port = 67);

    std::optional<dhcp_request> recv();

    void offer(dhcp_request& request);
    void ack(dhcp_request& request);
    void nack(dhcp_request& request);

    private:
    int       m_sock;
    ipv4_addr m_srv_addr;

    ipv4_addr local_addr();
};