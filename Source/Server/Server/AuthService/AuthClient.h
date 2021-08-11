/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Core/Utils/Endian.h"

class AuthService;
class NetConnection;
class Frpg2PacketStream;
class Frpg2MessageStream;
class RSAKeyPair;

// Response data sent in as part of the authentication flow. No idea
// why they didn't just use a protobuf for this.
#pragma pack(push,1)
struct Frpg2GameServerInfo
{
public:

    uint64_t auth_token;
    char     game_server_ip[16];
    uint8_t  stack_data[112];    // Game server leaks its stack!?
    uint16_t game_port;
    uint16_t padding    = 0x00;

    uint32_t unknown_1  = 0x00008000;
    uint32_t unknown_2  = 0x00008000;
    uint32_t unknown_3  = 0x0000A000;
    uint32_t unknown_4  = 0x0000A000;
    uint32_t unknown_5  = 0x00000080;
    uint32_t unknown_6  = 0x00008000;
    uint32_t unknown_7  = 0x0000A000;
    uint32_t unknown_8  = 0x000493E0;
    uint32_t unknown_9  = 0x000061A8;
    uint32_t unknown_10 = 0x0000000C;
    uint32_t unknown_11 = 0;

    void SwapEndian()
    {
        game_port = HostOrderToBigEndian(game_port);

        unknown_1 = HostOrderToBigEndian(unknown_1);
        unknown_2 = HostOrderToBigEndian(unknown_2);
        unknown_3 = HostOrderToBigEndian(unknown_3);
        unknown_4 = HostOrderToBigEndian(unknown_4);
        unknown_5 = HostOrderToBigEndian(unknown_5);
        unknown_6 = HostOrderToBigEndian(unknown_6);
        unknown_7 = HostOrderToBigEndian(unknown_7);
        unknown_8 = HostOrderToBigEndian(unknown_8);
        unknown_9 = HostOrderToBigEndian(unknown_9);
        unknown_10 = HostOrderToBigEndian(unknown_10);
        unknown_11 = HostOrderToBigEndian(unknown_11);
    }
};
#pragma pack(pop)

static_assert(sizeof(Frpg2GameServerInfo) == 184, "Frpg2GameServerInfo is not expected size.");

// Each state we need to go through for the client to authenticate.
enum class AuthClientState
{
    WaitingForHandshakeRequest,
    WaitingForServiceStatusRequest,
    WaitingForKeyData,
    WaitingForSteamTicket,
    Complete,
};

// Represents an individual client connected to the auth service.
class AuthClient
{
public:
    AuthClient(AuthService* OwningService, std::shared_ptr<NetConnection> InConnection, RSAKeyPair* InServerRSAKey);

    // If this returns true the client is expected to be disconnected and is disposed of.
    bool Poll();

    std::string GetName();

private:    
    AuthService* Service;

    std::shared_ptr<NetConnection> Connection;
    std::shared_ptr<Frpg2MessageStream> MessageStream;

    double LastMessageRecievedTime = 0.0;

    std::vector<uint8_t> CwcKey;
    std::vector<uint8_t> GameCwcKey;

    AuthClientState State = AuthClientState::WaitingForHandshakeRequest;

};