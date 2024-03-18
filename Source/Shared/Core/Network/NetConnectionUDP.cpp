/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Shared/Core/Network/NetConnectionUDP.h"
#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Random.h"
#include "Shared/Core/Utils/DebugObjects.h"
#include "Shared/Core/Crypto/Cipher.h"

#include <cstring>

#ifdef __linux__
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif

namespace {
    static inline constexpr float k_latency_minimum = 0.0f;
    static inline constexpr float k_latency_variance = 0.0f;
    static inline constexpr float k_drop_packet_probability = 0.0f;
    static inline constexpr bool k_emulate_dropped_backs = false;
    static inline constexpr bool k_emulate_latency = false;        
};

NetConnectionUDP::NetConnectionUDP(const std::string& InName)
    : Name(InName)
{
    RecieveBuffer.resize(64 * 1024);
}

NetConnectionUDP::NetConnectionUDP(NetConnectionUDP* InParent, SocketType ParentSocket, sockaddr_in InDestination, const std::string& InName, const NetIPAddress& InAddress)
    : Parent(InParent)
    , Destination(InDestination)
    , Name(InName)
    , bChild(true)
    , Socket(ParentSocket)
    , IPAddress(InAddress)
{
}

NetConnectionUDP::~NetConnectionUDP()
{
    if (Socket != INVALID_SOCKET_VALUE)
    {
        Disconnect();
    }
}

bool NetConnectionUDP::Listen(int Port)
{
    if (Socket != INVALID_SOCKET_VALUE)
    {
        return false;
    }

    Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (Socket == INVALID_SOCKET_VALUE)
    {
        ErrorS(GetName().c_str(), "Failed to create socket, error %i.", WSAGetLastError());
        return false;
    }

    // Allow forcibly reuse of socket port even if something else 
    // is trying to use it, or its not been freed correctly.
    int const_1 = 1;
    if (setsockopt(Socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&const_1, sizeof(const_1)))
    {
        Error(GetName().c_str(), "Failed to set socket options: SO_REUSEADDR");
        return false;        
    }

    // Boost buffer sizes 
    int BufferSize = 16 * 1024 * 1024;
    if (setsockopt(Socket, SOL_SOCKET, SO_RCVBUF, (const char*)&BufferSize, sizeof(BufferSize)))
    {
        Error(GetName().c_str(), "Failed to set socket options: SO_RCVBUF");
        return false;
    }
    if (setsockopt(Socket, SOL_SOCKET, SO_SNDBUF, (const char*)&BufferSize, sizeof(BufferSize)))
    {
        Error(GetName().c_str(), "Failed to set socket options: SO_SNDBUF");
        return false;
    }

    struct sockaddr_in ListenAddress;
    ListenAddress.sin_family = AF_INET;
    ListenAddress.sin_addr.s_addr = INADDR_ANY;
    ListenAddress.sin_port = htons(Port);

    if (bind(Socket, (struct sockaddr*)&ListenAddress, sizeof(ListenAddress)) < 0)
    {
        ErrorS(GetName().c_str(), "Failed to bind socket to port %i.", Port);
        return false;
    }

    bListening = true;
    bShuttingDownThreads = false;
    bErrorOnThreads = false;

    RecieveThread = std::make_unique<std::thread>([&]() {
        RecieveThreadEntry();
    });
    SendThread = std::make_unique<std::thread>([&]() {
        SendThreadEntry();
    });

    return true;
}

std::shared_ptr<NetConnection> NetConnectionUDP::Accept()
{
    if (Socket == INVALID_SOCKET_VALUE)
    {
        return nullptr;
    }

    if (NewConnections.size() > 0)
    {
        std::shared_ptr<NetConnectionUDP> Connection = NewConnections[0];
        NewConnections.erase(NewConnections.begin());
        return Connection;
    }

    return nullptr;
}

NetIPAddress NetConnectionUDP::GetAddress()
{
    return IPAddress;
}

