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
#include "Server/GameService/Utils/DS3_OnlineAreaPool.h"
#include "Server/Database/DatabaseTypes.h"
#include "Server.DarkSouls3/Protobuf/DS3_Protobufs.h"

struct Frpg2ReliableUdpMessage;
class Server;
class GameService;

// Handles all client requests to do with matchmaking
// Placing and retrieving summon signs.

class DS3_SignManager
    : public GameManager
{
public:    
    DS3_SignManager(Server* InServerInstance, GameService* InGameServiceInstance);

    virtual MessageHandleResult OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message) override;

    virtual std::string GetName() override;
    virtual void Poll() override;

    virtual void OnLostPlayer(GameClient* Client) override;

    size_t GetLiveCount() { return LiveCache.GetTotalEntries(); }

protected:
    bool CanMatchWith(const DS3_Frpg2RequestMessage::MatchingParameter& Client, const DS3_Frpg2RequestMessage::MatchingParameter& Match, bool IsRedSign);

    void RemoveSignAndNotifyAware(const std::shared_ptr<SummonSign>& Sign);

    MessageHandleResult Handle_RequestGetSignList(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestCreateSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestRemoveSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestUpdateSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestSummonSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestRejectSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestGetRightMatchingArea(GameClient* Client, const Frpg2ReliableUdpMessage& Message);

private:
    Server* ServerInstance;
    GameService* GameServiceInstance;

    DS3_OnlineAreaPool<SummonSign> LiveCache;

    uint32_t NextSignId = 1000;

};