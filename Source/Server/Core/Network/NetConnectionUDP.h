// Dark Souls 3 - Open Server

#pragma once

#include "Core/Network/NetConnection.h"

#include <stdlib.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#endif

class NetConnectionUDP
    : public NetConnection
{
public:
#if defined(_WIN32)
    using SocketType = SOCKET;
    using SocketLenType = int;
    const SocketType INVALID_SOCKET_VALUE = INVALID_SOCKET;
#else
    using SocketType = int;
    using SocketLenType = socklen_t;
    const SocketType INVALID_SOCKET_VALUE = 0;
#endif

public:
    NetConnectionUDP(SocketType ParentSocket, sockaddr_in DestinationIP, const std::string& InName);
    NetConnectionUDP(const std::string& InName);
    virtual ~NetConnectionUDP();

    virtual bool Listen(int Port) override;

    virtual std::shared_ptr<NetConnection> Accept() override;

    virtual bool Pump() override;

    virtual bool Connect(std::string Hostname, int Port) override;

    virtual bool Peek(std::vector<uint8_t>& Buffer, int Offset, int Count, int& BytesRecieved) override;
    virtual bool Recieve(std::vector<uint8_t>& Buffer, int Offset, int Count, int& BytesRecieved) override;
    virtual bool Send(const std::vector<uint8_t>& Buffer, int Offset, int Count) override;

    virtual bool Disconnect() override;

    virtual bool IsConnected() override;

    virtual std::string GetName() override;

private:
    std::string Name;

    SocketType Socket = INVALID_SOCKET_VALUE;

    bool bListening = false;
    bool bChild = false;

    sockaddr_in Destination = {};

    std::vector<uint8_t> RecieveBuffer;
    std::vector<std::vector<uint8_t>> RecieveQueue;

    std::vector<std::shared_ptr<NetConnectionUDP>> NewConnections;
    std::vector<std::weak_ptr<NetConnectionUDP>> ChildConnections;

};