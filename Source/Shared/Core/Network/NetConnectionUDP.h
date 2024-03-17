/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Shared/Core/Network/NetConnection.h"

#if defined(_WIN32)
#include <windows.h>
#include <ws2tcpip.h>
#include <winsock2.h>
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
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

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
    const SocketType INVALID_SOCKET_VALUE = -1;
#endif

public:
    NetConnectionUDP(NetConnectionUDP* Parent, SocketType ParentSocket, sockaddr_in DestinationIP, const std::string& InName, const NetIPAddress& InAddress);
    NetConnectionUDP(const std::string& InName);
    virtual ~NetConnectionUDP();

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
    struct PendingPacket
    {
        std::vector<uint8_t> Data;
        sockaddr_in SourceAddress;
        double ProcessTime = 0.0f;
    };

    void ProcessPacket(const PendingPacket& Packet);

    void RecieveThreadEntry();
    void SendThreadEntry();

private:

    std::string Name;
    NetIPAddress IPAddress;

    NetConnectionUDP* Parent = nullptr;
    SocketType Socket = INVALID_SOCKET_VALUE;

    bool bListening = false;
    bool bChild = false;

    sockaddr_in Destination = {};

    std::vector<uint8_t> RecieveBuffer;
    std::vector<std::vector<uint8_t>> RecieveQueue;

    std::vector<std::shared_ptr<NetConnectionUDP>> NewConnections;
    std::vector<std::weak_ptr<NetConnectionUDP>> ChildConnections;

    std::mutex PendingPacketsMutex;
    std::queue<std::unique_ptr<PendingPacket>> PendingPackets;

    std::queue<std::unique_ptr<PendingPacket>> SendQueue;
    std::mutex SendQueueMutex;
    std::condition_variable SendQueueCvar;

    std::unique_ptr<std::thread> RecieveThread;
    std::unique_ptr<std::thread> SendThread;
    bool bShuttingDownThreads = false;
    bool bErrorOnThreads = false;

};