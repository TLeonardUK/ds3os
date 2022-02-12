/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/Server.h"
#include "Server/GameService/GameService.h"
#include "Server/GameService/GameClient.h"
#include "Server/WebUIService/Handlers/PlayersHandler.h"
#include "Server/Core/Network/NetConnection.h"

#include "Core/Utils/Logging.h"
#include "Core/Utils/Strings.h"

PlayersHandler::PlayersHandler(WebUIService* InService)
    : WebUIHandler(InService)
{
} 

void PlayersHandler::Register(CivetServer* Server)
{
    Server->addHandler("/players", this);
}


void PlayersHandler::GatherPlayerInfo(PlayerInfo& Info, std::shared_ptr<GameClient> Client)
{
    auto State = Client->GetPlayerState();

    Info.ConnectionDuration = Client->GetConnectionDuration();

    if (!State.GetPlayerStatus().has_player_status() ||
        !State.GetPlayerStatus().has_play_data() ||
        State.GetPlayerId() == 0)
    {
        return;
    }

    auto Status = State.GetPlayerStatus().player_status();
    auto LogInfo = State.GetPlayerStatus().log_info();

    Info.SteamId = State.GetSteamId();
    Info.PlayerId = State.GetPlayerId();
    Info.CharacterName = State.GetCharacterName();
    Info.DeathCount = LogInfo.death_count();
    Info.MultiplayCount = LogInfo.multiplay_count();
    Info.SoulLevel = State.GetSoulLevel();
    Info.Souls = Status.souls();
    Info.SoulMemory = Status.soul_memory();
    Info.OnlineArea = State.GetCurrentArea();
    Info.PlayTime = State.GetPlayerStatus().play_data().play_time_seconds();    
    Info.Status = "Unknown";
    Info.CovenantState = GetEnumString<CovenantId>((CovenantId)Status.covenant());

    switch ((CovenantId)Status.covenant())
    {
    case CovenantId::Blade_of_the_Darkmoon:
    case CovenantId::Blue_Sentinels:
        {
            if (Status.has_can_summon_for_way_of_blue() && Status.can_summon_for_way_of_blue())
            {
                Info.CovenantState += " (Summonable)";
            }
            break;
        }
    case CovenantId::Watchdogs_of_Farron:
        {
            if (Status.has_can_summon_for_watchdog_of_farron() && Status.can_summon_for_watchdog_of_farron())
            {
                Info.CovenantState += " (Summonable)";
            }
            break;
        }
    case CovenantId::Aldrich_Faithfuls:
        {
            if (Status.has_can_summon_for_aldritch_faithful() && Status.can_summon_for_aldritch_faithful())
            {
                Info.CovenantState += " (Summonable)";
            }
            break;
        }
    case CovenantId::Spears_of_the_Church:
        {
            if (Status.has_can_summon_for_spear_of_church() && Status.can_summon_for_spear_of_church())
            {
                Info.CovenantState += " (Summonable)";
            }
            break;
        }
    }

    switch (Status.world_type())
    {
    case Frpg2PlayerData::WorldType::WorldType_None:
        {
        Info.Status = "Loading or in menus";
            break;
        }
    case Frpg2PlayerData::WorldType::WorldType_Multiplayer:
        {
            if (Status.net_mode() == Frpg2PlayerData::NetMode::NetMode_None)
            {
                Info.Status = "Multiplayer alone";
            }
            else if (Status.net_mode() == Frpg2PlayerData::NetMode::NetMode_Host)
            {
                InvasionTypeId TypeId = (InvasionTypeId)Status.invasion_type();
                switch (TypeId)
                {
                case InvasionTypeId::Summon_White:
                case InvasionTypeId::Summon_Red:
                case InvasionTypeId::Summon_Purple_White:
                case InvasionTypeId::Avatar:
                case InvasionTypeId::Arena_Battle_Royal:
                case InvasionTypeId::Umbasa_White:
                case InvasionTypeId::Summon_Sunlight_White:
                case InvasionTypeId::Summon_Sunlight_Dark:
                case InvasionTypeId::Summon_Purple_Dark:
                case InvasionTypeId::Covenant_Blade_of_the_Darkmoon:
                case InvasionTypeId::Blue_Sentinel:
                case InvasionTypeId::Force_Join_Session:
                {
                    Info.Status = "Hosting cooperation with " + GetEnumString<InvasionTypeId>(TypeId);
                    break;
                }
                case InvasionTypeId::Red_Hunter:
                case InvasionTypeId::Invade_Red:
                case InvasionTypeId::Covenant_Spear_of_the_Church:
                case InvasionTypeId::Guardian_of_Rosaria:
                case InvasionTypeId::Covenant_Watchdog_of_Farron:
                case InvasionTypeId::Covenant_Aldrich_Faithful:
                case InvasionTypeId::Invade_Sunlight_Dark:
                case InvasionTypeId::Invade_Purple_Dark:
                {
                    Info.Status = "Being invaded by " + GetEnumString<InvasionTypeId>(TypeId);
                    break;
                }
                }
            }
            else if (Status.net_mode() == Frpg2PlayerData::NetMode::NetMode_Client)
            {
                Info.Status = "In another world";
            }
            break;
        }
    case Frpg2PlayerData::WorldType::WorldType_Singleplayer:
        {
            std::string status = "Playing solo";
            if (Status.is_invadable())
            {
                status += " (Invadable)";
            }
            Info.Status = status;
            break;
        }
    }
}

