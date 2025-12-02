#pragma once
#include <string>
#include <winsock2.h>

class Network
{
public:
    Network();
    ~Network();

    bool Init(uint16_t listenPort);
    void Shutdown();

    bool Send(const void* data, size_t size,
              const std::string_view& ip, uint16_t port);

    bool Receive(void* buffer, size_t bufferSize, size_t& outSize);

private:
    SOCKET sock = INVALID_SOCKET;
};

enum class PlayerId
{
    Invalid,
    Player1,
    Player2
};

inline uint16_t GetPortForPlayer(PlayerId player)
{
    return player == PlayerId::Player1 ? 5000 : 5001;
}
