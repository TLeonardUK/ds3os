/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/MirrorKnight/DS2_MirrorKnightManager.h"
#include "Server/GameService/DS2_PlayerState.h"
#include "Server/GameService/Utils/DS2_GameIds.h"
#include "Server/GameService/GameClient.h"
#include "Server/GameService/GameService.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/DS2_Frpg2ReliableUdpMessage.h"
#include "Protobuf/DS2_Protobufs.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Config/BuildConfig.h"
#include "Server/GameService/Utils/DS2_NRSSRSanitizer.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/File.h"
#include "Shared/Core/Utils/Strings.h"
#include "Shared/Core/Utils/DiffTracker.h"

#include <cmath>

DS2_MirrorKnightManager::DS2_MirrorKnightManager(Server* InServerInstance, GameService* InGameServiceInstance)
    : ServerInstance(InServerInstance)
    , GameServiceInstance(InGameServiceInstance)
{
}

void DS2_MirrorKnightManager::OnLostPlayer(GameClient* Client)
{
    // Remove all the players signs from the cache.
    for (std::shared_ptr<SummonSign> Sign : Client->ActiveMirrorKnightSummonSigns)
    {
        RemoveSignAndNotifyAware(Sign);
    }
    Client->ActiveMirrorKnightSummonSigns.clear();
}

void DS2_MirrorKnightManager::RemoveSignAndNotifyAware(const std::shared_ptr<SummonSign>& Sign)
{
    LiveCache.erase(Sign->SignId);

    // Tell anyone who is aware of this sign that its been removed.
    for (uint32_t AwarePlayerId : Sign->AwarePlayerIds)
    {
        if (std::shared_ptr<GameClient> OtherClient = GameServiceInstance->FindClientByPlayerId(AwarePlayerId))
        {
            DS2_Frpg2RequestMessage::PushRequestRemoveMirrorKnightSign PushMessage;
            PushMessage.set_push_message_id(DS2_Frpg2RequestMessage::PushID_PushRequestRemoveMirrorKnightSign);
            PushMessage.set_player_id(Sign->PlayerId);
            PushMessage.set_sign_id(Sign->SignId);
            PushMessage.set_player_steam_id(Sign->PlayerSteamId);

            if (!OtherClient->MessageStream->Send(&PushMessage))
            {
                WarningS(OtherClient->GetName().c_str(), "Failed to send PushRequestRemoveMirrorKnightSign to aware player.");
            }
        }
    }

    Sign->BeingSummonedByPlayerId = 0;
    Sign->AwarePlayerIds.clear();
}

void DS2_MirrorKnightManager::Poll()
{
}

