/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/BloodMessage/DS2_BloodMessageManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/GameService/GameService.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/DS2_Frpg2ReliableUdpMessage.h"
#include "Server/GameService/Utils/DS2_NRSSRSanitizer.h"
#include "Protobuf/DS2_Protobufs.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"
#include "Server/Database/ServerDatabase.h"

#include "Config/BuildConfig.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/File.h"
#include "Shared/Core/Utils/Strings.h"

DS2_BloodMessageManager::DS2_BloodMessageManager(Server* InServerInstance, GameService* InGameServiceInstance)
    : ServerInstance(InServerInstance)
    , GameServiceInstance(InGameServiceInstance)
{
    LiveCache.SetMaxEntriesPerArea(InServerInstance->GetConfig().BloodMessageMaxLivePoolEntriesPerArea);
}

bool DS2_BloodMessageManager::Init()
{
    ServerDatabase& Database = ServerInstance->GetDatabase();

    int PrimeCountPerArea = ServerInstance->GetConfig().BloodMessagePrimeCountPerArea;

    // Prime the cache with a handful of the most recent messages from the database.
    int MessageCount = 0;

    const std::vector<DS2_OnlineAreaId>* Areas = GetEnumValues<DS2_OnlineAreaId>();
    for (DS2_OnlineAreaId AreaId : *Areas)
    {
        std::vector<std::shared_ptr<BloodMessage>> Messages = Database.FindRecentBloodMessage((uint32_t)AreaId, PrimeCountPerArea);
        for (const std::shared_ptr<BloodMessage>& Message : Messages)
        {
            LiveCache.Add({ Message->CellId, AreaId }, Message->MessageId, Message);
            MessageCount++;
        }
    }

    if (MessageCount > 0)
    {
        LogS(GetName().c_str(), "Primed live cache with %i blood messages.", MessageCount);
    }

    return true;
}

void DS2_BloodMessageManager::Poll()
{
}

void DS2_BloodMessageManager::TrimDatabase()
{
    ServerDatabase& Database = ServerInstance->GetDatabase();

    int MaxEntries = ServerInstance->GetConfig().BloodMessageMaxDatabaseEntries;

    Database.TrimBloodMessages(MaxEntries);
}

