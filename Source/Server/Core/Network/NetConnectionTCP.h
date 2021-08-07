// Dark Souls 3 - Open Server

#pragma once

#include "Core/Network/NetConnection.h"

#include <stdlib.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#endif

class NetConnectionTCP
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
    NetConnectionTCP(SocketType InSocket, const std::string& InName);
    NetConnectionTCP(const std::string& InName);
    virtual ~NetConnectionTCP();

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

protected:

    bool SendPartial(const std::vector<uint8_t>& Buffer, int Offset, int Count, int& BytesSent);

private:
    std::string Name;

    SocketType Socket = INVALID_SOCKET_VALUE;

    std::vector<uint8_t> SendQueue;

};