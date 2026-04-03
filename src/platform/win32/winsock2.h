#pragma once

#include "windows.h"

#if defined(__unix__) || defined(__APPLE__)
#include <cerrno>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

using SOCKET = int;
using u_long = unsigned long;

struct WSAData {
    WORD wVersion;
    WORD wHighVersion;
};

using WSADATA = WSAData;

#ifndef MAKEWORD
#define MAKEWORD(a, b) (static_cast<WORD>(((a) & 0xFF) | (((b) & 0xFF) << 8)))
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

#ifndef WSAEWOULDBLOCK
#define WSAEWOULDBLOCK EWOULDBLOCK
#endif

#ifndef WSAEINPROGRESS
#define WSAEINPROGRESS EINPROGRESS
#endif

#ifndef WSAEALREADY
#define WSAEALREADY EALREADY
#endif

inline int WSAStartup(WORD, WSADATA* data)
{
    if (data) {
        data->wVersion = MAKEWORD(2, 2);
        data->wHighVersion = MAKEWORD(2, 2);
    }
    return 0;
}

inline int WSACleanup()
{
    return 0;
}

inline int WSAGetLastError()
{
    return errno;
}

inline int ioctlsocket(SOCKET socketHandle, long command, u_long* value)
{
#if defined(__unix__) || defined(__APPLE__)
    return ::ioctl(socketHandle, command, value);
#else
    (void)socketHandle;
    (void)command;
    (void)value;
    return -1;
#endif
}

inline int closesocket(SOCKET socketHandle)
{
#if defined(__unix__) || defined(__APPLE__)
    return ::close(socketHandle);
#else
    (void)socketHandle;
    return -1;
#endif
}

inline int getsockopt(SOCKET socketHandle, int level, int optionName, char* optionValue, int* optionLength)
{
#if defined(__unix__) || defined(__APPLE__)
    socklen_t nativeLength = optionLength ? static_cast<socklen_t>(*optionLength) : 0;
    const int result = ::getsockopt(socketHandle, level, optionName, optionValue, optionLength ? &nativeLength : nullptr);
    if (optionLength) {
        *optionLength = static_cast<int>(nativeLength);
    }
    return result;
#else
    (void)socketHandle;
    (void)level;
    (void)optionName;
    (void)optionValue;
    (void)optionLength;
    return -1;
#endif
}

