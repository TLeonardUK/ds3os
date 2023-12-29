/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Server/Game.h"

class DS3_Game : public Game
{
public:

    virtual bool Protobuf_To_ReliableUdpMessageType(google::protobuf::MessageLite* Message, Frpg2ReliableUdpMessageType& Output) override;
    virtual bool ReliableUdpMessageType_To_Protobuf(Frpg2ReliableUdpMessageType Type, bool IsResponse, std::shared_ptr<google::protobuf::MessageLite>& Output) override;
    virtual bool ReliableUdpMessageType_Expects_Response(Frpg2ReliableUdpMessageType Type) override;

    virtual void RegisterGameManagers(GameService& Service) override;
    virtual std::unique_ptr<PlayerState> CreatePlayerState() override;

    virtual std::string GetAreaName(uint32_t AreaId) override;

    virtual std::string GetBossDiscordThumbnailUrl(uint32_t BossId) override;

    virtual void GetStatistics(GameService& Service, std::unordered_map<std::string, std::string>& Stats) override;

    virtual void SendManagementMessage(Frpg2ReliableUdpMessageStream& stream, const std::string& TextMessage) override;

};