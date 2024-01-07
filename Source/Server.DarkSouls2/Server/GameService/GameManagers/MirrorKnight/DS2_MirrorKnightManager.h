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
#include "Server.DarkSouls2/Protobuf/DS2_Protobufs.h"
#include "Server.DarkSouls2/Server/GameService/Utils/DS2_GameIds.h"
#include "Server.DarkSouls2/Server/GameService/Utils/DS2_CellAndAreaId.h"

struct Frpg2ReliableUdpMessage;
class Server;
class GameService;

// Handles all client requests to do with matchmaking
// Placing and retrieving mirror knight summon signs.

class DS2_MirrorKnightManager
    : public GameManager
{
public:    
    DS2_MirrorKnightManager(Server* InServerInstance, GameService* InGameServiceInstance);

    virtual MessageHandleResult OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message) override;

    virtual std::string GetName() override;
    virtual void Poll() override;

    virtual void OnLostPlayer(GameClient* Client) override;

    size_t GetLiveCount() { return LiveCache.size(); }

protected:
    bool CanMatchWith(const DS2_Frpg2RequestMessage::MatchingParameter& Client, const DS2_Frpg2RequestMessage::MatchingParameter& Match, uint32_t SignType);

    void RemoveSignAndNotifyAware(const std::shared_ptr<SummonSign>& Sign);

    MessageHandleResult Handle_RequestGetMirrorKnightSignList(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestCreateMirrorKnightSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestRemoveMirrorKnightSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestUpdateMirrorKnightSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestSummonMirrorKnightSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestRejectMirrorKnightSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    
private:
    Server* ServerInstance;
    GameService* GameServiceInstance;

    std::unordered_map<uint32_t, std::shared_ptr<SummonSign>> LiveCache;

    uint32_t NextSignId = 1000;

};