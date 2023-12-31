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
#include "Server/GameService/Utils/OnlineAreaPool.h"
#include "Server/Database/DatabaseTypes.h"
#include "Server.DarkSouls2/Server/GameService/Utils/DS2_GameIds.h"
#include "Server.DarkSouls2/Server/GameService/Utils/DS2_CellAndAreaId.h"

struct Frpg2ReliableUdpMessage;
class Server;
class GameService;

// Handles client requests relating to placing, reading and 
// upvoting blood message left in the world.

class DS2_BloodMessageManager
    : public GameManager
{
public:    
    DS2_BloodMessageManager(Server* InServerInstance, GameService* InGameServiceInstance);

    virtual bool Init() override;
    virtual void Poll() override;
    virtual void TrimDatabase() override;

    virtual MessageHandleResult OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message) override;

    virtual std::string GetName() override;

    size_t GetLiveCount() { return LiveCache.GetTotalEntries(); }

protected:
    MessageHandleResult Handle_RequestReentryBloodMessage(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestGetBloodMessageEvaluation(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestCreateBloodMessage(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestRemoveBloodMessage(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestGetBloodMessageList(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestGetAreaBloodMessageList(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestEvaluateBloodMessage(GameClient* Client, const Frpg2ReliableUdpMessage& Message);

private:
    Server* ServerInstance;
    GameService* GameServiceInstance;

    OnlineAreaPool<DS2_CellAndAreaId, BloodMessage> LiveCache;

};