/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/GameService/GameService.h"
#include "Server/GameService/PlayerState.h"

// This is an interface class that wraps any game-specific functionality.

class Game
{
public:
    virtual ~Game() = default;

    // Protobuf lookup 
    virtual bool Protobuf_To_ReliableUdpMessageType(google::protobuf::MessageLite* Message, Frpg2ReliableUdpMessageType& Output) = 0;
    virtual bool ReliableUdpMessageType_To_Protobuf(Frpg2ReliableUdpMessageType Type, bool IsResponse, std::shared_ptr<google::protobuf::MessageLite>& Output) = 0;
    virtual bool ReliableUdpMessageType_Expects_Response(Frpg2ReliableUdpMessageType Type) = 0;

    // Game service
    virtual void RegisterGameManagers(GameService& Service) = 0;
    virtual std::unique_ptr<PlayerState> CreatePlayerState() = 0;

    // Misc
    virtual std::string GetAreaName(uint32_t AreaId) = 0;

    // Discord support.
    virtual std::string GetBossDiscordThumbnailUrl(uint32_t BossId) = 0;

    // Webui support
    virtual void GetStatistics(GameService& Service, std::unordered_map<std::string, std::string>& Stats) = 0;

    // Messages
    virtual void SendManagementMessage(Frpg2ReliableUdpMessageStream& stream, const std::string& TextMessage) = 0;

};