/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Server/WebUIService/Handlers/WebUIHandler.h"
#include "Server/GameService/PlayerState.h"

#include <mutex>

class GameClient;

// /players
//
//		GET	- Gets a list of players and their current states.

class PlayersHandler : public WebUIHandler
{
public:
	PlayersHandler(WebUIService* InService);

	virtual bool handleGet(CivetServer* Server, struct mg_connection* Connection) override;
	virtual bool handleDelete(CivetServer* Server, struct mg_connection* Connection) override;

	virtual void Register(CivetServer* Server) override;

	virtual void GatherData() override;

protected:

	struct PlayerInfo
	{
		//PlayerState State; 

		std::string SteamId;
		uint32_t PlayerId;
		std::string CharacterName;
		size_t DeathCount;
		size_t MultiplayCount;
		size_t SoulLevel;
		size_t Souls;
		size_t SoulMemory;
		std::string CovenantState;
		uint32_t OnlineArea;
		std::string Status;
		double PlayTime;
		double AntiCheatScore;

		double ConnectionDuration;
	};

	void GatherPlayerInfo(PlayerInfo& Info, std::shared_ptr<GameClient> Client);

	std::mutex DataMutex;
	std::vector<PlayerInfo> PlayerInfos;

};