MessageHandleResult DS2_BloodMessageManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestReentryBloodMessage))
    {
        return Handle_RequestReentryBloodMessage(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestGetBloodMessageEvaluation))
    {
        return Handle_RequestGetBloodMessageEvaluation(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestCreateBloodMessage))
    {
        return Handle_RequestCreateBloodMessage(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestRemoveBloodMessage))
    {
        return Handle_RequestRemoveBloodMessage(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestGetBloodMessageList))
    {
        return Handle_RequestGetBloodMessageList(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestGetAreaBloodMessageList))
    {
        return Handle_RequestGetAreaBloodMessageList(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestEvaluateBloodMessage))
    {
        return Handle_RequestEvaluateBloodMessage(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult DS2_BloodMessageManager::Handle_RequestReentryBloodMessage(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    
    DS2_Frpg2RequestMessage::RequestReentryBloodMessage* Request = (DS2_Frpg2RequestMessage::RequestReentryBloodMessage*)Message.Protobuf.get();

    DS2_Frpg2RequestMessage::RequestReentryBloodMessageResponse Response;
    
    DS2_CellAndAreaId LocationId = { Request->cell_id(), (DS2_OnlineAreaId)Request->online_area_id() };
    uint32_t MessageId = Request->message_id();

    if (!LiveCache.Contains(LocationId, MessageId))
    {            
        if (std::shared_ptr<BloodMessage> DatabaseBloodMessage = Database.FindBloodMessage(MessageId))
        {
            LiveCache.Add(LocationId, MessageId, DatabaseBloodMessage);
        }
        else
        {
            LogS(Client->GetName().c_str(), "Could not find client recreate message %i.", MessageId);
        }
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestReentryBloodMessageResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_BloodMessageManager::Handle_RequestGetBloodMessageEvaluation(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestGetBloodMessageEvaluation* Request = (DS2_Frpg2RequestMessage::RequestGetBloodMessageEvaluation*)Message.Protobuf.get();

    DS2_Frpg2RequestMessage::RequestGetBloodMessageEvaluationResponse Response;
    Response.set_message_id(Request->message_id());

    DS2_CellAndAreaId LocationId = { Request->cell_id(), (DS2_OnlineAreaId)Request->online_area_id() };
    uint32_t MessageId = Request->message_id();

    // Try live cache first.
    if (std::shared_ptr<BloodMessage> ActiveMessage = LiveCache.Find(LocationId, MessageId))
    {
        Response.set_rating(ActiveMessage->RatingGood);
    }
    // Otherwise fall back to database.
    else if (std::shared_ptr<BloodMessage> ActiveMessage = Database.FindBloodMessage(MessageId))
    {
        Response.set_rating(ActiveMessage->RatingGood);

        LiveCache.Add(LocationId, ActiveMessage->MessageId, ActiveMessage);
    }
    // If we can't find it, just return 0 evaluation, this shouldn't happen in practice.
    else
    {
        WarningS(Client->GetName().c_str(), "Client requested evaluation of unknown message id '%u', returning 0.", MessageId);
        Response.set_rating(0);
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetBloodMessageEvaluationResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_BloodMessageManager::Handle_RequestCreateBloodMessage(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestCreateBloodMessage* Request = (DS2_Frpg2RequestMessage::RequestCreateBloodMessage*)Message.Protobuf.get();

    DS2_Frpg2RequestMessage::RequestCreateBloodMessageResponse Response;

    std::vector<uint8_t> MessageData;
    MessageData.assign(Request->message_data().data(), Request->message_data().data() + Request->message_data().size());

    // There is no NRSSR struct in blood messsage data, but we still make sure the size-delimited entry list is valid.
    if (BuildConfig::NRSSR_SANITY_CHECKS)
    {
        auto ValidationResult = DS2_NRSSRSanitizer::ValidateEntryList(MessageData.data(), MessageData.size());
        if (ValidationResult != DS2_NRSSRSanitizer::ValidationResult::Valid)
        {
            WarningS(Client->GetName().c_str(), "Blood message data recieved from client is invalid (error code %i).",
                static_cast<uint32_t>(ValidationResult));

            // Simply ignore the request. Perhaps sending a response with an invalid sign id or disconnecting the client would be better?
            return MessageHandleResult::Handled;
        }
    }

    if (std::shared_ptr<BloodMessage> ActiveMessage = Database.CreateBloodMessage((uint32_t)Request->online_area_id(), (uint64_t)Request->cell_id(), Player.GetPlayerId(), Player.GetSteamId(), Request->character_id(), MessageData))
    {
        Response.set_message_id(ActiveMessage->MessageId);

        LiveCache.Add({ ActiveMessage->CellId, (DS2_OnlineAreaId)ActiveMessage->OnlineAreaId }, ActiveMessage->MessageId, ActiveMessage);
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

MessageHandleResult DS2_BloodMessageManager::Handle_RequestRemoveBloodMessage(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestRemoveBloodMessage* Request = (DS2_Frpg2RequestMessage::RequestRemoveBloodMessage*)Message.Protobuf.get();

    LogS(Client->GetName().c_str(), "Removing blood message %i.", Request->message_id());

    if (Database.RemoveOwnBloodMessage(Player.GetPlayerId(), Request->message_id()))
    {
        LiveCache.Remove({ (uint64_t)Request->cell_id(), (DS2_OnlineAreaId)Request->online_area_id() }, Request->message_id());
    }
    else
    {
        WarningS(Client->GetName().c_str(), "Failed to remove blood message.");
    }

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
    DS2_Frpg2RequestMessage::RequestRemoveBloodMessageResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestRemoveBloodMessageResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_BloodMessageManager::Handle_RequestGetBloodMessageList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    const RuntimeConfig& Config = ServerInstance->GetConfig();
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestGetBloodMessageList* Request = (DS2_Frpg2RequestMessage::RequestGetBloodMessageList*)Message.Protobuf.get();
    DS2_Frpg2RequestMessage::RequestGetBloodMessageListResponse Response;
    Response.set_online_area_id(Request->online_area_id());
    Response.mutable_messages();

    int RemainingMessageCount = (int)Request->max_messages();

    if (!Config.DisableBloodMessages)
    {
        // Grab a random set of message from the live cache.
        for (int i = 0; i < Request->search_areas_size() && RemainingMessageCount > 0; i++)
        {
            const DS2_Frpg2RequestMessage::BloodMessageCellLimitData& Area = Request->search_areas(i);

            uint64_t CellId = Area.cell_id();
            uint32_t MaxForCell = Area.max_type_1() + Area.max_type_2(); // TODO: we need to figure out the difference between these two types.
            int GatherCount = std::min((int)MaxForCell, (int)RemainingMessageCount);

            std::vector<std::shared_ptr<BloodMessage>> AreaMessages = LiveCache.GetRandomSet(GatherCount, [CellId](DS2_CellAndAreaId Id) {
                return Id.CellId == CellId;
            });
            for (std::shared_ptr<BloodMessage>& AreaMsg : AreaMessages)
            {
                DS2_Frpg2RequestMessage::BloodMessageData& Data = *Response.mutable_messages()->Add();
                Data.set_player_id(AreaMsg->PlayerId);
                Data.set_character_id(AreaMsg->CharacterId); 
                Data.set_message_id(AreaMsg->MessageId);
                Data.set_good(AreaMsg->RatingGood);
                Data.set_message_data(AreaMsg->Data.data(), AreaMsg->Data.size());
                Data.set_player_steam_id(AreaMsg->PlayerSteamId);
                Data.set_cell_id(AreaMsg->CellId);

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

MessageHandleResult DS2_BloodMessageManager::Handle_RequestGetAreaBloodMessageList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    const RuntimeConfig& Config = ServerInstance->GetConfig();
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestGetAreaBloodMessageList* Request = (DS2_Frpg2RequestMessage::RequestGetAreaBloodMessageList*)Message.Protobuf.get();
    DS2_Frpg2RequestMessage::RequestGetBloodMessageListResponse Response;
    Response.set_online_area_id(Request->online_area_id());
    Response.mutable_messages();

    if (!Config.DisableBloodMessages)
    {
        DS2_OnlineAreaId AreaId = (DS2_OnlineAreaId)Request->online_area_id();
        int MaxForArea = Request->max_type_1() + Request->max_type_2(); // TODO: we need to figure out the difference between these two types.

        std::vector<std::shared_ptr<BloodMessage>> AreaMessages = LiveCache.GetRandomSet(MaxForArea, [AreaId](DS2_CellAndAreaId Id) {
            return Id.AreaId == AreaId;
        });

        for (std::shared_ptr<BloodMessage>& AreaMsg : AreaMessages)
        {
            DS2_Frpg2RequestMessage::BloodMessageData& Data = *Response.mutable_messages()->Add();
            Data.set_player_id(AreaMsg->PlayerId);
            Data.set_character_id(AreaMsg->CharacterId);
            Data.set_message_id(AreaMsg->MessageId);
            Data.set_good(AreaMsg->RatingGood);
            Data.set_message_data(AreaMsg->Data.data(), AreaMsg->Data.size());
            Data.set_player_steam_id(AreaMsg->PlayerSteamId);
            Data.set_cell_id(AreaMsg->CellId);
        }
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetAreaBloodMessageListResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_BloodMessageManager::Handle_RequestEvaluateBloodMessage(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestEvaluateBloodMessage* Request = (DS2_Frpg2RequestMessage::RequestEvaluateBloodMessage*)Message.Protobuf.get();

    std::shared_ptr<BloodMessage> ActiveMessage = nullptr;

    DS2_CellAndAreaId LocationId = { Request->cell_id(), (DS2_OnlineAreaId)Request->online_area_id() };

    // If its in live cache, evaluate it.
    if (ActiveMessage = LiveCache.Find(LocationId, Request->message_id()))
    {
        // Handled below
    }
    // Otherwise try and get it out of database.
    else if (ActiveMessage = Database.FindBloodMessage(Request->message_id()))
    {
        LiveCache.Add(LocationId, ActiveMessage->MessageId, ActiveMessage);
    }
    // If we can't find it, just return 0 evaluation, this shouldn't happen in practice.
    else
    {
        WarningS(Client->GetName().c_str(), "Client as attempted to evaluate unknown unknown message id '%u'.", Request->message_id());
    }

    if (ActiveMessage != nullptr)
    {
        if (ActiveMessage->PlayerId == Player.GetPlayerId())
        {
            WarningS(Client->GetName().c_str(), "Disconnecting client as attempted to evaluate own message id '%u'.", Request->message_id());
            return MessageHandleResult::Error;
        }

        // Update rating and commit to database.
        ActiveMessage->RatingGood++;

        if (!Database.SetBloodMessageEvaluation(Request->message_id(), ActiveMessage->RatingPoor, ActiveMessage->RatingGood))
        {
            WarningS(Client->GetName().c_str(), "Failed to update message evaluation for message id '%u'.", Request->message_id());
        }

        LogS(Client->GetName().c_str(), "Evaluating blood message %i as %s.", ActiveMessage->MessageId, "good");

        // Send push message to originating player if they are online.
        if (std::shared_ptr<GameClient> OriginClient = GameServiceInstance->FindClientByPlayerId(ActiveMessage->PlayerId))
        {
            LogS(Client->GetName().c_str(), "Sending push message for evaluation of blood message %i to player %i.", ActiveMessage->MessageId, ActiveMessage->PlayerId);

            DS2_Frpg2RequestMessage::PushRequestEvaluateBloodMessage EvaluatePushMessage;
            EvaluatePushMessage.set_push_message_id(DS2_Frpg2RequestMessage::PushID_PushRequestEvaluateBloodMessage);
            EvaluatePushMessage.set_player_id(Player.GetPlayerId());
            EvaluatePushMessage.set_player_steam_id(Player.GetSteamId());
            EvaluatePushMessage.set_message_id(ActiveMessage->MessageId);

            if (!OriginClient->MessageStream->Send(&EvaluatePushMessage))
            {
                WarningS(Client->GetName().c_str(), "Failed to send push message for evaluation of blood message %i to player %i.", ActiveMessage->MessageId, ActiveMessage->PlayerId);
            }
        }
    }

    std::string TypeStatisticKey = StringFormat("BloodMessage/TotalEvaluated");
    Database.AddGlobalStatistic(TypeStatisticKey, 1);
    Database.AddPlayerStatistic(TypeStatisticKey, Player.GetPlayerId(), 1);

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
    DS2_Frpg2RequestMessage::RequestEvaluateBloodMessageResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestEvaluateBloodMessageResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string DS2_BloodMessageManager::GetName()
{
    return "Blood Message";
}
