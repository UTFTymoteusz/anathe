#include "packet.hpp"

const char* names[7] = {
    "DHCP_DISCOVER", "DHCP_OFFER", "DHCP_REQUEST", "DHCP_DECLINE",
    "DHCP_ACK",      "DHCP_NACK",  "DHCP_RELEASE",
};

const char* strdhcpmsg(int msgtype) {
    if (msgtype < DHCP_DISCOVER || msgtype > DHCP_RELEASE)
        return "UNKNOWN";

    return names[msgtype - 1];
}