bool NetConnectionUDP::Connect(std::string Hostname, int Port, bool ForceLastIpEntry)
{
    if (Socket != INVALID_SOCKET_VALUE)
    {
        return false;
    }

    Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (Socket == INVALID_SOCKET_VALUE)
    {
        ErrorS(GetName().c_str(), "Failed to create socket, error %i.", WSAGetLastError());
        return false;
    }

    // Allow forcibly reuse of socket port even if something else 
    // is trying to use it, or its not been freed correctly.
    int const_1 = 1;
    if (setsockopt(Socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&const_1, sizeof(const_1)))
    {
        ErrorS(GetName().c_str(), "Failed to set socket options: SO_REUSEADDR");
        return false;
    }

    // Boost buffer sizes 
    int BufferSize = 16 * 1024 * 1024;
    if (setsockopt(Socket, SOL_SOCKET, SO_RCVBUF, (const char*)&BufferSize, sizeof(BufferSize)))
    {
        Error(GetName().c_str(), "Failed to set socket options: SO_RCVBUF");
        return false;
    }
    if (setsockopt(Socket, SOL_SOCKET, SO_SNDBUF, (const char*)&BufferSize, sizeof(BufferSize)))
    {
        Error(GetName().c_str(), "Failed to set socket options: SO_SNDBUF");
        return false;
    }

    struct sockaddr_in ListenAddress;
    ListenAddress.sin_family = AF_INET;
    ListenAddress.sin_addr.s_addr = INADDR_ANY;
    ListenAddress.sin_port = 0;

    if (bind(Socket, (struct sockaddr*)&ListenAddress, sizeof(ListenAddress)) < 0)
    {
        ErrorS(GetName().c_str(), "Failed to bind socket to port %i.", Port);
        return false;
    }

    Destination.sin_port = htons(Port);
    Destination.sin_family = AF_INET;
    memset(Destination.sin_zero, 0, sizeof(Destination.sin_zero));
    inet_pton(AF_INET, Hostname.c_str(), &(Destination.sin_addr));
    
    bShuttingDownThreads = false;
    bErrorOnThreads = false;

    RecieveThread = std::make_unique<std::thread>([&]() {
        RecieveThreadEntry();
    });
    SendThread = std::make_unique<std::thread>([&]() {
        SendThreadEntry();
    });

    return true;
}

bool NetConnectionUDP::Peek(std::vector<uint8_t>& Buffer, int Offset, int Count, int& BytesRecieved)
{
    if (RecieveQueue.size() == 0)
    {
        BytesRecieved = 0;
        return true;
    }

    std::vector<uint8_t>& NextPacket = RecieveQueue[0];
    if (Count > NextPacket.size())
    {
        ErrorS(GetName().c_str(), "Unable to peek udp packet. Peek size is larger than datagram size.");
        return false;
    }

    memcpy(Buffer.data() + Offset, NextPacket.data(), Count);
    BytesRecieved = Count;

    return true;
}

bool NetConnectionUDP::Recieve(std::vector<uint8_t>& Buffer, int Offset, int Count, int& BytesRecieved)
{
    if (RecieveQueue.size() == 0)
    {
        BytesRecieved = 0;
        return true;
    }

    std::vector<uint8_t> NextPacket = RecieveQueue[0];
    if (NextPacket.size() > Count)
    {
        ErrorS(GetName().c_str(), "Unable to recieve next udp packet, packet is larger than buffer. Packets must be recieved in their entirety.");
        return false;
    }
    RecieveQueue.erase(RecieveQueue.begin());

    memcpy(Buffer.data() + Offset, NextPacket.data(), NextPacket.size());
    BytesRecieved = (int)NextPacket.size();

    return true;
}

bool NetConnectionUDP::Send(const std::vector<uint8_t>& Buffer, int Offset, int Count)
{
    NetConnectionUDP* EnqueueConnection = this;
    if (bChild)
    {
        EnqueueConnection = Parent;    
    }

    std::unique_ptr<PendingPacket> Pending = std::make_unique<PendingPacket>();
    Pending->Data.resize(Count);
    memcpy(Pending->Data.data(), Buffer.data() + Offset, Count);
    Pending->SourceAddress = Destination;
    Pending->ProcessTime = 0.0f;

    {
        std::unique_lock lock(EnqueueConnection->SendQueueMutex);
        EnqueueConnection->SendQueue.push(std::move(Pending));
        EnqueueConnection->SendQueueCvar.notify_all();
    }
    
    return true;
}

bool NetConnectionUDP::Disconnect()
{
    if (Socket == INVALID_SOCKET_VALUE)
    {
        return false;
    }

    if (!bChild)
    {
        {
            std::unique_lock lock(SendQueueMutex);
            bShuttingDownThreads = true;
            SendQueueCvar.notify_all();
        }

        if (RecieveThread)
        {
            RecieveThread->join();
            RecieveThread = nullptr;
        }
        if (SendThread)
        {
            SendThread->join();
            SendThread = nullptr;
        }

#ifdef _WIN32
        closesocket(Socket);
#else
        close(Socket);
#endif
    }
    Socket = INVALID_SOCKET_VALUE;
    
    return false;
}

