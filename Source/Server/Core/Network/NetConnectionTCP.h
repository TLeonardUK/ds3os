/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Core/Network/NetConnection.h"

#include <stdlib.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <fcntl.h>

#define SOCKET_ERROR (-1)
#endif

#include <stdlib.h>

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
    const SocketType INVALID_SOCKET_VALUE = -1;
#endif

public:
    NetConnectionTCP(SocketType InSocket, const std::string& InName, const NetIPAddress& InAddress);
    NetConnectionTCP(const std::string& InName);
    virtual ~NetConnectionTCP();

    virtual bool Listen(int Port) override;

    virtual std::shared_ptr<NetConnection> Accept() override;

    virtual bool Pump() override;

    virtual bool Connect(std::string Hostname, int Port, bool ForceLastIpEntry) override;

    virtual bool Peek(std::vector<uint8_t>& Buffer, int Offset, int Count, int& BytesRecieved) override;
    virtual bool Recieve(std::vector<uint8_t>& Buffer, int Offset, int Count, int& BytesRecieved) override;
    virtual bool Send(const std::vector<uint8_t>& Buffer, int Offset, int Count) override;

    virtual bool Disconnect() override;

    virtual bool IsConnected() override;

    virtual NetIPAddress GetAddress() override;

    virtual std::string GetName() override;
    virtual void Rename(const std::string& Name) override;

protected:

    bool SendPartial(const std::vector<uint8_t>& Buffer, int Offset, int Count, int& BytesSent);

private:
    std::string Name;
    NetIPAddress IPAddress;

    bool HasDisconnected = false;

    SocketType Socket = INVALID_SOCKET_VALUE;

    std::vector<uint8_t> SendQueue;

};