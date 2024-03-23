/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/BloodMessage/DS3_BloodMessageManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/GameService/GameService.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/DS3_Frpg2ReliableUdpMessage.h"
#include "Server.DarkSouls3/Protobuf/DS3_Protobufs.h"

#include "Server/GameService/Utils/Data/DS3_BloodMessageData.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"
#include "Server/Database/ServerDatabase.h"

#include "Config/BuildConfig.h"
#include "Server/GameService/Utils/DS3_NRSSRSanitizer.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/File.h"
#include "Shared/Core/Utils/Strings.h"

DS3_BloodMessageManager::DS3_BloodMessageManager(Server* InServerInstance, GameService* InGameServiceInstance)
    : ServerInstance(InServerInstance)
    , GameServiceInstance(InGameServiceInstance)
{
    LiveCache.SetMaxEntriesPerArea(InServerInstance->GetConfig().BloodMessageMaxLivePoolEntriesPerArea);
}

bool DS3_BloodMessageManager::Init()
{
    ServerDatabase& Database = ServerInstance->GetDatabase();

    int PrimeCountPerArea = ServerInstance->GetConfig().BloodMessagePrimeCountPerArea;

    // Prime the cache with a handful of the most recent messages from the database.
    int MessageCount = 0;

    const std::vector<DS3_OnlineAreaId>* Areas = GetEnumValues<DS3_OnlineAreaId>();
    for (DS3_OnlineAreaId AreaId : *Areas)
    {
        std::vector<std::shared_ptr<BloodMessage>> Messages = Database.FindRecentBloodMessage((uint32_t)AreaId, 0, PrimeCountPerArea);
        for (const std::shared_ptr<BloodMessage>& Message : Messages)
        {
            LiveCache.Add(AreaId, Message->MessageId, Message);
            MessageCount++;
        }
    }

    if (MessageCount > 0)
    {
        LogS(GetName().c_str(), "Primed live cache with %i blood messages.", MessageCount);
    }

    return true;
}

void DS3_BloodMessageManager::Poll()
{
}

void DS3_BloodMessageManager::TrimDatabase()
{
    ServerDatabase& Database = ServerInstance->GetDatabase();

    int MaxEntries = ServerInstance->GetConfig().BloodMessageMaxDatabaseEntries;

    Database.TrimBloodMessages(MaxEntries);
}

