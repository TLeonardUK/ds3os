/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Core/Network/NetConnectionUDP.h"
#include "Core/Utils/Logging.h"
#include "Config/BuildConfig.h"
#include "Core/Crypto/Cipher.h"

NetConnectionUDP::NetConnectionUDP(const std::string& InName)
    : Name(InName)
{
    RecieveBuffer.resize(64 * 1024);
}

NetConnectionUDP::NetConnectionUDP(SocketType ParentSocket, sockaddr_in InDestination, const std::string& InName)
    : Destination(InDestination)
    , Name(InName)
    , bChild(true)
    , Socket(ParentSocket)
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
        Error("[%s] Failed to create socket, error %i.", GetName().c_str(), WSAGetLastError());
        return false;
    }

    // Allow forcibly reuse of socket port even if something else 
    // is trying to use it, or its not been freed correctly.
    int const_1 = 1;
    if (setsockopt(Socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&const_1, sizeof(const_1)))
    {
        Error("[%s] Failed to set socket options: SO_REUSEADDR", GetName().c_str());
        return false;        
    }

    // Set socket to non-blocking mode.
#if defined(_WIN32)
    unsigned long mode = 1;
    if (int result = ioctlsocket(Socket, FIONBIO, &mode); result != 0)
    {
        Error("[%s] Failed to set socket to non blocking with error 0x%08x", GetName().c_str(), result);
        return false;
    }
#else
    if (int flags = fcntl(Socket, F_GETFL, 0); flags == -1)
    {
        Error("[%s] Failed to get socket flags.", GetName().c_str());
        return false;
    }
    flags = flags | O_NONBLOCK;
    if (int result = fcntl(Socket, F_SETFL, flags); result != 0)
    {
        Error("[%s] Failed to set socket to non blocking with error 0x%08x", GetName().c_str(), result);
        return false;
    }
#endif

    struct sockaddr_in ListenAddress;
    ListenAddress.sin_family = AF_INET;
    ListenAddress.sin_addr.s_addr = INADDR_ANY;
    ListenAddress.sin_port = htons(Port);

    if (bind(Socket, (struct sockaddr*)&ListenAddress, sizeof(ListenAddress)) < 0)
    {
        Error("[%s] Failed to bind socket to port %i.", GetName().c_str(), Port);
        return false;
    }

    bListening = true;

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

bool NetConnectionUDP::Connect(std::string Hostname, int Port)
{
    Destination.sin_port = htons(Port);
    Destination.sin_family = AF_INET;
    memset(Destination.sin_zero, 0, sizeof(Destination.sin_zero));
    inet_pton(AF_INET, Hostname.c_str(), &(Destination.sin_addr));

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
        Error("[%s] Unable to peek udp packet. Peek size is larger than datagram size.", GetName().c_str());
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
        Error("[%s] Unable to recieve next udp packet, packet is larger than buffer. Packets must be recieved in their entirety.", GetName().c_str());
        return false;
    }
    RecieveQueue.erase(RecieveQueue.begin());

    memcpy(Buffer.data() + Offset, NextPacket.data(), NextPacket.size());
    BytesRecieved = (int)NextPacket.size();

    return true;
}

bool NetConnectionUDP::Send(const std::vector<uint8_t>& Buffer, int Offset, int Count)
{
    int Result = sendto(Socket, (char*)Buffer.data() + Offset, Count, 0, (sockaddr*)&Destination, sizeof(sockaddr_in));
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
            return false;
        }

        Error("[%s] Failed to send with error 0x%08x.", GetName().c_str(), error);
        return false;
    }
    else if (Result != Count)
    {
        Error("[%s] Failed to send packet in its entirety, wanted to send %i but sent %i. Datagram larger than MTU?", GetName().c_str(), Count, Result);
        return false;
    }

   // Log(">> %i", Result);

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
        closesocket(Socket);
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

bool NetConnectionUDP::Pump()
{
    if (Socket == INVALID_SOCKET_VALUE)
    {
        return false;
    }
    
    if (!bChild)
    {
        // Recieve any pending datagrams and route to the appropriate child recieve queue.

        socklen_t SourceAddressSize = sizeof(struct sockaddr);
        sockaddr_in SourceAddress = { 0 };

        int Result = recvfrom(Socket, (char*)RecieveBuffer.data(), (int)RecieveBuffer.size(), 0, (sockaddr*)&SourceAddress, &SourceAddressSize);
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
                return false;
            }

            Error("[%s] Failed to recieve with error 0x%08x.", GetName().c_str(), error);
            return false;
        }
        else if (Result > 0)
        {
            std::vector<uint8_t> Packet(RecieveBuffer.data(), RecieveBuffer.data() + Result);

            if (bListening)
            {
                // See if this came from a source we have an existing connection for.
                bool bRoutedPacket = false;
                for (auto ConnectionWeakPtr : ChildConnections)
                {
                    if (std::shared_ptr<NetConnectionUDP> Connection = ConnectionWeakPtr.lock())
                    {
                        if (Connection->Destination.sin_addr.S_un.S_addr == SourceAddress.sin_addr.S_un.S_addr &&
                            Connection->Destination.sin_port == SourceAddress.sin_port)
                        {
                            Connection->RecieveQueue.push_back(Packet);
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
                    snprintf(ClientName.data(), ClientName.size(), "%s{%s:%i}", Name.c_str(), inet_ntoa(SourceAddress.sin_addr), SourceAddress.sin_port);

                    std::shared_ptr<NetConnectionUDP> NewConnection = std::make_shared<NetConnectionUDP>(Socket, SourceAddress, ClientName.data());
                    NewConnection->RecieveQueue.push_back(Packet);
                    NewConnections.push_back(NewConnection);
                    ChildConnections.push_back(NewConnection);
                }
            }
            else
            {
                RecieveQueue.push_back(Packet);
            }

            //Log("<< %i", Result);
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