void PlayersHandler::GatherData()
{
    std::scoped_lock lock(DataMutex);

    std::shared_ptr<GameService> Game = Service->GetServer()->GetService<GameService>();
    std::vector<std::shared_ptr<GameClient>> Clients = Game->GetClients();

    // Add new clients.
    for (auto Client : Clients)
    {
        auto Iter = std::find_if(PlayerInfos.begin(), PlayerInfos.end(), [&Client](PlayerInfo& Info)
        {        
            return Info.SteamId == Client->GetPlayerState().GetSteamId();
        });

        if (Iter == PlayerInfos.end())
        {
            PlayerInfo Info;
            GatherPlayerInfo(Info, Client);
            PlayerInfos.push_back(Info);
        }

        // Update existing client if state has mutated.
        else
        {
            PlayerInfo& ExistingInfo = *Iter;
            GatherPlayerInfo(ExistingInfo, Client);
        }
    }

    // Remove clients who have left.
    auto iter = std::remove_if(PlayerInfos.begin(), PlayerInfos.end(), [&Clients](PlayerInfo& Info)
    {    
        auto Iter = std::find_if(Clients.begin(), Clients.end(), [Info](std::shared_ptr<GameClient>& Client){        
            return Info.SteamId == Client->GetPlayerState().GetSteamId();
        });

        return Iter == Clients.end();
    });
    PlayerInfos.erase(iter, PlayerInfos.end());
}

bool PlayersHandler::handleGet(CivetServer* Server, struct mg_connection* Connection)
{
    if (!Service->IsAuthenticated(Connection))
    {
        mg_send_http_error(Connection, 401, "Token invalid.");
        return true;
    }

    nlohmann::json json;
    {
        std::scoped_lock lock(DataMutex);

        auto playerArray = nlohmann::json::array();
        for (PlayerInfo& Info : PlayerInfos)
        {
            auto SecondsToString = [](double Time) {
                uint64_t Total = (uint64_t)Time;
                uint64_t Seconds = Total % 60;
                uint64_t Minutes = (Total / 60) % 60;
                uint64_t Hours = (Total / 60) / 60;
                return StringFormat("%i:%i:%i", Hours, Minutes, Seconds);
            };

            auto playerJson = nlohmann::json::object();
            playerJson["steamId"] = Info.SteamId;
            playerJson["playerId"] = Info.PlayerId;
            playerJson["characterName"] = Info.CharacterName;            
            playerJson["deathCount"] = Info.DeathCount;
            playerJson["multiplayCount"] = Info.MultiplayCount;
            playerJson["soulLevel"] = Info.SoulLevel;            
            playerJson["souls"] = Info.Souls;
            playerJson["soulMemory"] = Info.SoulMemory;
            playerJson["status"] = Info.Status;
            playerJson["covenant"] = Info.CovenantState;
            playerJson["location"] = GetEnumString<OnlineAreaId>(Info.OnlineArea);
            playerJson["connectionTime"] = SecondsToString(Info.ConnectionDuration);
            playerJson["playTime"] = SecondsToString(Info.PlayTime);

            playerArray.push_back(playerJson);
        }

        json["players"] = playerArray;   
    }

    RespondJson(Connection, json);

    MarkAsNeedsDataGather();

    return true;
}

bool PlayersHandler::handleDelete(CivetServer* Server, struct mg_connection* Connection)
{
    if (!Service->IsAuthenticated(Connection))
    {
        mg_send_http_error(Connection, 401, "Token invalid.");
        return true;
    }

    nlohmann::json json;
    if (!ReadJson(Server, Connection, json) ||
        !json.contains("playerId") ||
        !json.contains("ban"))
    {
        mg_send_http_error(Connection, 400, "Malformed body.");
        return true;
    }

    uint32_t playerId = json["playerId"];
    bool ban = json["ban"];

    ServerDatabase& Database = Service->GetServer()->GetDatabase();
    std::shared_ptr<GameService> Game = Service->GetServer()->GetService<GameService>();    
    if (std::shared_ptr<GameClient> Client = Game->FindClientByPlayerId(playerId))
    {
        if (ban)
        {
            LogS("WebUI", "Banning player: %i", Client->GetPlayerState().GetPlayerId());

            Database.BanPlayer(Client->GetPlayerState().GetSteamId());
        }
        else
        {
            LogS("WebUI", "Disconnected player: %i", Client->GetPlayerState().GetPlayerId());
        }

        Client->Connection->Disconnect();
    }

    nlohmann::json responseJson;
    RespondJson(Connection, responseJson);

    return true;
}