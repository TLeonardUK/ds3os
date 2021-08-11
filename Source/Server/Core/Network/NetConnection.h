/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <string>
#include <memory>
#include <Vector>

// Base class for network connections. This class handles
// both listening and connecting. There are different
// specializations of this class for each network protocol
// that is used (TCP / UDP).

class Cipher;

class NetConnection
{
public:
    virtual ~NetConnection() {};

    virtual bool Listen(int Port) = 0;

    virtual std::shared_ptr<NetConnection> Accept() = 0;

    virtual bool Connect(std::string Hostname, int Port) = 0;

    virtual bool Pump() = 0;

    // Replace with std::span's when they are available.
    virtual bool Peek(std::vector<uint8_t>& Buffer, int Offset, int Count, int& BytesRecieved) = 0;
    virtual bool Recieve(std::vector<uint8_t>& Buffer, int Offset, int Count, int& BytesRecieved) = 0; 
    virtual bool Send(const std::vector<uint8_t>& Buffer, int Offset, int Count) = 0;

    virtual bool Disconnect() = 0;

    virtual bool IsConnected() = 0;

    virtual std::string GetName() = 0;

};