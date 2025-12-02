#include "Network.h"
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

Network::Network() {}
Network::~Network() { Shutdown(); }

bool Network::Init(uint16_t listenPort)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        std::cerr << "WSAStartup failed\n";
        return false;
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET)
    {
        std::cerr << "Failed to create socket\n";
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(listenPort);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed\n";
        return false;
    }

    return true;
}

void Network::Shutdown()
{
    if (sock != INVALID_SOCKET)
    {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
    WSACleanup();
}

bool Network::Send(const void* data, size_t size,
                   const std::string_view& ip, uint16_t port)
{
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.data());

    int sent = sendto(sock,
                      reinterpret_cast<const char*>(data),
                      (int)size,
                      0,
                      (sockaddr*)&addr,
                      sizeof(addr));

    return sent == size;
}

bool Network::Receive(void* buffer, size_t bufferSize, size_t& outSize)
{
    sockaddr_in from{};
    int fromLen = sizeof(from);

    int bytes = recvfrom(sock,
                         reinterpret_cast<char*>(buffer),
                         (int)bufferSize,
                         0,
                         (sockaddr*)&from,
                         &fromLen);

    if (bytes <= 0)
        return false;

    outSize = bytes;

    return true;
}
