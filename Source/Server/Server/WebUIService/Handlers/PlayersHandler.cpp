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

void PlayersHandler::GatherData()
{
    std::scoped_lock lock(DataMutex);

    std::shared_ptr<GameService> Game = Service->GetServer()->GetService<GameService>();
    std::vector<std::shared_ptr<GameClient>> Clients = Game->GetClients();

    PlayerInfos.clear();
    for (auto Client : Clients)
    {        
        PlayerInfo Info;
        Info.State = Client->GetPlayerState();
        Info.ConnectionDuration = Client->GetConnectionDuration();
        PlayerInfos.push_back(Info);
    }
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
            if (!Info.State.PlayerStatus.has_player_status() ||
                Info.State.PlayerId == 0)
            {
                continue;
            }

            const Frpg2PlayerData::PlayerStatus& playerStatus = Info.State.PlayerStatus.player_status();
            const Frpg2PlayerData::LogInfo& logInfo = Info.State.PlayerStatus.log_info();

            uint64_t Total = (uint64_t)Info.ConnectionDuration;
            uint64_t Seconds = Total % 60;
            uint64_t Minutes = (Total / 60) % 60;
            uint64_t Hours = (Total / 60) / 60;

            auto playerJson = nlohmann::json::object();
            playerJson["steamId"] = Info.State.SteamId;
            playerJson["playerId"] = Info.State.PlayerId;
            playerJson["characterName"] = Info.State.CharacterName;            
            playerJson["deathCount"] = logInfo.death_count();
            playerJson["multiplayCount"] = logInfo.multiplay_count();
            playerJson["soulLevel"] = Info.State.SoulLevel;            
            playerJson["souls"] = playerStatus.souls();
            playerJson["soulMemory"] = playerStatus.soul_memory();
            playerJson["covenant"] = GetEnumString<CovenantId>((CovenantId)playerStatus.covenant());
            playerJson["location"] = GetEnumString<OnlineAreaId>(Info.State.CurrentArea);
            playerJson["status"] = "Unknown";

            switch (playerStatus.world_type())
            {
            case Frpg2PlayerData::WorldType::WorldType_Multiplayer:
                {
                    if (playerStatus.net_mode() == Frpg2PlayerData::NetMode::NetMode_Host)
                    {
                        playerJson["status"] = "Cooperating with another player";
                    }
                    else
                    {
                        InvasionTypeId TypeId = (InvasionTypeId)playerStatus.invasion_type();
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
                        case InvasionTypeId::Red_Hunter:
                        case InvasionTypeId::Force_Join_Session:
                            {
                                playerJson["status"] = "Cooperating as a " + GetEnumString<InvasionTypeId>(TypeId);
                                break;
                            }
                        case InvasionTypeId::Invade_Red:
                        case InvasionTypeId::Covenant_Spear_of_the_Church:
                        case InvasionTypeId::Guardian_of_Rosaria:
                        case InvasionTypeId::Covenant_Watchdog_of_Farron:
                        case InvasionTypeId::Covenant_Aldrich_Faithful:
                        case InvasionTypeId::Invade_Sunlight_Dark:
                        case InvasionTypeId::Invade_Purple_Dark:
                            {
                                playerJson["status"] = "Invading as a " + GetEnumString<InvasionTypeId>(TypeId);
                                break;
                            }
                        }
                    }
                    break;
                }
            case Frpg2PlayerData::WorldType::WorldType_Singleplayer:
                {
                    std::string status = "Playing solo";
                    if (playerStatus.is_invadable())
                    {
                        status += ", Invadable";
                    }
                    playerJson["status"] = status;
                    break;
                }
            }

            playerJson["connectionTime"] = StringFormat("%i:%i:%i", Hours, Minutes, Seconds);

            playerArray.push_back(playerJson);
        }

        json["players"] = playerArray;   
    }

    RespondJson(Connection, json);

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
            LogS("WebUI", "Banning player: %i", Client->GetPlayerState().PlayerId);

            Database.BanPlayer(Client->GetPlayerState().SteamId);
        }
        else
        {
            LogS("WebUI", "Disconnected player: %i", Client->GetPlayerState().PlayerId);
        }

        Client->Connection->Disconnect();
    }

    nlohmann::json responseJson;
    RespondJson(Connection, responseJson);

    return true;
}