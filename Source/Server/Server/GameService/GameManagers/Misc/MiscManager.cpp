/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Misc/MiscManager.h"
#include "Server/GameService/GameService.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Config/BuildConfig.h"
#include "Server/GameService/Utils/NRSSRSanitizer.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"

#include <unordered_set>

MiscManager::MiscManager(Server* InServerInstance, GameService* InGameServiceInstance)
    : ServerInstance(InServerInstance)
    , GameServiceInstance(InGameServiceInstance)
{
}

MessageHandleResult MiscManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestNotifyRingBell)
    {
        return Handle_RequestNotifyRingBell(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestSendMessageToPlayers)
    {
        return Handle_RequestSendMessageToPlayers(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestMeasureUploadBandwidth)
    {
        return Handle_RequestMeasureUploadBandwidth(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestMeasureDownloadBandwidth)
    {
        return Handle_RequestMeasureDownloadBandwidth(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetOnlineShopItemList)
    {
        return Handle_RequestGetOnlineShopItemList(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestBenchmarkThroughput)
    {
        return Handle_RequestBenchmarkThroughput(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

void MiscManager::Poll()
{
}

MessageHandleResult MiscManager::Handle_RequestNotifyRingBell(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestNotifyRingBell* Request = (DS3_Frpg2RequestMessage::RequestNotifyRingBell*)Message.Protobuf.get();
    
    // List of locations the user should be in to recieve a push notification about the bell.
    std::unordered_set<OnlineAreaId> NotifyLocations = {
        OnlineAreaId::Archdragon_Peak_Start,
        OnlineAreaId::Archdragon_Peak,
        OnlineAreaId::Archdragon_Peak_Ancient_Wyvern,
        OnlineAreaId::Archdragon_Peak_Dragon_kin_Mausoleum,
        OnlineAreaId::Archdragon_Peak_Nameless_King_Bonfire,
        OnlineAreaId::Archdragon_Peak_Second_Wyvern,
        OnlineAreaId::Archdragon_Peak_Great_Belfry,
        OnlineAreaId::Archdragon_Peak_Mausoleum_Lift
    };

    std::vector<std::shared_ptr<GameClient>> PotentialTargets = GameServiceInstance->FindClients([Client, Request, NotifyLocations](const std::shared_ptr<GameClient>& OtherClient) {
        return NotifyLocations.count(OtherClient->GetPlayerState().GetCurrentArea()) > 0;
    });

    for (std::shared_ptr<GameClient>& OtherClient : PotentialTargets)
    {
        DS3_Frpg2RequestMessage::PushRequestNotifyRingBell PushMessage;
        PushMessage.set_push_message_id(DS3_Frpg2RequestMessage::PushID_PushRequestNotifyRingBell);
        PushMessage.set_player_id(Player.GetPlayerId());
        PushMessage.set_online_area_id(Request->online_area_id());
        PushMessage.set_data(Request->data().data(), Request->data().size());

        if (!OtherClient->MessageStream->Send(&PushMessage))
        {
            WarningS(Client->GetName().c_str(), "Failed to send push message for bell ring to player '%s'", OtherClient->GetName().c_str());
        }
    }

    std::string TypeStatisticKey = StringFormat("Bell/TotalBellRings");
    Database.AddGlobalStatistic(TypeStatisticKey, 1);
    Database.AddPlayerStatistic(TypeStatisticKey, Player.GetPlayerId(), 1);

    DS3_Frpg2RequestMessage::RequestNotifyRingBellResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestNotifyRingBellResponse response.");
        return MessageHandleResult::Error;
    }

    if (ServerInstance->GetConfig().SendDiscordNotice_Bell)
    {
        ServerInstance->SendDiscordNotice(Client->shared_from_this(), DiscordNoticeType::Bell, "Rung archdragon bell.");
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult MiscManager::Handle_RequestSendMessageToPlayers(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestSendMessageToPlayers* Request = (DS3_Frpg2RequestMessage::RequestSendMessageToPlayers*)Message.Protobuf.get();

    // Wow this whole function seems unsafe, no idea why from software allows this to be a thing.

    std::vector<uint8_t> MessageData;
    MessageData.assign(Request->message().data(), Request->message().data() + Request->message().size());

    // RequestSendMessageToPlayers sanity checks (patch CVE-2022-24125)
    // The game seems to only use this function in the BreakIn (invasions) implementation.
    // Hence we should make sure that a malicious client does not use this send arbitrary data to other players.
    bool ShouldProcessRequest = true;
    if constexpr (BuildConfig::SEND_MESSAGE_TO_PLAYERS_SANITY_CHECKS)
    {
        // The request should contain a PushRequestBreakInTarget message
        DS3_Frpg2RequestMessage::PushRequestAllowBreakInTarget EmbdeddedMsg;

        // The request should specify exactly one player
        if (Request->player_ids_size() > 1)
        {
            WarningS(Client->GetName().c_str(), "Client attempted to send message to more than one (%i) client. This is not normal behaviour.", Request->player_ids_size());
            ShouldProcessRequest = false;
        }
        // The request should only ever contain a PushRequestAllowBreakInTarget message
        if (!EmbdeddedMsg.ParseFromString(Request->message()) || EmbdeddedMsg.push_message_id() != DS3_Frpg2RequestMessage::PushMessageId::PushID_PushRequestAllowBreakInTarget)
        {
            WarningS(Client->GetName().c_str(), "RequestSendMessageToPlayers embedded message provided by client is not a valid PushRequestAllowBreakInTarget message.");
            ShouldProcessRequest = false;
        }
    }

    if (ShouldProcessRequest)
    {
        for (int i = 0; i < Request->player_ids_size(); i++)
        {
            uint32_t PlayerId = Request->player_ids(i);

            std::shared_ptr<GameClient> TargetClient = GameServiceInstance->FindClientByPlayerId(PlayerId);
            if (!TargetClient)
            {
                WarningS(Client->GetName().c_str(), "Client attempted to send message to other client %i, but client doesn't exist.", PlayerId);
            }
            else
            {
                if (!TargetClient->MessageStream->SendRawProtobuf(MessageData))
                {
                    WarningS(Client->GetName().c_str(), "Failed to send raw protobuf from RequestSendMessageToPlayers to %s.", TargetClient->GetName().c_str());
                }
            }
        }
    }

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
    DS3_Frpg2RequestMessage::RequestSendMessageToPlayersResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestSendMessageToPlayersResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult MiscManager::Handle_RequestMeasureUploadBandwidth(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestMeasureUploadBandwidth* Request = (DS3_Frpg2RequestMessage::RequestMeasureUploadBandwidth*)Message.Protobuf.get();

    // Never seen this called by client.
    Ensure(false);

    DS3_Frpg2RequestMessage::RequestMeasureUploadBandwidthResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestMeasureUploadBandwidthResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult MiscManager::Handle_RequestMeasureDownloadBandwidth(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestMeasureDownloadBandwidth* Request = (DS3_Frpg2RequestMessage::RequestMeasureDownloadBandwidth*)Message.Protobuf.get();

    // Never seen this called by client.
    Ensure(false);

    DS3_Frpg2RequestMessage::RequestMeasureDownloadBandwidthResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestMeasureDownloadBandwidthResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult MiscManager::Handle_RequestGetOnlineShopItemList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestGetOnlineShopItemList* Request = (DS3_Frpg2RequestMessage::RequestGetOnlineShopItemList*)Message.Protobuf.get();

    // Never seen this called by client.
    Ensure(false);

    DS3_Frpg2RequestMessage::RequestGetOnlineShopItemListResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetOnlineShopItemListResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult MiscManager::Handle_RequestBenchmarkThroughput(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestBenchmarkThroughput* Request = (DS3_Frpg2RequestMessage::RequestBenchmarkThroughput*)Message.Protobuf.get();

    // Never seen this called by client.
    Ensure(false);

    DS3_Frpg2RequestMessage::RequestBenchmarkThroughputResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestBenchmarkThroughputResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string MiscManager::GetName()
{
    return "Misc";
}
