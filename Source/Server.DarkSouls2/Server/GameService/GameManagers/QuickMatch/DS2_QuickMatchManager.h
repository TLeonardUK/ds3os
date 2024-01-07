/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Server/GameService/GameManager.h"
#include "Protobuf/DS2_Protobufs.h"
#include "Server/GameService/Utils/DS2_GameIds.h"

struct Frpg2ReliableUdpMessage;
class Server;
class GameService;

// Handles client requests for joining/leaving quick matches (undead matches)

class DS2_QuickMatchManager
    : public GameManager
{
public:    
    DS2_QuickMatchManager(Server* InServerInstance, GameService* InGameServiceInstance);

    virtual MessageHandleResult OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message) override;

    virtual std::string GetName() override;

    virtual void Poll() override;

    virtual void OnLostPlayer(GameClient* Client) override;
    
    size_t GetLiveCount() { return Matches.size(); }

protected:
    MessageHandleResult Handle_RequestSearchQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestUnregisterQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestUpdateQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestJoinQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestRejectQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestRegisterQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message);

private:
    struct Match
    {
        uint32_t HostPlayerId;
        std::string HostPlayerSteamId;

        DS2_Frpg2RequestMessage::QuickMatchGameMode GameMode;
        DS2_Frpg2RequestMessage::MatchingParameter MatchingParams;

        uint32_t CellId;
        DS2_OnlineAreaId AreaId;

        bool HasStarted = false;
    };

private:
    bool CanMatchWith(GameClient* Client, const DS2_Frpg2RequestMessage::RequestSearchQuickMatch& Request, const std::shared_ptr<Match>& Match);

    std::shared_ptr<Match> GetMatchByHost(uint32_t HostPlayerId);

private:
    Server* ServerInstance;
    GameService* GameServiceInstance;

    std::vector<std::shared_ptr<Match>> Matches;

};