MessageHandleResult DS3_BloodMessageManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestReentryBloodMessage))
    {
        return Handle_RequestReentryBloodMessage(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestGetBloodMessageEvaluation))
    {
        return Handle_RequestGetBloodMessageEvaluation(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestCreateBloodMessage))
    {
        return Handle_RequestCreateBloodMessage(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestRemoveBloodMessage))
    {
        return Handle_RequestRemoveBloodMessage(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestGetBloodMessageList))
    {
        return Handle_RequestGetBloodMessageList(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestEvaluateBloodMessage))
    {
        return Handle_RequestEvaluateBloodMessage(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestReCreateBloodMessageList))
    {
        return Handle_RequestReCreateBloodMessageList(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult DS3_BloodMessageManager::Handle_RequestReentryBloodMessage(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();

    DS3_Frpg2RequestMessage::RequestReentryBloodMessage* Request = (DS3_Frpg2RequestMessage::RequestReentryBloodMessage*)Message.Protobuf.get();

    DS3_Frpg2RequestMessage::RequestReentryBloodMessageResponse Response;
    auto RecreateMessageIds = Response.mutable_recreate_message_ids();

    // Go through all ids, if they are already in the live pool, do nothing, if they
    // are in the database, move them to the live pool, if they don't exist in either
    // ask for them to be recreated.
    for (int i = 0; i < Request->messages_size(); i++)
    {
        const DS3_Frpg2RequestMessage::LocatedBloodMessage& Message = Request->messages(i);
        
        DS3_OnlineAreaId OnlineArea = static_cast<DS3_OnlineAreaId>(Message.online_area_id());
        uint32_t Id = Message.message_id();

        if (!LiveCache.Contains(OnlineArea, Id))
        {            
            if (std::shared_ptr<BloodMessage> DatabaseBloodMessage = Database.FindBloodMessage(Id))
            {
                LiveCache.Add(OnlineArea, Id, DatabaseBloodMessage);
            }
            else
            {
                LogS(Client->GetName().c_str(), "Requesting client to recreate message %i.", Id);
                RecreateMessageIds->Add(Id);
            }
        }
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestReentryBloodMessageResponse response.");
        return MessageHandleResult::Error;
    }
    
    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_BloodMessageManager::Handle_RequestReCreateBloodMessageList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();
    
    DS3_Frpg2RequestMessage::RequestReCreateBloodMessageList* Request = (DS3_Frpg2RequestMessage::RequestReCreateBloodMessageList*)Message.Protobuf.get();

    DS3_Frpg2RequestMessage::RequestReCreateBloodMessageListResponse Response;
    auto CreatedMessageIds = Response.mutable_message_ids();

    // Recreate all the messages passed through by the client.
    for (int i = 0; i < Request->blood_message_info_list_size(); i++)
    {
        const DS3_Frpg2RequestMessage::RequestReCreateBloodMessageList_Blood_message_info_list& MessageInfo = Request->blood_message_info_list(i);

        std::vector<uint8_t> MessageData;
        MessageData.assign(MessageInfo.message_data().data(), MessageInfo.message_data().data() + MessageInfo.message_data().size());

        // There is no NRSSR struct in blood messsage data, but we still make sure the size-delimited entry list is valid.
        if (BuildConfig::NRSSR_SANITY_CHECKS)
        {
            auto ValidationResult = DS3_NRSSRSanitizer::ValidateEntryList(MessageData.data(), MessageData.size());
            if (ValidationResult != DS3_NRSSRSanitizer::ValidationResult::Valid)
            {
                WarningS(Client->GetName().c_str(), "Blood message data recieved from client is invalid (error code %i).",
                    static_cast<uint32_t>(ValidationResult));

                // Simply ignore the request. Perhaps sending a response with an invalid sign id or disconnecting the client would be better?
                return MessageHandleResult::Handled;
            }
        }

        // Create the message in the database.
        std::shared_ptr<BloodMessage> BloodMessage = Database.CreateBloodMessage((uint32_t)MessageInfo.online_area_id(), 0, Player.GetPlayerId(), Player.GetSteamId(), Request->character_id(), MessageData);
        if (!BloodMessage)
        {
            WarningS(Client->GetName().c_str(), "Failed to recreate blood message.");
            continue;
        }

        LogS(Client->GetName().c_str(), "Recreated message %i.", BloodMessage->MessageId);

        LiveCache.Add((DS3_OnlineAreaId)BloodMessage->OnlineAreaId, BloodMessage->MessageId, BloodMessage);
        CreatedMessageIds->Add(BloodMessage->MessageId);
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestReCreateBloodMessageListResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_BloodMessageManager::Handle_RequestGetBloodMessageEvaluation(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestGetBloodMessageEvaluation* Request = (DS3_Frpg2RequestMessage::RequestGetBloodMessageEvaluation*)Message.Protobuf.get();

    DS3_Frpg2RequestMessage::RequestGetBloodMessageEvaluationResponse Response;
    auto MessageEvaluation = Response.mutable_messages();

    // Grab the current rating for every message, first from live if possible, then database.
    for (int i = 0; i < Request->messages_size(); i++)
    {
        const DS3_Frpg2RequestMessage::LocatedBloodMessage& MessageInfo = Request->messages(i);

        DS3_Frpg2RequestMessage::BloodMessageEvaluationData& EvalData = *MessageEvaluation->Add();
        EvalData.set_message_id(MessageInfo.message_id());

        // Try live cache first.
        if (std::shared_ptr<BloodMessage> ActiveMessage = LiveCache.Find((DS3_OnlineAreaId)MessageInfo.online_area_id(), MessageInfo.message_id()))
        {
            EvalData.set_good(ActiveMessage->RatingGood);
            EvalData.set_poor(ActiveMessage->RatingPoor);
        }
        // Otherwise fall back to database.
        else if (std::shared_ptr<BloodMessage> ActiveMessage = Database.FindBloodMessage(MessageInfo.message_id()))
        {
            EvalData.set_good(ActiveMessage->RatingGood);
            EvalData.set_poor(ActiveMessage->RatingPoor);

            LiveCache.Add((DS3_OnlineAreaId)ActiveMessage->OnlineAreaId, ActiveMessage->MessageId, ActiveMessage);
        }
        // If we can't find it, just return 0 evaluation, this shouldn't happen in practice.
        else
        {
            WarningS(Client->GetName().c_str(), "Client requested evaluation of unknown message id '%u', returning 0.", MessageInfo.message_id());
            EvalData.set_good(0);
            EvalData.set_poor(0);
        }
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetBloodMessageEvaluationResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_BloodMessageManager::Handle_RequestCreateBloodMessage(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestCreateBloodMessage* Request = (DS3_Frpg2RequestMessage::RequestCreateBloodMessage*)Message.Protobuf.get();

    DS3_Frpg2RequestMessage::RequestCreateBloodMessageResponse Response;

    std::vector<uint8_t> MessageData;
    MessageData.assign(Request->message_data().data(), Request->message_data().data() + Request->message_data().size());

    // There is no NRSSR struct in blood messsage data, but we still make sure the size-delimited entry list is valid.
    if (BuildConfig::NRSSR_SANITY_CHECKS)
    {
        auto ValidationResult = DS3_NRSSRSanitizer::ValidateEntryList(MessageData.data(), MessageData.size());
        if (ValidationResult != DS3_NRSSRSanitizer::ValidationResult::Valid)
        {
            WarningS(Client->GetName().c_str(), "Blood message data recieved from client is invalid (error code %i).",
                static_cast<uint32_t>(ValidationResult));

            // Simply ignore the request. Perhaps sending a response with an invalid sign id or disconnecting the client would be better?
            return MessageHandleResult::Handled;
        }
    }

    DS3_BloodMessageData data;
    if (DS3_BloodMessageData::Parse(MessageData, data))
    {
        std::string msg = data.ToString();
        LogS(Client->GetName().c_str(), "BloodMessage Created: %s", data.ToString().c_str());
    }

    if (std::shared_ptr<BloodMessage> ActiveMessage = Database.CreateBloodMessage((uint32_t)Request->online_area_id(), 0, Player.GetPlayerId(), Player.GetSteamId(), Request->character_id(), MessageData))
    {
        Response.set_message_id(ActiveMessage->MessageId);

        LiveCache.Add((DS3_OnlineAreaId)ActiveMessage->OnlineAreaId, ActiveMessage->MessageId, ActiveMessage);
    }
    else
    {
        WarningS(Client->GetName().c_str(), "Failed to create blood message.");
    }

    std::string TypeStatisticKey = StringFormat("BloodMessage/TotalCreated");
    Database.AddGlobalStatistic(TypeStatisticKey, 1);
    Database.AddPlayerStatistic(TypeStatisticKey, Player.GetPlayerId(), 1);
    
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestCreateBloodMessageResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_BloodMessageManager::Handle_RequestRemoveBloodMessage(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestRemoveBloodMessage* Request = (DS3_Frpg2RequestMessage::RequestRemoveBloodMessage*)Message.Protobuf.get();

    LogS(Client->GetName().c_str(), "Removing blood message %i.", Request->message_id());

    if (Database.RemoveOwnBloodMessage(Player.GetPlayerId(), Request->message_id()))
    {
        LiveCache.Remove((DS3_OnlineAreaId)Request->online_area_id(), Request->message_id());
    }
    else
    {
        WarningS(Client->GetName().c_str(), "Failed to remove blood message.");
    }

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
    DS3_Frpg2RequestMessage::RequestRemoveBloodMessageResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestRemoveBloodMessageResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_BloodMessageManager::Handle_RequestGetBloodMessageList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    const RuntimeConfig& Config = ServerInstance->GetConfig();
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestGetBloodMessageList* Request = (DS3_Frpg2RequestMessage::RequestGetBloodMessageList*)Message.Protobuf.get();
    DS3_Frpg2RequestMessage::RequestGetBloodMessageListResponse Response;
    Response.mutable_messages();

    int RemainingMessageCount = (int)Request->max_messages();

    if (!Config.DisableBloodMessages)
    {
        // Grab a random set of message from the live cache.
        for (int i = 0; i < Request->search_areas_size() && RemainingMessageCount > 0; i++)
        {
            const DS3_Frpg2RequestMessage::BloodMessageDomainLimitData& Area = Request->search_areas(i);

            DS3_OnlineAreaId AreaId = (DS3_OnlineAreaId)Area.online_area_id();
            uint32_t MaxForArea = Area.max_type_1() + Area.max_type_2(); // TODO: we need to figure out the difference between these two types.
            int GatherCount = std::min((int)MaxForArea, RemainingMessageCount);

            std::vector<std::shared_ptr<BloodMessage>> AreaMessages = LiveCache.GetRandomSet(AreaId, GatherCount);
            for (std::shared_ptr<BloodMessage>& AreaMsg : AreaMessages)
            {
                DS3_Frpg2RequestMessage::BloodMessageData& Data = *Response.mutable_messages()->Add();
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
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetBloodMessageListResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_BloodMessageManager::Handle_RequestEvaluateBloodMessage(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestEvaluateBloodMessage* Request = (DS3_Frpg2RequestMessage::RequestEvaluateBloodMessage*)Message.Protobuf.get();

    std::shared_ptr<BloodMessage> ActiveMessage = nullptr;

    // If its in live cache, evaluate it.
    if (ActiveMessage = LiveCache.Find((DS3_OnlineAreaId)Request->online_area_id(), Request->message_id()))
    {
        // Handled below
    }
    // Otherwise try and get it out of database.
    else if (ActiveMessage = Database.FindBloodMessage(Request->message_id()))
    {
        LiveCache.Add((DS3_OnlineAreaId)ActiveMessage->OnlineAreaId, ActiveMessage->MessageId, ActiveMessage);
    }
    // If we can't find it, just return 0 evaluation, this shouldn't happen in practice.
    else
    {
        WarningS(Client->GetName().c_str(), "Client attempted to evaluate unknown unknown message id '%u'.", Request->message_id());
    }

    if (ActiveMessage != nullptr)
    {
        if (ActiveMessage->PlayerId == Player.GetPlayerId())
        {
            WarningS(Client->GetName().c_str(), "Disconnecting client as attempted to evaluate own message id '%u'.", Request->message_id());
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
            WarningS(Client->GetName().c_str(), "Failed to update message evaluation for message id '%u'.", Request->message_id());
        }

        LogS(Client->GetName().c_str(), "Evaluating blood message %i as %s.", ActiveMessage->MessageId, Request->was_poor() ? "poor" : "good");

        // Send push message to originating player if they are online.
        if (std::shared_ptr<GameClient> OriginClient = GameServiceInstance->FindClientByPlayerId(ActiveMessage->PlayerId))
        {
            LogS(Client->GetName().c_str(), "Sending push message for evaluation of blood message %i to player %i.", ActiveMessage->MessageId, ActiveMessage->PlayerId);

            DS3_Frpg2RequestMessage::PushRequestEvaluateBloodMessage EvaluatePushMessage;
            EvaluatePushMessage.set_push_message_id(DS3_Frpg2RequestMessage::PushID_PushRequestEvaluateBloodMessage);
            EvaluatePushMessage.set_player_id(Player.GetPlayerId());
            EvaluatePushMessage.set_player_steam_id(Player.GetSteamId());
            EvaluatePushMessage.set_message_id(ActiveMessage->MessageId);
            EvaluatePushMessage.set_was_poor(Request->was_poor());

            if (!OriginClient->MessageStream->Send(&EvaluatePushMessage))
            {
                WarningS(Client->GetName().c_str(), "Failed to send push message for evaluation of blood message %i to player %i..", ActiveMessage->MessageId, ActiveMessage->PlayerId);
            }
        }
    }

    std::string TypeStatisticKey = StringFormat("BloodMessage/TotalEvaluated");
    Database.AddGlobalStatistic(TypeStatisticKey, 1);
    Database.AddPlayerStatistic(TypeStatisticKey, Player.GetPlayerId(), 1);

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
    DS3_Frpg2RequestMessage::RequestEvaluateBloodMessageResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestEvaluateBloodMessageResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string DS3_BloodMessageManager::GetName()
{
    return "Blood Message";
}