MessageHandleResult DS2_MirrorKnightManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestGetMirrorKnightSignList))
    {
        return Handle_RequestGetMirrorKnightSignList(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestCreateMirrorKnightSign))
    {
        return Handle_RequestCreateMirrorKnightSign(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestRemoveMirrorKnightSign))
    {
        return Handle_RequestRemoveMirrorKnightSign(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestUpdateMirrorKnightSign))
    {
        return Handle_RequestUpdateMirrorKnightSign(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestSummonMirrorKnightSign))
    {
        return Handle_RequestSummonMirrorKnightSign(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestRejectMirrorKnightSign))
    {
        return Handle_RequestRejectMirrorKnightSign(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

bool DS2_MirrorKnightManager::CanMatchWith(const DS2_Frpg2RequestMessage::MatchingParameter& Host, const DS2_Frpg2RequestMessage::MatchingParameter& Match, uint32_t SignType)
{
    const RuntimeConfig& Config = ServerInstance->GetConfig();

    // Sign globally disabled?
    if (Config.DisableInvasions)
    {
        return false;
    }

    return Config.DS2_MirrorKnightMatchingParameters.CheckMatch(Host.soul_memory(), Match.soul_memory(), Host.name_engraved_ring() > 0);
}

MessageHandleResult DS2_MirrorKnightManager::Handle_RequestGetMirrorKnightSignList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestGetMirrorKnightSignList* Request = (DS2_Frpg2RequestMessage::RequestGetMirrorKnightSignList*)Message.Protobuf.get();
    DS2_Frpg2RequestMessage::RequestGetMirrorKnightSignListResponse Response;

    int RemainingSignCount = (int)Request->max_signs();

    // Grab as many recent signs as we can from the cache that match our matching criteria.
    for (auto& SignPair : LiveCache)
    {
        std::shared_ptr<SummonSign> Sign = SignPair.second;

        DS2_Frpg2RequestMessage::SignData* SignData = Response.add_sign_data();
        SignData->mutable_sign_info()->set_player_id(Sign->PlayerId);
        SignData->mutable_sign_info()->set_sign_id(Sign->SignId);
        SignData->set_online_area_id((uint32_t)Sign->OnlineAreaId);
        SignData->mutable_matching_parameter()->CopyFrom(static_cast<DS2_Frpg2RequestMessage::MatchingParameter&>(*Sign->MatchingParameters));
        SignData->set_player_struct(Sign->PlayerStruct.data(), Sign->PlayerStruct.size());
        SignData->set_player_steam_id(Sign->PlayerSteamId);
        SignData->set_cell_id((uint32_t)Sign->CellId);
        SignData->set_sign_type(DS2_Frpg2RequestMessage::SignType_MirrorKnight);
            
        // Make sure user is marked as aware of the sign so we can clear up when the sign is removed.
        Sign->AwarePlayerIds.insert(Player.GetPlayerId());

        RemainingSignCount--;
        if (RemainingSignCount <= 0)
        {
            break;
        }
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetSignListResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_MirrorKnightManager::Handle_RequestCreateMirrorKnightSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestCreateMirrorKnightSign* Request = (DS2_Frpg2RequestMessage::RequestCreateMirrorKnightSign*)Message.Protobuf.get();

    // There is no NRSSR struct in the sign metadata, but we still make sure the size-delimited entry list is valid.
    if (BuildConfig::NRSSR_SANITY_CHECKS)
    {
        auto ValidationResult = DS2_NRSSRSanitizer::ValidateEntryList(Request->data().data(), Request->data().size());
        if (ValidationResult != DS2_NRSSRSanitizer::ValidationResult::Valid)
        {
            WarningS(Client->GetName().c_str(), "RequestCreateMirrorKnightSign message recieved from client contains ill formated binary data (error code %i).",
                static_cast<uint32_t>(ValidationResult));

            // Simply ignore the request. Perhaps sending a response with an invalid sign id or disconnecting the client would be better?
            return MessageHandleResult::Handled;
        }
    }

    std::shared_ptr<SummonSign> Sign = std::make_shared<SummonSign>();
    Sign->SignId = NextSignId++;
    Sign->PlayerId = Player.GetPlayerId();
    Sign->PlayerSteamId = Player.GetSteamId();
    Sign->PlayerStruct.assign(Request->data().data(), Request->data().data() + Request->data().size());
    Sign->MatchingParameters = std::make_unique<DS2_Frpg2RequestMessage::MatchingParameter>(Request->matching_parameter());

    LiveCache[Sign->SignId] = Sign;
    Client->ActiveMirrorKnightSummonSigns.push_back(Sign);

    DS2_Frpg2RequestMessage::RequestCreateMirrorKnightSignResponse Response;
    Response.set_sign_id(Sign->SignId);

    std::string TypeStatisticKey = StringFormat("Sign/TotalCreated");
    Database.AddGlobalStatistic(TypeStatisticKey, 1);
    Database.AddPlayerStatistic(TypeStatisticKey, Player.GetPlayerId(), 1);

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetMirrorKnightSignListResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_MirrorKnightManager::Handle_RequestRemoveMirrorKnightSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS2_Frpg2RequestMessage::RequestRemoveMirrorKnightSign* Request = (DS2_Frpg2RequestMessage::RequestRemoveMirrorKnightSign*)Message.Protobuf.get();

    auto iter = LiveCache.find(Request->sign_id());
    if (iter == LiveCache.end())
    {
        WarningS(Client->GetName().c_str(), "Client as attempted to remove non-existant summon sign, %i, has probably already been cleaned up by CreateSummonSign.", Request->sign_id());
        return MessageHandleResult::Handled;
    }

    std::shared_ptr<SummonSign> Sign = iter->second;
    LiveCache.erase(iter);

    // Tell anyone who is aware of this sign that its been removed.
    RemoveSignAndNotifyAware(Sign);

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
    DS2_Frpg2RequestMessage::RequestRemoveMirrorKnightSignResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestRemoveSignResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_MirrorKnightManager::Handle_RequestUpdateMirrorKnightSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS2_Frpg2RequestMessage::RequestUpdateMirrorKnightSign* Request = (DS2_Frpg2RequestMessage::RequestUpdateMirrorKnightSign*)Message.Protobuf.get();

    // I think the game uses this as something of a hearbeat to keep the sign active in the pool.
    // We keep all players signs active until they are removed, so we don't need to handle this.

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
    DS2_Frpg2RequestMessage::RequestUpdateMirrorKnightSignResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestUpdateMirrorKnightSignResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_MirrorKnightManager::Handle_RequestSummonMirrorKnightSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestSummonMirrorKnightSign* Request = (DS2_Frpg2RequestMessage::RequestSummonMirrorKnightSign*)Message.Protobuf.get();

    bool bSuccess = true;

    // Make sure the NRSSR data contained within this message is valid (if the CVE-2022-24126 fix is enabled)
    if (BuildConfig::NRSSR_SANITY_CHECKS)
    {
        auto ValidationResult = DS2_NRSSRSanitizer::ValidateEntryList(Request->player_struct().data(), Request->player_struct().size());
        if (ValidationResult != DS2_NRSSRSanitizer::ValidationResult::Valid)
        {
            WarningS(Client->GetName().c_str(), "RequestSummonSign message recieved from client contains ill formated binary data (error code %i).",
                static_cast<uint32_t>(ValidationResult));

            bSuccess = false;
        }
    }

    DS2_Frpg2RequestMessage::SummonErrorId SummonError = DS2_Frpg2RequestMessage::SummonErrorId_NoLongerBeSummonable;

    // First check the sign still exists, if it doesn't, send a reject message as its probably already used.
    std::shared_ptr<SummonSign> Sign = nullptr;
    auto iter = LiveCache.find(Request->sign_info().sign_id());
    if (iter == LiveCache.end())
    {
        WarningS(Client->GetName().c_str(), "Client attempted to use invalid summon sign, sending back rejection, %i.", Request->sign_info().sign_id());
        bSuccess = false;
        SummonError = DS2_Frpg2RequestMessage::SummonErrorId_SignHasDisappeared;
    }
    else
    {
        Sign = iter->second;
    }

    // Sign is already being summoned, send rejection.
    if (Sign && Sign->BeingSummonedByPlayerId != 0)
    {
        WarningS(Client->GetName().c_str(), "Client attempted to use summon sign that is already being summoned, sending back rejection, %i.", Request->sign_info().sign_id());
        bSuccess = false;        
        SummonError = DS2_Frpg2RequestMessage::SummonErrorId_SignAlreadyUsed;
    }

    // Send an accept message to the source client of the sign telling them someone is trying to summon them.
    if (bSuccess)
    {
        std::shared_ptr<GameClient> OriginClient = GameServiceInstance->FindClientByPlayerId(Sign->PlayerId);
        Ensure(OriginClient != nullptr); // Should always be valid if the sign is in the cache.

        DS2_Frpg2RequestMessage::PushRequestSummonMirrorKnightSign PushMessage;
        PushMessage.set_push_message_id(DS2_Frpg2RequestMessage::PushID_PushRequestSummonMirrorKnightSign);
        PushMessage.set_player_id(Player.GetPlayerId());
        PushMessage.set_player_steam_id(Player.GetSteamId());
        PushMessage.set_sign_id(Sign->SignId);
        PushMessage.set_player_struct(Request->player_struct().data(), Request->player_struct().size());

        if (!OriginClient->MessageStream->Send(&PushMessage))
        {
            WarningS(OriginClient->GetName().c_str(), "Failed to send PushRequestSummonSign.");
            bSuccess = false;
        }
        else
        {
            Sign->BeingSummonedByPlayerId = Player.GetPlayerId();
        }
    }

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
    DS2_Frpg2RequestMessage::RequestSummonMirrorKnightSignResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestSummonSignResponse response.");
        return MessageHandleResult::Error;
    }

    // If failure then send a reject message.
    if (!bSuccess)
    {
        DS2_Frpg2RequestMessage::PushRequestRejectMirrorKnightSign PushMessage;
        PushMessage.set_push_message_id(DS2_Frpg2RequestMessage::PushID_PushRequestRejectMirrorKnightSign);
        PushMessage.set_error(SummonError);
        PushMessage.set_player_steam_id(Player.GetSteamId());
        PushMessage.mutable_sign_info()->CopyFrom(Request->sign_info());

        if (!Client->MessageStream->Send(&PushMessage))
        {
            WarningS(Client->GetName().c_str(), "Failed to send PushRequestRejectMirrorKnightSign.");
            return MessageHandleResult::Error;
        }
    }
    else
    {
        std::string PoolStatisticKey = StringFormat("Sign/TotalSummonsRequested/SignType=%u", (uint32_t)Sign->Type);
        Database.AddGlobalStatistic(PoolStatisticKey, 1);
        Database.AddPlayerStatistic(PoolStatisticKey, Player.GetPlayerId(), 1);

        std::string TypeStatisticKey = StringFormat("Sign/TotalSummonsRequested");
        Database.AddGlobalStatistic(TypeStatisticKey, 1);
        Database.AddPlayerStatistic(TypeStatisticKey, Player.GetPlayerId(), 1);
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_MirrorKnightManager::Handle_RequestRejectMirrorKnightSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS2_Frpg2RequestMessage::RequestRejectMirrorKnightSign* Request = (DS2_Frpg2RequestMessage::RequestRejectMirrorKnightSign*)Message.Protobuf.get();

    // First check the sign still exists, if it doesn't, send a reject message as its probably already used.
    std::shared_ptr<SummonSign> Sign = nullptr;
    auto iter = LiveCache.find(Request->sign_id());
    if (iter == LiveCache.end())
    {
        WarningS(Client->GetName().c_str(), "Client attempted to reject invalid summon sign, %i.", Request->sign_id());
        return MessageHandleResult::Handled;
    }
    else
    {
        Sign = iter->second;
    }

    // Send PushRequestRejectSign response to whichever client attempted to summon them.
    if (Sign->BeingSummonedByPlayerId != 0)
    {
        if (std::shared_ptr<GameClient> OtherClient = GameServiceInstance->FindClientByPlayerId(Sign->BeingSummonedByPlayerId))
        {
            // TODO: Needs validation.

            DS2_Frpg2RequestMessage::PushRequestRejectMirrorKnightSign PushMessage;
            PushMessage.set_push_message_id(DS2_Frpg2RequestMessage::PushID_PushRequestRejectMirrorKnightSign);
            PushMessage.set_error(Request->error());
            PushMessage.set_player_steam_id(Sign->PlayerSteamId);
            PushMessage.mutable_sign_info()->set_player_id(Sign->PlayerId);
            PushMessage.mutable_sign_info()->set_sign_id(Sign->SignId);

            if (!OtherClient->MessageStream->Send(&PushMessage))
            {
                WarningS(Client->GetName().c_str(), "Failed to send PushRequestRejectSign to summoner.");
                return MessageHandleResult::Error;
            }
        }
        else
        {
            WarningS(Client->GetName().c_str(), "PlayerId summoning sign no longer exists, nothing to reject.");
        }        

        Sign->BeingSummonedByPlayerId = 0;
    }

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
    DS2_Frpg2RequestMessage::RequestRejectMirrorKnightSignResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestRejectSignResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string DS2_MirrorKnightManager::GetName()
{
    return "Mirror Knight";
}
