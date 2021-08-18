/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/BloodMessage/BloodMessageManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"
#include "Server/Database/ServerDatabase.h"

#include "Core/Utils/Logging.h"
#include "Core/Utils/File.h"

BloodMessageManager::BloodMessageManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
    LiveMessageCache.SetMaxEntriesPerArea(InServerInstance->GetConfig().BloodMessageMaxLiveCacheEntriesPerArea);
}

MessageHandleResult BloodMessageManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestReentryBloodMessage)
    {
        return Handle_RequestReentryBloodMessage(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetBloodMessageEvaluation)
    {
        return Handle_RequestGetBloodMessageEvaluation(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestCreateBloodMessage)
    {
        return Handle_RequestCreateBloodMessage(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestRemoveBloodMessage)
    {
        return Handle_RequestRemoveBloodMessage(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetBloodMessageList)
    {
        return Handle_RequestGetBloodMessageList(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestEvaluateBloodMessage)
    {
        return Handle_RequestEvaluateBloodMessage(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestReCreateBloodMessageList)
    {
        return Handle_RequestReCreateBloodMessageList(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult BloodMessageManager::Handle_RequestReentryBloodMessage(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();

    Frpg2RequestMessage::RequestReentryBloodMessage* Request = (Frpg2RequestMessage::RequestReentryBloodMessage*)Message.Protobuf.get();
    Ensure(Request->unknown_2() == 1);

    Frpg2RequestMessage::RequestReentryBloodMessageResponse Response;
    auto RecreateMessageIds = Response.mutable_recreate_message_ids();

    // Go through all ids, if they are already in the live pool, do nothing, if they
    // are in the database, move them to the live pool, if they don't exist in either
    // ask for them to be recreated.
    for (int i = 0; i < Request->messages_size(); i++)
    {
        const Frpg2RequestMessage::LocatedBloodMessage& Message = Request->messages(i);
        
        OnlineAreaId OnlineArea = static_cast<OnlineAreaId>(Message.online_area_id());
        uint32_t Id = Message.message_id();

        if (!LiveMessageCache.Contains(OnlineArea, Id))
        {            
            if (std::shared_ptr<BloodMessage> DatabaseBloodMessage = Database.FindBloodMessage(Id))
            {
                LiveMessageCache.Add(OnlineArea, Id, DatabaseBloodMessage);
            }
            else
            {
                Log("[%s] Requesting client to recreate message %i.", Client->GetName().c_str(), Id);
                RecreateMessageIds->Add(Id);
            }
        }
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestReentryBloodMessageResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }
    
    return MessageHandleResult::Handled;
}

MessageHandleResult BloodMessageManager::Handle_RequestReCreateBloodMessageList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();
    
    Frpg2RequestMessage::RequestReCreateBloodMessageList* Request = (Frpg2RequestMessage::RequestReCreateBloodMessageList*)Message.Protobuf.get();
    Ensure(Request->unknown_2() == 1);

    Frpg2RequestMessage::RequestReCreateBloodMessageListResponse Response;
    auto CreatedMessageIds = Response.mutable_message_ids();

    WriteBytesToFile("Z:\\ds3os\\Temp\\RequestReCreateBloodMessageList.bin", Message.Payload);

    // Recreate all the messages passed through by the client.
    for (int i = 0; i < Request->blood_message_info_list_size(); i++)
    {
        const Frpg2RequestMessage::RequestReCreateBloodMessageList_Blood_message_info_list& MessageInfo = Request->blood_message_info_list(i);

        std::vector<uint8_t> MessageData;
        MessageData.assign(MessageInfo.message_data().data(), MessageInfo.message_data().data() + MessageInfo.message_data().size());

        // Create the message in the database.
        std::shared_ptr<BloodMessage> BloodMessage = Database.CreateBloodMessage((OnlineAreaId)MessageInfo.online_area_id(), Player.PlayerId, Player.SteamId, MessageData);
        if (!BloodMessage)
        {
            Warning("[%s] Disconnecting client as failed to recreate blood message.", Client->GetName().c_str());
            return MessageHandleResult::Error;
        }

        Log("[%s] Recreated message message %i.", Client->GetName().c_str(), BloodMessage->MessageId);

        LiveMessageCache.Add(BloodMessage->OnlineAreaId, BloodMessage->MessageId, BloodMessage);
        CreatedMessageIds->Add(BloodMessage->MessageId);
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestReCreateBloodMessageListResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult BloodMessageManager::Handle_RequestGetBloodMessageEvaluation(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    Frpg2RequestMessage::RequestGetBloodMessageEvaluation* Request = (Frpg2RequestMessage::RequestGetBloodMessageEvaluation*)Message.Protobuf.get();

    Frpg2RequestMessage::RequestGetBloodMessageEvaluationResponse Response;
    auto MessageEvaluation = Response.messages();

    // Grab the current rating for every message, first from live if possible, then database.
    for (int i = 0; i < Request->messages_size(); i++)
    {
        const Frpg2RequestMessage::LocatedBloodMessage& MessageInfo = Request->messages(i);

        Frpg2RequestMessage::BloodMessageEvaluationData& EvalData = *MessageEvaluation.Add();
        EvalData.set_message_id(MessageInfo.message_id());

        // Try live cache first.
        if (std::shared_ptr<BloodMessage> ActiveMessage = LiveMessageCache.Find((OnlineAreaId)MessageInfo.online_area_id(), MessageInfo.message_id()))
        {
            EvalData.set_good(ActiveMessage->RatingGood);
            EvalData.set_poor(ActiveMessage->RatingPoor);
        }
        // Otherwise fall back to database.
        else if (std::shared_ptr<BloodMessage> ActiveMessage = Database.FindBloodMessage(MessageInfo.message_id()))
        {
            EvalData.set_good(ActiveMessage->RatingGood);
            EvalData.set_poor(ActiveMessage->RatingPoor);

            LiveMessageCache.Add(ActiveMessage->OnlineAreaId, ActiveMessage->MessageId, ActiveMessage);
        }
        // If we can't find it, just return 0 evaluation, this shouldn't happen in practice.
        else
        {
            Warning("[%s] Client requested evaluation of unknown message id '%u', returning 0.", Client->GetName().c_str(), MessageInfo.message_id());
            EvalData.set_good(0);
            EvalData.set_poor(0);
        }        
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetBloodMessageEvaluationResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult BloodMessageManager::Handle_RequestCreateBloodMessage(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestCreateBloodMessage* Request = (Frpg2RequestMessage::RequestCreateBloodMessage*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestCreateBloodMessageResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestCreateBloodMessageResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult BloodMessageManager::Handle_RequestRemoveBloodMessage(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestRemoveBloodMessage* Request = (Frpg2RequestMessage::RequestRemoveBloodMessage*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestRemoveBloodMessageResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestRemoveBloodMessageResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult BloodMessageManager::Handle_RequestGetBloodMessageList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestGetBloodMessageList* Request = (Frpg2RequestMessage::RequestGetBloodMessageList*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestGetBloodMessageListResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetBloodMessageListResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult BloodMessageManager::Handle_RequestEvaluateBloodMessage(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestEvaluateBloodMessage* Request = (Frpg2RequestMessage::RequestEvaluateBloodMessage*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestEvaluateBloodMessageResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestEvaluateBloodMessageResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string BloodMessageManager::GetName()
{
    return "Blood Message";
}
