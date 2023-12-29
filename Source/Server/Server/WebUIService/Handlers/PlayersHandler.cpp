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
#include "Shared/Core/Network/NetConnection.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"

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
    auto& State = Client->GetPlayerState();

    Info.ConnectionDuration = Client->GetConnectionDuration();

    Info.SteamId = State.GetSteamId();
    Info.PlayerId = State.GetPlayerId();
    Info.CharacterName = State.GetCharacterName();
    Info.SoulLevel = State.GetSoulLevel();
    Info.OnlineArea = State.GetCurrentAreaId();
    Info.AntiCheatScore = State.GetAntiCheatState().Penalty;
    Info.Souls = State.GetSoulCount();
    Info.SoulMemory = State.GetSoulMemory();
    Info.DeathCount = State.GetDeathCount();
    Info.MultiplayCount = State.GetMultiplayerSessionCount();
    Info.PlayTime = State.GetPlayTime();
    Info.Status = "Unknown";
    Info.CovenantState = State.GetConvenantStatusDescription();
    Info.Status = State.GetStatusDescription();
}

void PlayersHandler::GatherData()
{
    std::scoped_lock lock(DataMutex);

    std::shared_ptr<GameService> Game = Service->GetServer()->GetService<GameService>();
    std::vector<std::shared_ptr<GameClient>> Clients = Game->GetClients();

    std::vector<std::shared_ptr<GameClient>> ValidClients;
    for (auto Client : Clients)
    {
        auto& State = Client->GetPlayerState();

        if (!State.IsInGame())
        {
            continue;
        }

        ValidClients.push_back(Client);
    }

    // Add new clients.
    for (auto Client : ValidClients)
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
    auto iter = std::remove_if(PlayerInfos.begin(), PlayerInfos.end(), [&ValidClients](PlayerInfo& Info)
    {    
        auto Iter = std::find_if(ValidClients.begin(), ValidClients.end(), [Info](std::shared_ptr<GameClient>& Client){
            return Info.SteamId == Client->GetPlayerState().GetSteamId();
        });

        return Iter == ValidClients.end();
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

            uint64_t SteamId64;
            sscanf(Info.SteamId.c_str(), "%016llx", &SteamId64);

            playerJson["steamId64"] = std::to_string(SteamId64);
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
            playerJson["location"] = Service->GetServer()->GetGameInterface().GetAreaName(Info.OnlineArea);
            playerJson["connectionTime"] = SecondsToString(Info.ConnectionDuration);
            playerJson["playTime"] = SecondsToString(Info.PlayTime);
            playerJson["antiCheatScore"] = Info.AntiCheatScore;

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