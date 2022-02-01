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

// /message
//
//		POST - Sends a message to a player (or all players)

class MessageHandler : public WebUIHandler
{
public:
	MessageHandler(WebUIService* InService);

	virtual bool handlePost(CivetServer* Server, struct mg_connection* Connection) override;

	virtual void Register(CivetServer* Server) override;

protected:

};