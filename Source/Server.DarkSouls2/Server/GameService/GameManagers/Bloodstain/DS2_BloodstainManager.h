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

// Handles client requests relating to leaving
// and replaying bloodstain ghosts in the world.

class DS2_BloodstainManager
    : public GameManager
{
public:    
    DS2_BloodstainManager(Server* InServerInstance);

    virtual bool Init() override;
    virtual void TrimDatabase() override;

    virtual MessageHandleResult OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message) override;

    virtual std::string GetName() override;

    size_t GetLiveCount() { return LiveCache.GetTotalEntries(); }

protected:
    MessageHandleResult Handle_RequestCreateBloodstain(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestGetBloodstainList(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestGetAreaBloodstainList(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestGetDeadingGhost(GameClient* Client, const Frpg2ReliableUdpMessage& Message);

private:
    Server* ServerInstance;

    OnlineAreaPool<DS2_CellAndAreaId, Bloodstain> LiveCache;

    uint32_t NextMemoryCacheStainId = std::numeric_limits<uint32_t>::max();

};