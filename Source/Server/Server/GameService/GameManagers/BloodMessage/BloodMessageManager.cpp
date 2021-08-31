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
#include "Server/GameService/GameService.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"
#include "Server/Database/ServerDatabase.h"

#include "Core/Utils/Logging.h"
#include "Core/Utils/File.h"

BloodMessageManager::BloodMessageManager(Server* InServerInstance, GameService* InGameServiceInstance)
    : ServerInstance(InServerInstance)
    , GameServiceInstance(InGameServiceInstance)
{
    LiveCache.SetMaxEntriesPerArea(InServerInstance->GetConfig().BloodMessageMaxLivePoolEntriesPerArea);
}

bool BloodMessageManager::Init()
{
    ServerDatabase& Database = ServerInstance->GetDatabase();

    int PrimeCountPerArea = ServerInstance->GetConfig().BloodMessagePrimeCountPerArea;

    // Prime the cache with a handful of the most recent messages from the database.
    int MessageCount = 0;

    const std::vector<OnlineAreaId>* Areas = GetEnumValues<OnlineAreaId>();
    for (OnlineAreaId AreaId : *Areas)
    {
        std::vector<std::shared_ptr<BloodMessage>> Messages = Database.FindRecentBloodMessage(AreaId, PrimeCountPerArea);
        for (const std::shared_ptr<BloodMessage>& Message : Messages)
        {
            LiveCache.Add(AreaId, Message->MessageId, Message);
            MessageCount++;
        }
    }

    if (MessageCount > 0)
    {
        Log("[%s] Primed live cache with %i blood messages.", GetName().c_str(), MessageCount);
    }

    return true;
}

