/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Matchmaking/MatchmakingManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Core/Utils/Logging.h"

MatchmakingManager::MatchmakingManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

MessageHandleResult MatchmakingManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetSignList)
    {
        return Handle_RequestGetSignList(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestCreateSign)
    {
        return Handle_RequestCreateSign(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestRemoveSign)
    {
        return Handle_RequestRemoveSign(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult MatchmakingManager::Handle_RequestGetSignList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestGetSignList* Request = (Frpg2RequestMessage::RequestGetSignList*)Message.Protobuf.get();

    // TODO: Do something.

    Frpg2RequestMessage::RequestGetSignListResponse Response;
    Frpg2RequestMessage::GetSignResult* Result = Response.mutable_get_sign_result();
/*      
        Frpg2RequestMessage::SignData* Data = Result->add_sign_data();
        Data->set_player_struct("");
        Data->set_online_area_id(0);
*/

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetSignListResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }
    
    return MessageHandleResult::Handled;
}

MessageHandleResult MatchmakingManager::Handle_RequestCreateSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestCreateSign* Request = (Frpg2RequestMessage::RequestCreateSign*)Message.Protobuf.get();

    // TODO: Do something.

    Frpg2RequestMessage::RequestCreateSignResponse Response;
    Response.set_sign_unique_number(1); // TODO: actual value please.

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetSignListResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult MatchmakingManager::Handle_RequestRemoveSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestCreateSign* Request = (Frpg2RequestMessage::RequestCreateSign*)Message.Protobuf.get();

    // TODO: Do something.

    Frpg2RequestMessage::RequestRemoveSignResponse Response;

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestRemoveSignResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string MatchmakingManager::GetName()
{
    return "Matchmaking";
}
