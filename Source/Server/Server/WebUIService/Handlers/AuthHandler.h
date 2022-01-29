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

// /auth
//
//		GET	- Checks validity of token.
//		params: token
//		returns: 200 on valid, 401 if not valid
//
//		POST - Creates a new token from username/password (logs user in)
//		params: username, password
//		returns: token, or 401 if not valid

class AuthHandler : public WebUIHandler
{
public:
	AuthHandler(WebUIService* InService);

	virtual bool handleGet(CivetServer* Server, struct mg_connection* Connection) override;
	virtual bool handlePost(CivetServer* Server, struct mg_connection* Connection) override;

	virtual void Register(CivetServer* Server) override;


};