std::string NetConnectionUDP::GetName()
{
    return Name;
}

void NetConnectionUDP::Rename(const std::string& InName)
{
    Name = InName;
}

bool NetConnectionUDP::IsConnected()
{
    // No way of telling with UDP, assume yes.
    return true;
}

void NetConnectionUDP::ProcessPacket(const PendingPacket& Packet)
{
    if (bListening)
    {
        // See if this came from a source we have an existing connection for.
        bool bRoutedPacket = false;
        for (auto ConnectionWeakPtr : ChildConnections)
        {
            if (std::shared_ptr<NetConnectionUDP> Connection = ConnectionWeakPtr.lock())
            {
                if (Connection->Destination.sin_addr.s_addr == Packet.SourceAddress.sin_addr.s_addr &&
                    Connection->Destination.sin_port == Packet.SourceAddress.sin_port)
                {
                    Connection->RecieveQueue.push_back(Packet.Data);
                    bRoutedPacket = true;
                    break;
                }
            }
        }

        // Otherwise create a new connection and use that.
        if (!bRoutedPacket)
        {
            std::vector<char> ClientName;
            ClientName.resize(64);
            snprintf(ClientName.data(), ClientName.size(), "%s:%s:%i", Name.c_str(), inet_ntoa(Packet.SourceAddress.sin_addr), Packet.SourceAddress.sin_port);

#ifdef _WIN32
            NetIPAddress NetClientAddress(
                Packet.SourceAddress.sin_addr.S_un.S_un_b.s_b1,
                Packet.SourceAddress.sin_addr.S_un.S_un_b.s_b2,
                Packet.SourceAddress.sin_addr.S_un.S_un_b.s_b3,
                Packet.SourceAddress.sin_addr.S_un.S_un_b.s_b4);
#else

            NetIPAddress NetClientAddress(
                (Packet.SourceAddress.sin_addr.s_addr) & 0xFF,
                (Packet.SourceAddress.sin_addr.s_addr >> 8) & 0xFF,
                (Packet.SourceAddress.sin_addr.s_addr >> 16) & 0xFF,
                (Packet.SourceAddress.sin_addr.s_addr >> 24) & 0xFF
            );
#endif

            std::shared_ptr<NetConnectionUDP> NewConnection = std::make_shared<NetConnectionUDP>(this, Socket, Packet.SourceAddress, ClientName.data(), NetClientAddress);
            NewConnection->RecieveQueue.push_back(Packet.Data);
            NewConnections.push_back(NewConnection);
            ChildConnections.push_back(NewConnection);
        }
    }
    else
    {
        RecieveQueue.push_back(Packet.Data);
    }
}

void NetConnectionUDP::RecieveThreadEntry()
{
    while (!bShuttingDownThreads)
    {
        // Recieve any pending datagrams and route to the appropriate child recieve queue.
        socklen_t SourceAddressSize = sizeof(struct sockaddr);
        sockaddr_in SourceAddress = { 0 };

        timeval Timeout;
        Timeout.tv_sec = 0;
        Timeout.tv_usec = 1000 * 10;

        fd_set SocketSet;
        FD_ZERO(&SocketSet);
        FD_SET(Socket, &SocketSet);

        // Wait until a message is available, or wakeup after a timeout to check shutdown state 
        // (this is kinda garbage, we should look at a better way to handle this).
        if (select(Socket + 1, &SocketSet, nullptr, nullptr, &Timeout) == 0)
        {
            continue;
        }

        // Recieve the next message on the socket.
        int Flags = 0;
        int Result = recvfrom(Socket, (char*)RecieveBuffer.data(), (int)RecieveBuffer.size(), Flags, (sockaddr*)&SourceAddress, &SourceAddressSize);
        if (Result < 0)
        {
#if defined(_WIN32)
            int error = WSAGetLastError();
#else
            int error = errno;
#endif

            // Blocking is fine, just return.
#if defined(_WIN32)
            if (error == WSAEWOULDBLOCK)
#else        
            if (error == EWOULDBLOCK || error == EAGAIN)
#endif
            {
                continue;
            }

            ErrorS(GetName().c_str(), "Failed to recieve with error 0x%08x.", error);
            // We ignore the error and keep continuing to try and recieve.
        }
        else if (Result > 0)
        {
            std::vector<uint8_t> Packet(RecieveBuffer.data(), RecieveBuffer.data() + Result);

            bool bDropPacket = false;

            if constexpr (k_emulate_dropped_backs)
            {
                if (FRandRange(0.0f, 1.0f) <= k_drop_packet_probability)
                {
                    bDropPacket = true;
                }
            }

            if (!bDropPacket)
            {
                double Latency = k_latency_minimum + FRandRange(-k_latency_variance, k_latency_variance);

                std::unique_ptr<PendingPacket> Pending = std::make_unique<PendingPacket>();
                Pending->Data = Packet;
                Pending->SourceAddress = SourceAddress;
                Pending->ProcessTime = GetSeconds() + (Latency / 1000.0f);

                std::unique_lock lock(PendingPacketsMutex);
                PendingPackets.push(std::move(Pending));
            }

            //Log("<< %zi bytes", (size_t)Result);

            Debug::UdpBytesRecieved.Add(Result);
        }
    }
}

