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

// /bans
//
//		GET		- Gets a list of all active bans.
//		DELETE	- Removes the ban for a specific steam-id.

class BansHandler : public WebUIHandler
{
public:
	BansHandler(WebUIService* InService);

	virtual bool handleGet(CivetServer* Server, struct mg_connection* Connection) override;
	virtual bool handleDelete(CivetServer* Server, struct mg_connection* Connection) override;

	virtual void Register(CivetServer* Server) override;
};