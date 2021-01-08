#pragma once

#if _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif

#include "net/endian.hpp"
#include "net/ipv4.hpp"
#include "net/mac.hpp"

void net_init();