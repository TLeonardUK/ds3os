/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Signs/DS3_SignManager.h"
#include "Server/GameService/DS3_PlayerState.h"
#include "Server/GameService/Utils/DS3_GameIds.h"
#include "Server/GameService/GameClient.h"
#include "Server/GameService/GameService.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/DS3_Frpg2ReliableUdpMessage.h"
#include "Server.DarkSouls3/Protobuf/DS3_Protobufs.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Config/BuildConfig.h"
#include "Server/GameService/Utils/DS3_NRSSRSanitizer.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/File.h"
#include "Shared/Core/Utils/Strings.h"
#include "Shared/Core/Utils/DiffTracker.h"

#include <cmath>

DS3_SignManager::DS3_SignManager(Server* InServerInstance, GameService* InGameServiceInstance)
    : ServerInstance(InServerInstance)
    , GameServiceInstance(InGameServiceInstance)
{
    LiveCache.SetMaxEntriesPerArea(InServerInstance->GetConfig().SummonSignMaxEntriesPerArea);
}

void DS3_SignManager::OnLostPlayer(GameClient* Client)
{
    // Remove all the players signs from the cache.
    for (std::shared_ptr<SummonSign> Sign : Client->ActiveSummonSigns)
    {
        RemoveSignAndNotifyAware(Sign);
    }
    Client->ActiveSummonSigns.clear();
}

void DS3_SignManager::RemoveSignAndNotifyAware(const std::shared_ptr<SummonSign>& Sign)
{
    LiveCache.Remove((DS3_OnlineAreaId)Sign->OnlineAreaId, Sign->SignId);

    // Tell anyone who is aware of this sign that its been removed.
    for (uint32_t AwarePlayerId : Sign->AwarePlayerIds)
    {
        if (std::shared_ptr<GameClient> OtherClient = GameServiceInstance->FindClientByPlayerId(AwarePlayerId))
        {
            DS3_Frpg2RequestMessage::PushRequestRemoveSign PushMessage;
            PushMessage.set_push_message_id(DS3_Frpg2RequestMessage::PushID_PushRequestRemoveSign);
            PushMessage.mutable_message()->set_player_id(Sign->PlayerId);
            PushMessage.mutable_message()->set_sign_id(Sign->SignId);

            if (!OtherClient->MessageStream->Send(&PushMessage))
            {
                WarningS(OtherClient->GetName().c_str(), "Failed to send PushRequestRemoveSign to aware player.");
            }
        }
    }

    Sign->BeingSummonedByPlayerId = 0;
    Sign->AwarePlayerIds.clear();
}

void DS3_SignManager::Poll()
{
}

