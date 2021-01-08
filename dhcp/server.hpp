#pragma once

#include "net.hpp"
#include "request.hpp"

#include <optional>

struct dhcp_request;
class dhcp_optionreader;

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

    void      read_options(dhcp_request& request, dhcp_optionreader& reader);
    ipv4_addr local_addr();
};