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

#include <memory>

struct Frpg2ReliableUdpMessage;
class Server;
class GameService;

// Handles client requests for visitation (joining other users games via covenants - blue sentinels etc)

class DS2_VisitorManager
    : public GameManager
{
public:    
    DS2_VisitorManager(Server* InServerInstance, GameService* InGameServiceInstance);

    virtual MessageHandleResult OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message) override;

    virtual std::string GetName() override;

protected:
    bool CanMatchWith(const DS2_Frpg2RequestMessage::MatchingParameter& Client, const std::shared_ptr<GameClient>& Match);

    MessageHandleResult Handle_RequestGetVisitorList(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestVisit(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestRejectVisit(GameClient* Client, const Frpg2ReliableUdpMessage& Message);

private:
    Server* ServerInstance;
    GameService* GameServiceInstance;

};