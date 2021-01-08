#pragma once

#if _WIN32
#undef WINVER
#undef _WIN32_WINNT

#define WINVER 0x0600
#define _WIN32_WINNT 0x0600

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