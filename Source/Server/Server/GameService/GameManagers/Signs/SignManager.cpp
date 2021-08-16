/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Signs/SignManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Core/Utils/Logging.h"

SignManager::SignManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

MessageHandleResult SignManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
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
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestUpdateSign)
    {
        return Handle_RequestUpdateSign(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestRejectSign)
    {
        return Handle_RequestRejectSign(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetRightMatchingArea)
    {
        return Handle_RequestGetRightMatchingArea(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult SignManager::Handle_RequestGetSignList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestGetSignList* Request = (Frpg2RequestMessage::RequestGetSignList*)Message.Protobuf.get();

    // TODO: Implement

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

MessageHandleResult SignManager::Handle_RequestCreateSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestCreateSign* Request = (Frpg2RequestMessage::RequestCreateSign*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestCreateSignResponse Response;
    Response.set_sign_unique_number(1); // TODO: actual value please.

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetSignListResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult SignManager::Handle_RequestRemoveSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestRemoveSign* Request = (Frpg2RequestMessage::RequestRemoveSign*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestRemoveSignResponse Response;

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestRemoveSignResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult SignManager::Handle_RequestUpdateSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestUpdateSign* Request = (Frpg2RequestMessage::RequestUpdateSign*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestUpdateSignResponse Response;

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestUpdateSignResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult SignManager::Handle_RequestRejectSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestRejectSign* Request = (Frpg2RequestMessage::RequestRejectSign*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestRejectSignResponse Response;

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestRejectSignResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult SignManager::Handle_RequestGetRightMatchingArea(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestGetRightMatchingArea* Request = (Frpg2RequestMessage::RequestGetRightMatchingArea*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestGetRightMatchingAreaResponse Response;

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetRightMatchingArea response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string SignManager::GetName()
{
    return "Signs";
}