void NetConnectionUDP::SendThreadEntry()
{
    while (!bShuttingDownThreads)
    {
        std::unique_ptr<PendingPacket> SendPacket;

        // Grab next packet to send.
        {
            std::unique_lock lock(SendQueueMutex);
            while (true)
            {
                if (bShuttingDownThreads)
                {
                    return;
                }

                if (!SendQueue.empty())
                {
                    SendPacket = std::move(SendQueue.front());
                    SendQueue.pop();
                    break;
                }
                else
                {
                    SendQueueCvar.wait(lock);
                }
            }
        }

        // Send the packet!
        while (true)
        {
            int Result = sendto(Socket, (char*)SendPacket->Data.data(), SendPacket->Data.size(), 0, (sockaddr*)&SendPacket->SourceAddress, sizeof(sockaddr_in));
            if (Result < 0)
            {
    #if defined(_WIN32)
                int error = WSAGetLastError();
    #else
                int error = errno;
    #endif

                // Blocking is fine, just return.
    #if defined(_WIN32)
                if (error == WSAEWOULDBLOCK)
    #else        
                if (error == EWOULDBLOCK || error == EAGAIN)
    #endif
                {
                    continue;
                }

                ErrorS(GetName().c_str(), "Failed to send with error 0x%08x.", error);
                bErrorOnThreads = true;
                return;
            }
            else if (Result != SendPacket->Data.size())
            {
                ErrorS(GetName().c_str(), "Failed to send packet in its entirety, wanted to send %i but sent %i. Datagram larger than MTU?", SendPacket->Data.size(), Result);
                bErrorOnThreads = true;
                return;
            }

            Debug::UdpBytesSent.Add(SendPacket->Data.size());
            break;
        }

        //Log(">> %zi bytes", SendPacket->Data.size());
    }
}

bool NetConnectionUDP::Pump()
{
    if (Socket == INVALID_SOCKET_VALUE)
    {
        return false;
    }
    
    if (!bChild)
    {
        if (bErrorOnThreads)
        {
            return false;
        }
    }

    // Recieve pending packets.
    {
        std::vector<std::unique_ptr<PendingPacket>> PacketsToProcess;
        
        // Grab all the packets in the recieve queue that currently need processing.
        // Keep this code slim so we don't hold the mutex longer than neccessary (as we don't currently do this lock-free)
        {
            std::unique_lock lock(PendingPacketsMutex);
            while (!PendingPackets.empty())
            {
                bool Process = true;
                PendingPacket* NextPacket = PendingPackets.front().get();                

                if constexpr (k_emulate_latency)
                {
                    Process = (GetSeconds() >= NextPacket->ProcessTime);
                }

                if (Process)
                {
                    PacketsToProcess.push_back(std::move(PendingPackets.front()));
                    PendingPackets.pop();
                }
                else
                {
                    break;
                }
            }
        }

        // Process away.
        for (auto& packet : PacketsToProcess)
        {
            ProcessPacket(*packet);
        }
    }

    // Clear out and children who have gone stale.
    for (auto iter = ChildConnections.begin(); iter != ChildConnections.end(); /*empty*/)
    {
        if ((*iter).expired())
        {
            iter = ChildConnections.erase(iter);
        }
        else
        {
            iter++;
        }
    }

    return false;
}
