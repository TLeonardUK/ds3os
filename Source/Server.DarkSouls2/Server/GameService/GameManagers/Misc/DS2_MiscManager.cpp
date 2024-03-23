/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Misc/DS2_MiscManager.h"
#include "Server/GameService/DS2_PlayerState.h"
#include "Server/GameService/GameService.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/DS2_Frpg2ReliableUdpMessage.h"

#include "Server/GameService/Utils/DS2_GameIds.h"
#include "Server/GameService/Utils/DS2_NRSSRSanitizer.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Config/BuildConfig.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"

#include <unordered_set>

DS2_MiscManager::DS2_MiscManager(Server* InServerInstance, GameService* InGameServiceInstance)
    : ServerInstance(InServerInstance)
    , GameServiceInstance(InGameServiceInstance)
{
}

MessageHandleResult DS2_MiscManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestSendMessageToPlayers))
    {
        return Handle_RequestSendMessageToPlayers(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestGetTotalDeathCount))
    {
        return Handle_RequestGetTotalDeathCount(Client, Message);
    }
    
    return MessageHandleResult::Unhandled;
}

void DS2_MiscManager::Poll()
{
}

MessageHandleResult DS2_MiscManager::Handle_RequestSendMessageToPlayers(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS2_Frpg2RequestMessage::RequestSendMessageToPlayers* Request = (DS2_Frpg2RequestMessage::RequestSendMessageToPlayers*)Message.Protobuf.get();

    // Wow this whole function seems unsafe, no idea why from software allows this to be a thing.

    std::vector<uint8_t> MessageData;
    MessageData.assign(Request->message().data(), Request->message().data() + Request->message().size());
    
    // DS2 actually uses this function for more than just break-ins, they are used for quick matches and some other id's I'm not sure of yet.
    // So this check is being disabled for now, should be revisted in future.
    bool ShouldProcessRequest = true;

    // RequestSendMessageToPlayers sanity checks (patch CVE-2022-24125)
    // The game seems to only use this function in the BreakIn (invasions) implementation.
    // Hence we should make sure that a malicious client does not use this send arbitrary data to other players.
    if constexpr (BuildConfig::SEND_MESSAGE_TO_PLAYERS_SANITY_CHECKS)
    {
        // Limit player ids this can be sent to - for arenas we can send this to more than one player.
        if (Request->player_ids_size() > 6)
        {
            WarningS(Client->GetName().c_str(), "Client attempted to send message to too many (%i) clients. This is not normal behaviour.", Request->player_ids_size());
            ShouldProcessRequest = false;
        }

        // Validate PushRequestAllowBreakInTarget. The game also uses this function for other message types (during areans for example), we should
        // track down and add validation to those messages as well.
        if (BuildConfig::NRSSR_SANITY_CHECKS)
        {
            auto ValidationResult = DS2_NRSSRSanitizer::ValidatePushMessages(Request->message());
            if (ValidationResult != DS2_NRSSRSanitizer::ValidationResult::Valid)
            {
                WarningS(Client->GetName().c_str(), "PushRequestAllowBreakInTarget message recieved from client contains ill formated binary data (error code %i).",
                    static_cast<uint32_t>(ValidationResult));

                ShouldProcessRequest = false;            
            }
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
    DS2_Frpg2RequestMessage::RequestSendMessageToPlayersResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestSendMessageToPlayersResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_MiscManager::Handle_RequestGetTotalDeathCount(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();

    DS2_Frpg2RequestMessage::RequestGetTotalDeathCount* Request = (DS2_Frpg2RequestMessage::RequestGetTotalDeathCount*)Message.Protobuf.get();

    DS2_Frpg2RequestMessage::RequestGetTotalDeathCountResponse Response;

    std::string TotalStatisticKey = StringFormat("Player/TotalDeaths");
    Response.set_total_death_count(Database.GetGlobalStatistic(TotalStatisticKey));

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetTotalDeathCountResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string DS2_MiscManager::GetName()
{
    return "Misc";
}
