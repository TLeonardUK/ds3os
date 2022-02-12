/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Core/Network/NetConnectionTCP.h"
#include "Core/Utils/Logging.h"
#include "Core/Utils/DebugObjects.h"
#include "Config/BuildConfig.h"
#include "Core/Crypto/Cipher.h"

NetConnectionTCP::NetConnectionTCP(const std::string& InName)
    : Name(InName)
{
}

NetConnectionTCP::NetConnectionTCP(SocketType InSocket, const std::string& InName, const NetIPAddress& InAddress)
    : Socket(InSocket)
    , Name(InName)
    , IPAddress(InAddress)
{
}

NetConnectionTCP::~NetConnectionTCP()
{
    if (Socket != INVALID_SOCKET_VALUE)
    {
        Disconnect();
    }
}

bool NetConnectionTCP::Listen(int Port)
{
    if (Socket != INVALID_SOCKET_VALUE)
    {
        return false;
    }

    Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (Socket == INVALID_SOCKET_VALUE)
    {
        ErrorS(GetName().c_str(), "Failed to create socket, error %i.",  WSAGetLastError());
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

    // Turn off nagles algorithm.
    if (setsockopt(Socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&const_1, sizeof(const_1)))
    {
        ErrorS(GetName().c_str(), "Failed to set socket options: TCP_NODELAY");
        return false;
    }

    // Set socket to non-blocking mode.
#if defined(_WIN32)
    unsigned long mode = 1;
    if (int result = ioctlsocket(Socket, FIONBIO, &mode); result != 0)
    {
        ErrorS(GetName().c_str(), "Failed to set socket to non blocking with error 0x%08x", result);
        return false;
    }
#else
    if (int flags = fcntl(Socket, F_GETFL, 0); flags == -1)
    {
        ErrorS(GetName().c_str(), "Failed to get socket flags.");
        return false;
    }
    flags = flags | O_NONBLOCK;
    if (int result = fcntl(Socket, F_SETFL, flags); result != 0)
    {
        ErrorS(GetName().c_str(), "Failed to set socket to non blocking with error 0x%08x", result);
        return false;
    }
#endif

    struct sockaddr_in ListenAddress;
    ListenAddress.sin_family = AF_INET;
    ListenAddress.sin_addr.s_addr = INADDR_ANY;
    ListenAddress.sin_port = htons(Port);

    if (bind(Socket, (struct sockaddr*)&ListenAddress, sizeof(ListenAddress)) < 0)
    {
        ErrorS(GetName().c_str(), "Failed to bind socket to port %i.", Port);
        return false;
    }

    if (listen(Socket, 64) < 0)
    {
        ErrorS(GetName().c_str(), "Failed to listen on socket on port %i.", Port);
        return false;
    }

    return true;
}

std::shared_ptr<NetConnection> NetConnectionTCP::Accept()
{
    if (Socket == INVALID_SOCKET_VALUE)
    {
        return nullptr;
    }

    struct sockaddr_in ClientAddress;
    int AddressLength = sizeof(ClientAddress);

    SocketType NewSocket = accept(Socket, (struct sockaddr*)&ClientAddress, (SocketLenType*)&AddressLength);
    if (NewSocket != INVALID_SOCKET_VALUE)
    {
        std::vector<char> ClientName;
        ClientName.resize(64);
        snprintf(ClientName.data(), ClientName.size(), "%s:%s:%i", Name.c_str(), inet_ntoa(ClientAddress.sin_addr), ClientAddress.sin_port);

        // TODO: Keep track of these clients and disconnect them when 
        //       this socket is disconnected.

        NetIPAddress NetClientAddress(
            ClientAddress.sin_addr.S_un.S_un_b.s_b1, 
            ClientAddress.sin_addr.S_un.S_un_b.s_b2, 
            ClientAddress.sin_addr.S_un.S_un_b.s_b3, 
            ClientAddress.sin_addr.S_un.S_un_b.s_b4);

        return std::make_shared<NetConnectionTCP>(NewSocket, ClientName.data(), NetClientAddress);
    }

    return nullptr;
}

NetIPAddress NetConnectionTCP::GetAddress()
{
    return IPAddress;
}

bool NetConnectionTCP::Connect(std::string Hostname, int Port, bool ForceLastIpEntry)
{
    if (Socket != INVALID_SOCKET_VALUE)
    {
        return false;
    }

    Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (Socket == INVALID_SOCKET_VALUE)
    {
        ErrorS(GetName().c_str(), "Failed to create socket, error %i.", WSAGetLastError());
        return false;
    }

    sockaddr_in SockAddr;
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_port = htons(Port);

    hostent* HostEntry = gethostbyname(Hostname.c_str());
    if (HostEntry == nullptr)
    {
        ErrorS(GetName().c_str(), "Failed to resolve hostname '%s'.", Hostname.c_str());
        return false;
    }
    else
    {
        Ensure(HostEntry->h_addrtype == AF_INET); // Only support ipv4 right now.

        if (ForceLastIpEntry)
        {
            for (int i = 0; HostEntry->h_addr_list[i] != nullptr; i++)
            {
                SockAddr.sin_addr.s_addr = *(u_long*)HostEntry->h_addr_list[i];
            }
        }
        else
        {
            SockAddr.sin_addr.s_addr = *(u_long*)HostEntry->h_addr_list[0];
        }
    }

    int Result = connect(Socket, (SOCKADDR*)&SockAddr, sizeof(SockAddr));
    if (Result == SOCKET_ERROR)
    {
#if defined(_WIN32)
        int error = WSAGetLastError();
#else
        int error = errno;
#endif

        ErrorS(GetName().c_str(), "Failed to recieve with error 0x%08x.", error);
        return false;
    }

    // Turn off nagles algorithm.
    unsigned int const_1 = 1;
    if (setsockopt(Socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&const_1, sizeof(const_1)))
    {
        ErrorS(GetName().c_str(), "Failed to set socket options: TCP_NODELAY");
        return false;
    }

    // Set socket to non-blocking mode.
#if defined(_WIN32)
    unsigned long mode = 1;
    if (int result = ioctlsocket(Socket, FIONBIO, &mode); result != 0)
    {
        ErrorS(GetName().c_str(), "Failed to set socket to non blocking with error 0x%08x", result);
        return false;
    }
#else
    if (int flags = fcntl(Socket, F_GETFL, 0); flags == -1)
    {
        ErrorS(GetName().c_str(), "Failed to get socket flags.");
        return false;
    }
    flags = flags | O_NONBLOCK;
    if (int result = fcntl(Socket, F_SETFL, flags); result != 0)
    {
        ErrorS(GetName().c_str(), "Failed to set socket to non blocking with error 0x%08x", result);
        return false;
    }
#endif

    return true;
}

bool NetConnectionTCP::Peek(std::vector<uint8_t>& Buffer, int Offset, int Count, int& BytesRecieved)
{
    // TODO: We don't currently use this for anything, so not implementing yet.
    Ensure(false);
    return false;
}

bool NetConnectionTCP::Recieve(std::vector<uint8_t>& Buffer, int Offset, int Count, int& BytesRecieved)
{
    if (Socket == INVALID_SOCKET_VALUE)
    {
        return false;
    }

    BytesRecieved = 0;

    int Result = recv(Socket, reinterpret_cast<char*>(Buffer.data() + Offset), Count, 0);
    if (Result < 0)
    {
#if defined(_WIN32)
        int error = WSAGetLastError();
#else
        int error = errno;
#endif

        // Blocking is fine, just return true but no bytes if that occurs.
#if defined(_WIN32)
        if (error == WSAEWOULDBLOCK)
#else        
        if (error == EWOULDBLOCK || error == EAGAIN)
#endif
        {
            return true;
        }

        ErrorS(GetName().c_str(), "Failed to recieve with error 0x%08x.", error);
        return false;
    }
    else if (Result == 0)
    {
        HasDisconnected = true;
    }

    Debug::TcpBytesRecieved.Add(Result);

    BytesRecieved = Result;
    return true;
}

bool NetConnectionTCP::Send(const std::vector<uint8_t>& Buffer, int Offset, int Count)
{
    std::vector<uint8_t> CipheredBuffer;
    CipheredBuffer.resize(Count);

    memcpy(CipheredBuffer.data(), Buffer.data() + Offset, Count);

    size_t InsertOffset = SendQueue.size();
    size_t NewQueueSize = SendQueue.size() + CipheredBuffer.size();

    if (NewQueueSize > BuildConfig::MAX_SEND_QUEUE_SIZE)
    {
        WarningS(GetName().c_str(), "Failed to send packet, send queue is saturated.");
        return false;
    }

    SendQueue.resize(NewQueueSize);

    memcpy(SendQueue.data() + InsertOffset, CipheredBuffer.data(), CipheredBuffer.size());

    return true;
}

bool NetConnectionTCP::SendPartial(const std::vector<uint8_t>& Buffer, int Offset, int Count, int& BytesSent)
{
    if (Socket == INVALID_SOCKET_VALUE)
    {
        return false;
    }

    BytesSent = 0;

    int Result = send(Socket, reinterpret_cast<const char*>(Buffer.data() + Offset), Count, 0);
    if (Result < 0)
    {
#if defined(_WIN32)
        int error = WSAGetLastError();
#else
        int error = errno;
#endif

        // Blocking is fine, just return true but no bytes if that occurs.
#if defined(_WIN32)
        if (error == WSAEWOULDBLOCK)
#else        
        if (error == EWOULDBLOCK || error == EAGAIN)
#endif
        {
            return true;
        }

        ErrorS(GetName().c_str(), "Failed to send with error 0x%08x.", error);
        return false;
    }

    Debug::TcpBytesSent.Add(Result);

    BytesSent = Result;

    return true;
}

bool NetConnectionTCP::Disconnect()
{
    if (Socket == INVALID_SOCKET_VALUE)
    {
        return false;
    }

    closesocket(Socket);
    Socket = INVALID_SOCKET_VALUE;
    
    return false;
}

std::string NetConnectionTCP::GetName()
{
    return Name;
}

void NetConnectionTCP::Rename(const std::string& InName)
{
    Name = InName;
}

bool NetConnectionTCP::IsConnected()
{
    if (HasDisconnected)
    {
        return false;
    }

    int error_code;
    int error_code_size = sizeof(error_code);
    if (getsockopt(Socket, SOL_SOCKET, SO_ERROR, (char*)&error_code, &error_code_size) < 0)
    {
        return false;
    }

    return (error_code == 0);
}

bool NetConnectionTCP::Pump()
{
    // Send any data that we are able to.
    while (SendQueue.size() > 0)
    {
        int BytesSent = 0;
        //LogS(GetName().c_str(), "SEND %i", SendQueue.size());
        if (!SendPartial(SendQueue, 0, (int)SendQueue.size(), BytesSent))
        {
            WarningS(GetName().c_str(), "Failed to send on connection.");
            return true;
        }

        if (BytesSent == 0)
        {
            break;
        }
        else
        {
            SendQueue.erase(SendQueue.begin(), SendQueue.begin() + BytesSent);
        }
    }

    return false;
}