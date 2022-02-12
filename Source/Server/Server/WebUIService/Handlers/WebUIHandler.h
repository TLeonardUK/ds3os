/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Server/WebUIService/WebUIService.h"

#include "ThirdParty/nlohmann/json.hpp"

class WebUIHandler : public CivetHandler
{
public:
	WebUIHandler(WebUIService* InService);

	// Used by derived classes to register all uri handlers.
	virtual void Register(CivetServer* Server) = 0;

	// Called continually on the main thread, should be used to gather
	// any data that may need to be provided to the web-ui. Simplifies
	// multithreading as most of the server data isn't thread-safe.
	virtual void GatherData() {};

	// Determines if GatherData needs to be called for this handler.
	virtual bool NeedsDataGather();

	// Marks this handler as needing data gather callbacks as data has been requested.
	virtual void MarkAsNeedsDataGather();

protected:
	void RespondJson(struct mg_connection* Connection, nlohmann::json& Json);

	bool ReadJson(CivetServer* Server, struct mg_connection* Connection, nlohmann::json& Json);

protected:
	WebUIService* Service;
	
	double LastMarkedAsNeedingDataGather = 0.0f;
	double LastDataGather = 0.0f;

};