void BloodMessageManager::Poll()
{
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

        if (!LiveCache.Contains(OnlineArea, Id))
        {            
            if (std::shared_ptr<BloodMessage> DatabaseBloodMessage = Database.FindBloodMessage(Id))
            {
                LiveCache.Add(OnlineArea, Id, DatabaseBloodMessage);
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

    Frpg2RequestMessage::RequestReCreateBloodMessageListResponse Response;
    auto CreatedMessageIds = Response.mutable_message_ids();

    // Recreate all the messages passed through by the client.
    for (int i = 0; i < Request->blood_message_info_list_size(); i++)
    {
        const Frpg2RequestMessage::RequestReCreateBloodMessageList_Blood_message_info_list& MessageInfo = Request->blood_message_info_list(i);

        std::vector<uint8_t> MessageData;
        MessageData.assign(MessageInfo.message_data().data(), MessageInfo.message_data().data() + MessageInfo.message_data().size());

        // Create the message in the database.
        std::shared_ptr<BloodMessage> BloodMessage = Database.CreateBloodMessage((OnlineAreaId)MessageInfo.online_area_id(), Player.PlayerId, Player.SteamId, Request->character_id(), MessageData);
        if (!BloodMessage)
        {
            Warning("[%s] Failed to recreate blood message.", Client->GetName().c_str());
            continue;
        }

        Log("[%s] Recreated message %i.", Client->GetName().c_str(), BloodMessage->MessageId);

        LiveCache.Add(BloodMessage->OnlineAreaId, BloodMessage->MessageId, BloodMessage);
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
    auto MessageEvaluation = Response.mutable_messages();

    // Grab the current rating for every message, first from live if possible, then database.
    for (int i = 0; i < Request->messages_size(); i++)
    {
        const Frpg2RequestMessage::LocatedBloodMessage& MessageInfo = Request->messages(i);

        Frpg2RequestMessage::BloodMessageEvaluationData& EvalData = *MessageEvaluation->Add();
        EvalData.set_message_id(MessageInfo.message_id());

        // Try live cache first.
        if (std::shared_ptr<BloodMessage> ActiveMessage = LiveCache.Find((OnlineAreaId)MessageInfo.online_area_id(), MessageInfo.message_id()))
        {
            EvalData.set_good(ActiveMessage->RatingGood);
            EvalData.set_poor(ActiveMessage->RatingPoor);
        }
        // Otherwise fall back to database.
        else if (std::shared_ptr<BloodMessage> ActiveMessage = Database.FindBloodMessage(MessageInfo.message_id()))
        {
            EvalData.set_good(ActiveMessage->RatingGood);
            EvalData.set_poor(ActiveMessage->RatingPoor);

            LiveCache.Add(ActiveMessage->OnlineAreaId, ActiveMessage->MessageId, ActiveMessage);
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
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    Frpg2RequestMessage::RequestCreateBloodMessage* Request = (Frpg2RequestMessage::RequestCreateBloodMessage*)Message.Protobuf.get();

    Frpg2RequestMessage::RequestCreateBloodMessageResponse Response;

    std::vector<uint8_t> MessageData;
    MessageData.assign(Request->message_data().data(), Request->message_data().data() + Request->message_data().size());

    if (std::shared_ptr<BloodMessage> ActiveMessage = Database.CreateBloodMessage((OnlineAreaId)Request->online_area_id(), Player.PlayerId, Player.SteamId, Request->character_id(), MessageData))
    {
        Response.set_message_id(ActiveMessage->MessageId);

        LiveCache.Add(ActiveMessage->OnlineAreaId, ActiveMessage->MessageId, ActiveMessage);
    }
    else
    {
        Warning("[%s] Disconnecting client as failed to create blood message.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestCreateBloodMessageResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult BloodMessageManager::Handle_RequestRemoveBloodMessage(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    Frpg2RequestMessage::RequestRemoveBloodMessage* Request = (Frpg2RequestMessage::RequestRemoveBloodMessage*)Message.Protobuf.get();

    Log("[%s] Removing blood message %i.", Client->GetName().c_str(), Request->message_id());

    if (Database.RemoveOwnBloodMessage(Player.PlayerId, Request->message_id()))
    {
        LiveCache.Remove((OnlineAreaId)Request->online_area_id(), Request->message_id());
    }
    else
    {
        Warning("[%s] Failed to remove blood message.", Client->GetName().c_str());
    }

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
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
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    Frpg2RequestMessage::RequestGetBloodMessageList* Request = (Frpg2RequestMessage::RequestGetBloodMessageList*)Message.Protobuf.get();
    Frpg2RequestMessage::RequestGetBloodMessageListResponse Response;

    uint32_t RemainingMessageCount = Request->max_messages();

    // Grab a random set of message from the live cache.
    for (int i = 0; i < Request->search_areas_size() && RemainingMessageCount > 0; i++)
    {
        const Frpg2RequestMessage::BloodMessageDomainLimitData& Area = Request->search_areas(i);

        OnlineAreaId AreaId = (OnlineAreaId)Area.online_area_id();
        uint32_t MaxForArea = Area.max_type_1() + Area.max_type_2(); // TODO: we need to figure out the difference between these two types.
        uint32_t GatherCount = std::min(MaxForArea, RemainingMessageCount);

        std::vector<std::shared_ptr<BloodMessage>> AreaMessages = LiveCache.GetRandomSet(AreaId, GatherCount);
        for (std::shared_ptr<BloodMessage>& AreaMsg : AreaMessages)
        {
            // Filter players own messages.
            if (AreaMsg->PlayerId == Player.PlayerId)
            {
                continue;
            }

            //Log("[%s] Returning blood message %i in area %i.", Client->GetName().c_str(), AreaMsg->MessageId, AreaMsg->OnlineAreaId);

            Frpg2RequestMessage::BloodMessageData& Data = *Response.mutable_messages()->Add();
            Data.set_player_id(AreaMsg->PlayerId);
            Data.set_character_id(AreaMsg->CharacterId); 
            Data.set_message_id(AreaMsg->MessageId);
            Data.set_good(AreaMsg->RatingGood);
            Data.set_message_data(AreaMsg->Data.data(), AreaMsg->Data.size());
            Data.set_player_steam_id(AreaMsg->PlayerSteamId);
            Data.set_online_area_id((uint32_t)AreaMsg->OnlineAreaId);
            Data.set_poor(AreaMsg->RatingPoor);

            RemainingMessageCount--;
        }
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetBloodMessageListResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult BloodMessageManager::Handle_RequestEvaluateBloodMessage(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    Frpg2RequestMessage::RequestEvaluateBloodMessage* Request = (Frpg2RequestMessage::RequestEvaluateBloodMessage*)Message.Protobuf.get();

    std::shared_ptr<BloodMessage> ActiveMessage = nullptr;

    // If its in live cache, evaluate it.
    if (ActiveMessage = LiveCache.Find((OnlineAreaId)Request->online_area_id(), Request->message_id()))
    {
        // Handled below
    }
    // Otherwise try and get it out of database.
    else if (ActiveMessage = Database.FindBloodMessage(Request->message_id()))
    {
        LiveCache.Add(ActiveMessage->OnlineAreaId, ActiveMessage->MessageId, ActiveMessage);
    }
    // If we can't find it, just return 0 evaluation, this shouldn't happen in practice.
    else
    {
        Warning("[%s] Disconnecting client as attempted to evaluate unknown unknown message id '%u'.", Client->GetName().c_str(), Request->message_id());
        return MessageHandleResult::Error;
    }

    if (ActiveMessage != nullptr)
    {
        if (ActiveMessage->PlayerId == Player.PlayerId)
        {
            Warning("[%s] Disconnecting client as attempted to evaluate own message id '%u'.", Client->GetName().c_str(), Request->message_id());
            return MessageHandleResult::Error;
        }

        // Update rating and commit to database.
        // TODO: We could batch these updates if it becomes problematic.
        if (Request->was_poor())
        {
            ActiveMessage->RatingPoor++;
        }
        else
        {
            ActiveMessage->RatingGood++;
        }

        if (!Database.SetBloodMessageEvaluation(Request->message_id(), ActiveMessage->RatingPoor, ActiveMessage->RatingGood))
        {
            Warning("[%s] Failed to update message evaluation for message id '%u'.", Client->GetName().c_str(), Request->message_id());
        }

        Log("[%s] Evaluating blood message %i as %s.", Client->GetName().c_str(), ActiveMessage->MessageId, Request->was_poor() ? "poor" : "good");

        // Send push message to originating player if they are online.
        if (std::shared_ptr<GameClient> OriginClient = GameServiceInstance->FindClientByPlayerId(ActiveMessage->PlayerId))
        {
            Log("[%s] Sending push message for evaluation of blood message %i to player %i.", Client->GetName().c_str(), ActiveMessage->MessageId, ActiveMessage->PlayerId);

            Frpg2RequestMessage::PushRequestEvaluateBloodMessage EvaluatePushMessage;
            EvaluatePushMessage.set_push_message_id(Frpg2RequestMessage::PushID_PushRequestEvaluateBloodMessage);
            EvaluatePushMessage.set_player_id(Player.PlayerId);
            EvaluatePushMessage.set_player_steam_id(Player.SteamId);
            EvaluatePushMessage.set_message_id(ActiveMessage->MessageId);
            EvaluatePushMessage.set_was_poor(Request->was_poor());

            if (!OriginClient->MessageStream->Send(&EvaluatePushMessage))
            {
                Warning("[%s] Failed to send push message for evaluation of blood message %i to player %i..", Client->GetName().c_str(), ActiveMessage->MessageId, ActiveMessage->PlayerId);
            }
        }
    }

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
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