MessageHandleResult DS3_SignManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestGetSignList))
    {
        return Handle_RequestGetSignList(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestCreateSign))
    {
        return Handle_RequestCreateSign(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestRemoveSign))
    {
        return Handle_RequestRemoveSign(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestUpdateSign))
    {
        return Handle_RequestUpdateSign(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestSummonSign))
    {
        return Handle_RequestSummonSign(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestRejectSign))
    {
        return Handle_RequestRejectSign(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestGetRightMatchingArea))
    {
        return Handle_RequestGetRightMatchingArea(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

bool DS3_SignManager::CanMatchWith(const DS3_Frpg2RequestMessage::MatchingParameter& Host, const DS3_Frpg2RequestMessage::MatchingParameter& Match, uint32_t SignType)
{
    const RuntimeConfig& Config = ServerInstance->GetConfig();

    // Sign globally disabled?
    bool IsDisabled = (SignType == DS3_Frpg2RequestMessage::SignType_RedSoapstone) ? Config.DisableInvasions : Config.DisableCoop;
    if (IsDisabled)
    {
        return false;
    }

    // If password missmatch then no match.
    if (Host.password() != Match.password())
    {
        return false;
    }

    // Check matching parameters.
    if (!ServerInstance->GetConfig().SummonSignMatchingParameters.CheckMatch(
            Host.soul_level(), Host.weapon_level(), 
            Match.soul_level(), Match.weapon_level(), 
            Host.password().size() > 0
        ))
    {
        return false;
    }

    return true;
}

MessageHandleResult DS3_SignManager::Handle_RequestGetSignList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    PlayerState& Player = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestGetSignList* Request = (DS3_Frpg2RequestMessage::RequestGetSignList*)Message.Protobuf.get();
    DS3_Frpg2RequestMessage::RequestGetSignListResponse Response;

#ifdef _DEBUG
    static DiffTracker Tracker;
    Tracker.Field(Player.GetCharacterName().c_str(), "MatchingParameters.unknown_id_2", Request->matching_parameter().unknown_id_2());
    Tracker.Field(Player.GetCharacterName().c_str(), "MatchingParameters.unknown_id_5", Request->matching_parameter().unknown_id_5());
    if (Request->matching_parameter().has_unknown_string())
    {
        Tracker.Field(Player.GetCharacterName().c_str(), "MatchingParameters.unknown_string", Request->matching_parameter().unknown_string());
    }
    if (Request->matching_parameter().has_unknown_id_15())
    {
        Tracker.Field(Player.GetCharacterName().c_str(), "MatchingParameters.unknown_id_15", Request->matching_parameter().unknown_id_15());
    }
    Tracker.Field(Player.GetCharacterName().c_str(), "RequestGetSignList.unknown_id_1", Request->unknown_id_1());
    Tracker.Field(Player.GetCharacterName().c_str(), "SignGetFlags.unknown_id_1", Request->sign_get_flags().unknown_id_1());
    Tracker.Field(Player.GetCharacterName().c_str(), "SignGetFlags.unknown_id_2", Request->sign_get_flags().unknown_id_2());
    Tracker.Field(Player.GetCharacterName().c_str(), "SignGetFlags.unknown_id_3", Request->sign_get_flags().unknown_id_3());
#endif

    uint32_t RemainingSignCount = Request->max_signs();

    DS3_Frpg2RequestMessage::GetSignResult* SignResult = Response.mutable_get_sign_result();

    // Grab as many recent signs as we can from the cache that match our matching criteria.
    for (int i = 0; i < Request->search_areas_size() && RemainingSignCount > 0; i++)
    {
        const DS3_Frpg2RequestMessage::SignDomainGetInfo& Area = Request->search_areas(i);

        // Grab list of signs the client already has.
        std::unordered_set<uint32_t> ClientExistingSignId;
        for (int j = 0; j < Area.already_have_signs_size(); j++)
        {
            ClientExistingSignId.insert(Area.already_have_signs(j).sign_id());
        }

        DS3_OnlineAreaId AreaId = (DS3_OnlineAreaId)Area.online_area_id();
        uint32_t MaxForArea = Area.max_signs();
        uint32_t GatherCount = std::min(MaxForArea, RemainingSignCount);

        std::vector<std::shared_ptr<SummonSign>> AreaSigns = LiveCache.GetRecentSet(AreaId, GatherCount, [this, &Player, &Request](const std::shared_ptr<SummonSign>& Sign) { 
            return CanMatchWith(
                Request->matching_parameter(), 
                static_cast<DS3_Frpg2RequestMessage::MatchingParameter&>(*Sign->MatchingParameters.get()), 
                Sign->Type
            );
        });

        for (std::shared_ptr<SummonSign>& Sign : AreaSigns)
        {
            // If client already has sign data we only need to return a limited set of data.
            if (ClientExistingSignId.count(Sign->SignId) > 0)
            {
                DS3_Frpg2RequestMessage::SignInfo* SignInfo = SignResult->add_sign_info_without_data();
                SignInfo->set_player_id(Sign->PlayerId);
                SignInfo->set_sign_id(Sign->SignId);
            }
            else
            {
                DS3_Frpg2RequestMessage::SignData* SignData = SignResult->add_sign_data();
                SignData->mutable_sign_info()->set_player_id(Sign->PlayerId);
                SignData->mutable_sign_info()->set_sign_id(Sign->SignId);
                SignData->set_online_area_id((uint32_t)Sign->OnlineAreaId);
                SignData->mutable_matching_parameter()->CopyFrom(static_cast<DS3_Frpg2RequestMessage::MatchingParameter&>(*Sign->MatchingParameters));
                SignData->set_player_struct(Sign->PlayerStruct.data(), Sign->PlayerStruct.size());
                SignData->set_steam_id(Sign->PlayerSteamId);
                SignData->set_sign_type((DS3_Frpg2RequestMessage::SignType)Sign->Type);
            }

            // Make sure user is marked as aware of the sign so we can clear up when the sign is removed.
            Sign->AwarePlayerIds.insert(Player.GetPlayerId());

            RemainingSignCount--;
        }
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetSignListResponse response.");
        return MessageHandleResult::Error;
    }
    
    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_SignManager::Handle_RequestCreateSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestCreateSign* Request = (DS3_Frpg2RequestMessage::RequestCreateSign*)Message.Protobuf.get();

    // There is no NRSSR struct in the sign metadata, but we still make sure the size-delimited entry list is valid.
    if (BuildConfig::NRSSR_SANITY_CHECKS)
    {
        auto ValidationResult = DS3_NRSSRSanitizer::ValidateEntryList(Request->player_struct().data(), Request->player_struct().size());
        if (ValidationResult != DS3_NRSSRSanitizer::ValidationResult::Valid)
        {
            WarningS(Client->GetName().c_str(), "RequestCreateSign message recieved from client contains ill formated binary data (error code %i).",
                static_cast<uint32_t>(ValidationResult));

            // Simply ignore the request. Perhaps sending a response with an invalid sign id or disconnecting the client would be better?
            return MessageHandleResult::Handled;
        }
    }

    std::shared_ptr<SummonSign> Sign = std::make_shared<SummonSign>();
    Sign->SignId = NextSignId++;
    Sign->OnlineAreaId = (uint32_t)Request->online_area_id();
    Sign->PlayerId = Player.GetPlayerId();
    Sign->PlayerSteamId = Player.GetSteamId();
    Sign->Type = Request->sign_type();
    Sign->PlayerStruct.assign(Request->player_struct().data(), Request->player_struct().data() + Request->player_struct().size());
    Sign->MatchingParameters = std::make_unique<DS3_Frpg2RequestMessage::MatchingParameter>(Request->matching_parameter());

    LiveCache.Add((DS3_OnlineAreaId)Sign->OnlineAreaId, Sign->SignId, Sign);
    Client->ActiveSummonSigns.push_back(Sign);

    DS3_Frpg2RequestMessage::RequestCreateSignResponse Response;
    Response.set_sign_id(Sign->SignId);

    std::string TypeStatisticKey = StringFormat("Sign/TotalCreated");
    Database.AddGlobalStatistic(TypeStatisticKey, 1);
    Database.AddPlayerStatistic(TypeStatisticKey, Player.GetPlayerId(), 1);

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetSignListResponse response.");
        return MessageHandleResult::Error;
    }

    if (ServerInstance->GetConfig().SendDiscordNotice_SummonSign)
    {
        if (Request->matching_parameter().password().empty())
        {
            if (Sign->Type == DS3_Frpg2RequestMessage::SignType_RedSoapstone)
            {
                ServerInstance->SendDiscordNotice(Client->shared_from_this(), DiscordNoticeType::SummonSignPvP,
                    StringFormat("Placed a public red summon sign in '%s'.", GetEnumString((DS3_OnlineAreaId)Sign->OnlineAreaId).c_str()),
                    0,
                    {
                        { "Soul Level", std::to_string(Client->GetPlayerState().GetSoulLevel()), true },
                        { "Weapon Level", std::to_string(Client->GetPlayerState().GetMaxWeaponLevel()), true }
                    }
                );
            }
            else
            {
                ServerInstance->SendDiscordNotice(Client->shared_from_this(), DiscordNoticeType::SummonSign, 
                    StringFormat("Placed a public summon sign in '%s'.", GetEnumString((DS3_OnlineAreaId)Sign->OnlineAreaId).c_str()),
                    0,
                    {
                        { "Soul Level", std::to_string(Client->GetPlayerState().GetSoulLevel()), true },
                        { "Weapon Level", std::to_string(Client->GetPlayerState().GetMaxWeaponLevel()), true }
                    }
                );
            }
        }
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_SignManager::Handle_RequestRemoveSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestRemoveSign* Request = (DS3_Frpg2RequestMessage::RequestRemoveSign*)Message.Protobuf.get();

    std::shared_ptr<SummonSign> Sign = LiveCache.Find((DS3_OnlineAreaId)Request->online_area_id(), Request->sign_id());
    if (!Sign)
    {
        WarningS(Client->GetName().c_str(), "Client as attempted to remove non-existant summon sign, %i, has probably already been cleaned up by CreateSummonSign.", Request->sign_id());
        return MessageHandleResult::Handled;
    }

    if (auto Iter = std::find(Client->ActiveSummonSigns.begin(), Client->ActiveSummonSigns.end(), Sign); Iter != Client->ActiveSummonSigns.end())
    {
        Client->ActiveSummonSigns.erase(Iter);

        // Tell anyone who is aware of this sign that its been removed.
        RemoveSignAndNotifyAware(Sign);
    }
    else
    {
        WarningS(Client->GetName().c_str(), "Client attempted to remove summon sign that didn't belong to them, %i.", Request->sign_id());        
    }

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
    DS3_Frpg2RequestMessage::RequestRemoveSignResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestRemoveSignResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_SignManager::Handle_RequestUpdateSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestUpdateSign* Request = (DS3_Frpg2RequestMessage::RequestUpdateSign*)Message.Protobuf.get();

    // I think the game uses this as something of a hearbeat to keep the sign active in the pool.
    // We keep all players signs active until they are removed, so we don't need to handle this.

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
    DS3_Frpg2RequestMessage::RequestUpdateSignResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestUpdateSignResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_SignManager::Handle_RequestSummonSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestSummonSign* Request = (DS3_Frpg2RequestMessage::RequestSummonSign*)Message.Protobuf.get();

    bool bSuccess = true;

    // Make sure the NRSSR data contained within this message is valid (if the CVE-2022-24126 fix is enabled)
    if (BuildConfig::NRSSR_SANITY_CHECKS)
    {
        auto ValidationResult = DS3_NRSSRSanitizer::ValidateEntryList(Request->player_struct().data(), Request->player_struct().size());
        if (ValidationResult != DS3_NRSSRSanitizer::ValidationResult::Valid)
        {
            WarningS(Client->GetName().c_str(), "RequestSummonSign message recieved from client contains ill formated binary data (error code %i).",
                static_cast<uint32_t>(ValidationResult));

            bSuccess = false;
        }
    }

    // First check the sign still exists, if it doesn't, send a reject message as its probably already used.
    std::shared_ptr<SummonSign> Sign = LiveCache.Find((DS3_OnlineAreaId)Request->online_area_id(), Request->sign_info().sign_id());
    if (!Sign)
    {
        WarningS(Client->GetName().c_str(), "Client attempted to use invalid summon sign, sending back rejection, %i.", Request->sign_info().sign_id());
        bSuccess = false;
    }

    // Sign is already being summoned, send rejection.
    if (Sign && Sign->BeingSummonedByPlayerId != 0)
    {
        WarningS(Client->GetName().c_str(), "Client attempted to use summon sign that is already being summoned, sending back rejection, %i.", Request->sign_info().sign_id());
        bSuccess = false;        
    }

    // Send an accept message to the source client of the sign telling them someone is trying to summon them.
    if (bSuccess)
    {
        std::shared_ptr<GameClient> OriginClient = GameServiceInstance->FindClientByPlayerId(Sign->PlayerId);
        Ensure(OriginClient != nullptr); // Should always be valid if the sign is in the cache.

        DS3_Frpg2RequestMessage::PushRequestSummonSign PushMessage;
        PushMessage.set_push_message_id(DS3_Frpg2RequestMessage::PushID_PushRequestSummonSign);
        PushMessage.mutable_message()->set_player_id(Player.GetPlayerId());
        PushMessage.mutable_message()->set_steam_id(Player.GetSteamId());
        PushMessage.mutable_message()->mutable_sign_info()->set_sign_id(Sign->SignId);
        PushMessage.mutable_message()->mutable_sign_info()->set_player_id(Sign->PlayerId);
        PushMessage.mutable_message()->set_player_struct(Request->player_struct().data(), Request->player_struct().size());

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
    DS3_Frpg2RequestMessage::RequestSummonSignResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestSummonSignResponse response.");
        return MessageHandleResult::Error;
    }

    // If failure then send a reject message.
    if (!bSuccess)
    {
        DS3_Frpg2RequestMessage::PushRequestRejectSign PushMessage;
        PushMessage.set_push_message_id(DS3_Frpg2RequestMessage::PushID_PushRequestRejectSign);
        PushMessage.mutable_message()->set_unknown_2(1);
        PushMessage.mutable_message()->set_sign_id(Request->sign_info().sign_id());

        if (!Client->MessageStream->Send(&PushMessage))
        {
            WarningS(Client->GetName().c_str(), "Failed to send PushRequestRejectSign.");
            return MessageHandleResult::Error;
        }
    }
    else
    {
        std::string PoolStatisticKey = StringFormat("Sign/TotalSummonsRequested/SignType=%u", Sign->Type);
        Database.AddGlobalStatistic(PoolStatisticKey, 1);
        Database.AddPlayerStatistic(PoolStatisticKey, Player.GetPlayerId(), 1);

        std::string TypeStatisticKey = StringFormat("Sign/TotalSummonsRequested");
        Database.AddGlobalStatistic(TypeStatisticKey, 1);
        Database.AddPlayerStatistic(TypeStatisticKey, Player.GetPlayerId(), 1);
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_SignManager::Handle_RequestRejectSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestRejectSign* Request = (DS3_Frpg2RequestMessage::RequestRejectSign*)Message.Protobuf.get();

    // First check the sign still exists, if it doesn't, send a reject message as its probably already used.
    std::shared_ptr<SummonSign> Sign = LiveCache.Find(Request->sign_id());
    if (!Sign)
    {
        WarningS(Client->GetName().c_str(), "Client attempted to reject summoning for invalid sign (or sign cancelled), %i.", Request->sign_id());
        return MessageHandleResult::Handled;
    }

    // Send PushRequestRejectSign response to whichever client attempted to summon them.
    if (Sign->BeingSummonedByPlayerId != 0)
    {
        if (std::shared_ptr<GameClient> OtherClient = GameServiceInstance->FindClientByPlayerId(Sign->BeingSummonedByPlayerId))
        {
            DS3_Frpg2RequestMessage::PushRequestRejectSign PushMessage;
            PushMessage.set_push_message_id(DS3_Frpg2RequestMessage::PushID_PushRequestRejectSign);
            PushMessage.mutable_message()->set_unknown_2(1);
            PushMessage.mutable_message()->set_sign_id(Sign->SignId);

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
    DS3_Frpg2RequestMessage::RequestRejectSignResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestRejectSignResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_SignManager::Handle_RequestGetRightMatchingArea(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    const RuntimeConfig& Config = ServerInstance->GetConfig();

    DS3_Frpg2RequestMessage::RequestGetRightMatchingArea* Request = (DS3_Frpg2RequestMessage::RequestGetRightMatchingArea*)Message.Protobuf.get();
    DS3_Frpg2RequestMessage::RequestGetRightMatchingAreaResponse Response;

    // There are a few ways to handle this. I'm not sure how the server itself does it.
    // The simplest is to go through each client and check if they can match with the user, and use that 
    // to derive the best matching areas. The other option is we use the number of summon signs and recent invasions to do it
    // that might give a more accurate display of what is available for coop?

    std::string Password = Request->matching_parameter().password();
    int SoulLevel = Request->matching_parameter().soul_level();
    int WeaponLevel = Request->matching_parameter().weapon_level();

    std::unordered_map<DS3_OnlineAreaId, int> PotentialAreas;
    int MaxAreaPopulation = 1;

    for (std::shared_ptr<GameClient>& OtherClient : GameServiceInstance->GetClients())
    {
        int OtherSoulLevel = OtherClient->GetPlayerState().GetSoulLevel();
        int OtherWeaponLevel = OtherClient->GetPlayerState().GetMaxWeaponLevel();
        DS3_OnlineAreaId OtherArea = OtherClient->GetPlayerStateType<DS3_PlayerState>().GetCurrentArea();

        if (OtherArea == DS3_OnlineAreaId::None)
        {
            continue;
        }

        if (OtherClient.get() == Client)
        {
            continue;
        }

        // TODO: We don't have the other clients password stored, so we can't take that into account for this, it
        // would be good if we could somehow.

        // Is the client in range to summon or invade us?
        if (Config.SummonSignMatchingParameters.CheckMatch(SoulLevel, WeaponLevel, OtherSoulLevel, OtherWeaponLevel, false) ||
            Config.DarkSpiritInvasionMatchingParameters.CheckMatch(OtherSoulLevel, OtherWeaponLevel, SoulLevel, WeaponLevel, false))
        {
            if (auto Iter = PotentialAreas.find(OtherArea); Iter != PotentialAreas.end())
            {
                MaxAreaPopulation = std::max(MaxAreaPopulation, ++PotentialAreas[OtherArea]);
            }
            else
            {
                PotentialAreas.emplace(OtherArea, 1);
            }
        }
    }

    // Normalize the values to the 0-5 range the client expects and return them.
    for (auto Pair : PotentialAreas)
    {
        int NormalizedPopulation = (int)std::ceil((Pair.second / (float)MaxAreaPopulation) * 5.0f);

        DS3_Frpg2RequestMessage::RequestGetRightMatchingAreaResponse_Area_info& Info = *Response.add_area_info();
        Info.set_online_area_id((uint32_t)Pair.first);
        Info.set_population(NormalizedPopulation);
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetRightMatchingArea response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string DS3_SignManager::GetName()
{
    return "Signs";
}
