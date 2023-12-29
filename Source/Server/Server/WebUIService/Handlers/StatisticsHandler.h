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

// /statistics
//
//		GET	- Gets some general statistics about the server.

class StatisticsHandler : public WebUIHandler
{
public:
	StatisticsHandler(WebUIService* InService);

	virtual bool handleGet(CivetServer* Server, struct mg_connection* Connection) override;

	virtual void Register(CivetServer* Server) override;

	virtual void GatherData() override;

protected:

	struct Sample
	{
		std::string Timestamp;
		size_t ActivePlayers;
	};

	std::mutex DataMutex;

	std::vector<Sample> Samples;
	double NextSampleTime = 0.0;
	double MustSampleTime = 0.0;

	std::unordered_map<std::string, std::string> Statistics;
	std::map<uint32_t, size_t> PopulatedAreas; 

	size_t UniquePlayerCount = 0;

	size_t PreviousSampleClientSize = 0;

	constexpr static inline double MinSampleInterval = 60.0f;
	constexpr static inline double MaxSampleInterval = 30 * 60.0f;
	constexpr static inline size_t MaxSamples = 60 * 24 * 7; // One week of samples.

};