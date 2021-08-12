/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <string>
#include <filesystem>
#include <nlohmann/json.hpp>

// Wraps up the values for an announcement show to the user when they join the game.
struct RuntimeConfigAnnouncement
{
    std::string Header;
    std::string Body;

    bool Serialize(nlohmann::json& Json, bool Loading);
};

// Configuration saved and loaded at runtime by the server from a configuration file.
class RuntimeConfig
{
public:

    // Name used in the server import file.
    std::string ServerName = "Dark Souls 3 Server";

    // Description used in the server import file.
    std::string ServerDescription = "A custom Dark Souls 3 server.";

    // Hostname of the server that should be used for connecting.
    std::string ServerHostname = "127.0.0.1";

    // IP of the server that should be used for connecting. 
    // TODO: If none is supplied then the IP will be the resolved IP of ServerHostname.
    // TODO: Private ip of server will be returned rather than this if client is on same subnet.
    std::string ServerIP = "127.0.0.1";

    // Network port the login server listens for connections on.
    int LoginServerPort = 50050;

    // Network port the authentication server listens for connections on.
    int AuthServerPort = 50000;

    // Network port the game server listens for connections on.
    int GameServerPort = 50010;

    // Announcements that show up when a user joins the game.
    std::vector<RuntimeConfigAnnouncement> Announcements = {
        { "Welcome to DS3OS", "\nYou have connected to an unofficial, work-in-progress, Dark Souls III server. Stability is not guaranteed, but welcome!\n\nMore information on this project is available here:\nhttps://github.com/tleonarduk/ds3os" }
    };

public:

    bool Save(const std::filesystem::path& Path);
    bool Load(const std::filesystem::path& Path);
    bool Serialize(nlohmann::json& Json, bool Loading);

};