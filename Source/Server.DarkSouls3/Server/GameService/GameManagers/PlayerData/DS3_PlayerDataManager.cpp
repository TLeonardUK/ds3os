/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/PlayerData/DS3_PlayerDataManager.h"
#include "Server/GameService/DS3_PlayerState.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/DS3_Frpg2ReliableUdpMessage.h"
#include "Server/GameService/Utils/DS3_GameIds.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"
#include "Shared/Core/Utils/DiffTracker.h"

#include "Shared/Core/Network/NetConnection.h"

DS3_PlayerDataManager::DS3_PlayerDataManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

MessageHandleResult DS3_PlayerDataManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestUpdateLoginPlayerCharacter))
    {
        return Handle_RequestUpdateLoginPlayerCharacter(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestUpdatePlayerStatus))
    {
        return Handle_RequestUpdatePlayerStatus(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestUpdatePlayerCharacter))
    {
        return Handle_RequestUpdatePlayerCharacter(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestGetPlayerCharacter))
    {
        return Handle_RequestGetPlayerCharacter(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestGetLoginPlayerCharacter))
    {
        return Handle_RequestGetLoginPlayerCharacter(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestGetPlayerCharacterList))
    {
        return Handle_RequestGetPlayerCharacterList(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult DS3_PlayerDataManager::Handle_RequestUpdateLoginPlayerCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& State = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestUpdateLoginPlayerCharacter* Request = (DS3_Frpg2RequestMessage::RequestUpdateLoginPlayerCharacter*)Message.Protobuf.get();

    std::shared_ptr<Character> Character = Database.FindCharacter(State.GetPlayerId(), Request->character_id());
    if (!Character)
    {
        std::vector<uint8_t> Data;        
        if (!Database.CreateOrUpdateCharacter(State.GetPlayerId(), Request->character_id(), Data))
        {
            WarningS(Client->GetName().c_str(), "Disconnecting client as failed to find or update character %i.", Request->character_id());
            return MessageHandleResult::Error;
        }

        Character = Database.FindCharacter(State.GetPlayerId(), Request->character_id());
        Ensure(Character);
    }

    DS3_Frpg2RequestMessage::RequestUpdateLoginPlayerCharacterResponse Response;
    Response.set_character_id(Request->character_id());

    DS3_Frpg2RequestMessage::QuickMatchRank* Rank = Response.mutable_quickmatch_brawl_rank();
    Rank->set_rank(Character->QuickMatchBrawlRank);
    Rank->set_xp(Character->QuickMatchBrawlXp);

    Rank = Response.mutable_quickmatch_dual_rank();
    Rank->set_rank(Character->QuickMatchDuelRank);
    Rank->set_xp(Character->QuickMatchDuelXp);

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestUpdateLoginPlayerCharacterResponse response.");
        return MessageHandleResult::Error;
    }
    
    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_PlayerDataManager::Handle_RequestUpdatePlayerStatus(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestUpdatePlayerStatus* Request = (DS3_Frpg2RequestMessage::RequestUpdatePlayerStatus*)Message.Protobuf.get();

    auto& State = Client->GetPlayerStateType<DS3_PlayerState>();

    // Merge the delta into the current state.
    std::string bytes = Request->status();

    DS3_Frpg2PlayerData::AllStatus status;
    if (!status.ParseFromArray(bytes.data(), (int)bytes.size()))
    {
        WarningS(Client->GetName().c_str(), "Failed to parse DS3_Frpg2PlayerData::AllStatus from RequestUpdatePlayerStatus.");

        // Don't take this as an error, it will resolve itself on next send.
        return MessageHandleResult::Handled;
    }

    // MergeFrom combines arrays, so we need to do some fuckyness here to handle this.
    if (status.has_player_status() && status.player_status().played_areas_size() > 0)
    {
        State.GetPlayerStatus_Mutable().mutable_player_status()->clear_played_areas();
    }
    if (status.has_player_status() && status.player_status().unknown_18_size() > 0)
    {
        State.GetPlayerStatus_Mutable().mutable_player_status()->clear_unknown_18();
    }
    if (status.has_player_status() && status.player_status().anticheat_data_size() > 0)
    {
        State.GetPlayerStatus_Mutable().mutable_player_status()->clear_anticheat_data();
    }
    State.GetPlayerStatus_Mutable().MergeFrom(status);

    // Keep track of the players character id.
    if (State.GetPlayerStatus().player_status().has_character_id())
    {
        State.SetCharacterId(State.GetPlayerStatus().player_status().character_id());
    }

    // Keep track of the players character name, useful for logging.
    if (State.GetPlayerStatus().player_status().has_name())
    {
        std::string NewCharacterName = State.GetPlayerStatus().player_status().name();
        if (State.GetCharacterName() != NewCharacterName)
        {
            State.SetCharacterName(NewCharacterName);

            std::string NewConnectionName = StringFormat("%i:%s", State.GetPlayerId(), State.GetCharacterName().c_str());

            LogS(Client->GetName().c_str(), "Renaming connection to '%s'.", NewConnectionName.c_str());

            // Rename connection after this point as it easier to keep track of than ip:port pairs.
            Client->Connection->Rename(NewConnectionName);
        }
    }

    // Print a log if user has changed online location.
    if (State.GetPlayerStatus().has_player_location())
    {
        DS3_OnlineAreaId AreaId = static_cast<DS3_OnlineAreaId>(State.GetPlayerStatus().player_location().online_area_id());
        if (AreaId != State.GetCurrentArea() && AreaId != DS3_OnlineAreaId::None)
        {
            VerboseS(Client->GetName().c_str(), "User has entered '%s' (0x%08x)", GetEnumString(AreaId).c_str(), AreaId);
            State.SetCurrentArea(AreaId);
        }
    }

    // Grab some matchmaking values.
    if (State.GetPlayerStatus().has_player_status())
    {
        // Grab invadability state.
        if (State.GetPlayerStatus().player_status().has_is_invadable())
        {
            bool NewState = State.GetPlayerStatus().player_status().is_invadable();
            if (NewState != State.GetIsInvadable())
            {
                VerboseS(Client->GetName().c_str(), "User is now %s", NewState ? "invadable" : "no longer invadable");
                State.SetIsInvadable(NewState);
            }
        }

        // Grab soul level / weapon level.
        if (State.GetPlayerStatus().player_status().has_soul_level())
        {
            State.SetSoulLevel(State.GetPlayerStatus().player_status().soul_level());
        }
        if (State.GetPlayerStatus().player_status().has_max_weapon_level())
        {
            State.SetMaxWeaponLevel(State.GetPlayerStatus().player_status().max_weapon_level());
        }

        // Grab whatever visitor pool they should be in.
        DS3_Frpg2RequestMessage::VisitorPool NewVisitorPool = DS3_Frpg2RequestMessage::VisitorPool::VisitorPool_None;
        if (State.GetPlayerStatus().player_status().has_can_summon_for_way_of_blue() && State.GetPlayerStatus().player_status().can_summon_for_way_of_blue())
        {
            NewVisitorPool = DS3_Frpg2RequestMessage::VisitorPool::VisitorPool_Way_of_Blue;
        }
        if (State.GetPlayerStatus().player_status().has_can_summon_for_watchdog_of_farron() && State.GetPlayerStatus().player_status().can_summon_for_watchdog_of_farron())
        {
            NewVisitorPool = DS3_Frpg2RequestMessage::VisitorPool::VisitorPool_Watchdog_of_Farron;
        }
        if (State.GetPlayerStatus().player_status().has_can_summon_for_aldritch_faithful() && State.GetPlayerStatus().player_status().can_summon_for_aldritch_faithful())
        {
            NewVisitorPool = DS3_Frpg2RequestMessage::VisitorPool::VisitorPool_Aldrich_Faithful;
        }
        if (State.GetPlayerStatus().player_status().has_can_summon_for_spear_of_church() && State.GetPlayerStatus().player_status().can_summon_for_spear_of_church())
        {
            NewVisitorPool = DS3_Frpg2RequestMessage::VisitorPool::VisitorPool_Spear_of_the_Church;
        }

        if (NewVisitorPool != State.GetVisitorPool())
        {
            State.SetVisitorPool(NewVisitorPool);
        }
    }

#if 0//def _DEBUG
    static DiffTracker Tracker;
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_id_2", State.GetPlayerStatus().player_status().unknown_2());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_id_6", State.GetPlayerStatus().player_status().unknown_6());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_id_9", State.GetPlayerStatus().player_status().unknown_9());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_id_14", State.GetPlayerStatus().player_status().unknown_14());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_id_32", State.GetPlayerStatus().player_status().unknown_32());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_id_33", State.GetPlayerStatus().player_status().unknown_33());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_id_63", State.GetPlayerStatus().player_status().unknown_63());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_id_76", State.GetPlayerStatus().player_status().unknown_76());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_id_78", State.GetPlayerStatus().player_status().unknown_78());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_id_80", State.GetPlayerStatus().player_status().unknown_80());
    for (int i = 0; i < State.GetPlayerStatus().player_status().anticheat_data_size(); i++)
    {
        Tracker.Field(State.GetCharacterName().c_str(), StringFormat("PlayerStatus.player_status.anticheat[%i]", i), State.GetPlayerStatus().player_status().anticheat_data(i));
    }
    for (int i = 0; i < State.GetPlayerStatus().player_status().unknown_18_size(); i++)
    {
        Tracker.Field(State.GetCharacterName().c_str(), StringFormat("PlayerStatus.player_status.unknown_18[%i]", i), State.GetPlayerStatus().player_status().unknown_18(i));
    }
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.equipment.unknown_id_59", State.GetPlayerStatus().equipment().unknown_59());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.equipment.unknown_id_60", State.GetPlayerStatus().equipment().unknown_60());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats.unknown_id_1", State.GetPlayerStatus().stats_info().unknown_1());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats.unknown_id_2", State.GetPlayerStatus().stats_info().unknown_2());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats.unknown_id_3", State.GetPlayerStatus().stats_info().unknown_3());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats.unknown_id_4", State.GetPlayerStatus().stats_info().unknown_4());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats.unknown_id_5", State.GetPlayerStatus().stats_info().unknown_5());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats.unknown_id_6", State.GetPlayerStatus().stats_info().unknown_6());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.play_data.unknown_id_4", State.GetPlayerStatus().play_data().unknown_4());
#endif

    DS3_Frpg2RequestMessage::RequestUpdatePlayerStatusResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestUpdatePlayerStatusResponse response.");
        return MessageHandleResult::Error;
    }

    // Send discord notification when the user lights a bonfire.
    if (State.GetPlayerStatus().has_play_data())
    {
        std::vector<uint32_t>& litBonfires = State.GetLitBonfires_Mutable();

        for (size_t i = 0; i < State.GetPlayerStatus().play_data().bonfire_info_size(); i++)
        {
            auto& bonfireInfo = State.GetPlayerStatus().play_data().bonfire_info(i);
            if (bonfireInfo.has_been_lit())
            {
                uint32_t bonfireId = bonfireInfo.bonfire_id();
                if (auto Iter = std::find(litBonfires.begin(), litBonfires.end(), bonfireId); Iter == litBonfires.end())
                {
                    if (State.GetHasInitialState())
                    {
                        LogS(Client->GetName().c_str(), "Has lit bonfire %i.", bonfireId);

                        std::string BonfireName = GetEnumString<DS3_BonfireId>((DS3_BonfireId)bonfireId);

                        if (!BonfireName.empty())
                        {
                            ServerInstance->SendDiscordNotice(Client->shared_from_this(), DiscordNoticeType::BonfireLit,
                                StringFormat("Lit the '%s' bonfire.", BonfireName.c_str())
                            );
                        }
                    }

                    litBonfires.push_back(bonfireId);
                }
            }
        }
    }

    State.SetHasInitialState(true);

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_PlayerDataManager::Handle_RequestUpdatePlayerCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& State = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestUpdatePlayerCharacter* Request = (DS3_Frpg2RequestMessage::RequestUpdatePlayerCharacter*)Message.Protobuf.get();

    std::vector<uint8_t> Data;
    Data.assign(Request->character_data().data(), Request->character_data().data() + Request->character_data().size());

    if (!Database.CreateOrUpdateCharacter(State.GetPlayerId(), Request->character_id(), Data))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to find or update character %i.", Request->character_id());
        return MessageHandleResult::Error;
    }

    DS3_Frpg2RequestMessage::RequestUpdatePlayerCharacterResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestUpdatePlayerCharacterResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_PlayerDataManager::Handle_RequestGetPlayerCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& State = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestGetPlayerCharacter* Request = (DS3_Frpg2RequestMessage::RequestGetPlayerCharacter*)Message.Protobuf.get();
    DS3_Frpg2RequestMessage::RequestGetPlayerCharacterResponse Response;

    std::vector<uint8_t> CharacterData;
    std::shared_ptr<Character> Character = Database.FindCharacter(Request->player_id(), Request->character_id());
    if (Character)
    {
        CharacterData = Character->Data;
    }

    Response.set_player_id(Request->player_id());
    Response.set_character_id(Request->character_id());
    Response.set_character_data(CharacterData.data(), CharacterData.size());

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetPlayerCharacterResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_PlayerDataManager::Handle_RequestGetLoginPlayerCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestGetLoginPlayerCharacter* Request = (DS3_Frpg2RequestMessage::RequestGetLoginPlayerCharacter*)Message.Protobuf.get();

    // TODO: Implement
    Ensure(false); // Never seen this in use.

    DS3_Frpg2RequestMessage::RequestGetLoginPlayerCharacterResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetLoginPlayerCharacterResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_PlayerDataManager::Handle_RequestGetPlayerCharacterList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestGetPlayerCharacterList* Request = (DS3_Frpg2RequestMessage::RequestGetPlayerCharacterList*)Message.Protobuf.get();

    // TODO: Implement
    Ensure(false); // Never seen this in use.

    DS3_Frpg2RequestMessage::RequestGetPlayerCharacterListResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetPlayerCharacterListResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string DS3_PlayerDataManager::GetName()
{
    return "Player Data";
}
