/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Ranking/RankingManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Core/Utils/Logging.h"

RankingManager::RankingManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

MessageHandleResult RankingManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestRegisterRankingData)
    {
        return Handle_RequestRegisterRankingData(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetRankingData)
    {
        return Handle_RequestGetRankingData(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetCharacterRankingData)
    {
        return Handle_RequestGetCharacterRankingData(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestCountRankingData)
    {
        return Handle_RequestCountRankingData(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult RankingManager::Handle_RequestRegisterRankingData(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestRegisterRankingData* Request = (Frpg2RequestMessage::RequestRegisterRankingData*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestRegisterRankingDataResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestRegisterRankingDataResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult RankingManager::Handle_RequestGetRankingData(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestGetRankingData* Request = (Frpg2RequestMessage::RequestGetRankingData*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestGetRankingDataResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetRankingDataResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult RankingManager::Handle_RequestGetCharacterRankingData(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestGetCharacterRankingData* Request = (Frpg2RequestMessage::RequestGetCharacterRankingData*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestGetCharacterRankingDataResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetCharacterRankingDataResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult RankingManager::Handle_RequestCountRankingData(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestCountRankingData* Request = (Frpg2RequestMessage::RequestCountRankingData*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestCountRankingDataResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestCountRankingDataResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string RankingManager::GetName()
{
    return "Ranking";
}
