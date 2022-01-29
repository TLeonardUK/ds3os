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

// /players
//
//		GET	- Gets a list of players and their current states.

class PlayersHandler : public WebUIHandler
{
public:
	PlayersHandler(WebUIService* InService);

	virtual bool handleGet(CivetServer* Server, struct mg_connection* Connection) override;

	virtual void Register(CivetServer* Server) override;

	virtual void GatherData() override;

protected:

	struct PlayerInfo
	{
		PlayerState State; 
		double ConnectionDuration;
	};

	std::mutex DataMutex;
	std::vector<PlayerInfo> PlayerInfos;

};