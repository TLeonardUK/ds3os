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
#include "ThirdParty/nlohmann/json.hpp"

// Wraps up the values for an announcement show to the user when they join the game.
struct RuntimeConfigAnnouncement
{
    std::string Header;
    std::string Body;

    bool Serialize(nlohmann::json& Json, bool Loading);
};

// Wraps the parameters used during matching for the various 
// summoning systems.
struct RuntimeConfigMatchingParameters
{
    // Calculated as:
    //      (soul_level * LowerLimitMultiplier) + LowerLimitModifier
    float LowerLimitMultiplier =  1.0f;
    float LowerLimitModifier   = -10;
    float UpperLimitMultiplier =  1.0f;
    float UpperLimitModifier   =  10;

    // Maximum weapon levels a user with the given max weapon level can match with.
    std::vector<int> WeaponLevelUpperLimit = { 1, 2, 3, 4, 6, 7, 8, 9, 10, 10, 10 };

    // Range at which the upper limit of the range is removed and the lower
    // limit is set to this value.
    int RangeRemovalLevel = 351;

    // If true the range is ignored when a password is set.
    bool PasswordDisablesLimits = true;

    // Flat disables all the level matching.
    bool DisableLevelMatching = false;

    // Flat disables all the weapon level matching.
    bool DisableWeaponLevelMatching = false;

    bool Serialize(nlohmann::json& Json, bool Loading);

    bool CheckMatch(int HostSoulLevel, int HostWeaponLevel, int ClientSoulLevel, int ClientWeaponLevel, bool HasPassword) const;
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

    // Maximum number of blood messages to store per area in the cache.
    // If greater than this value are added, the oldest will be removed.
    int BloodMessageMaxLivePoolEntriesPerArea = 100;

    // How many blood messages to insert into the live pool from the database
    // when the server starts. Saves the game looking empty until enough players
    // re-enter their messages.
    int BloodMessagePrimeCountPerArea = 50;

    // Maximum number of blood stains to store per area in the cache.
    // If greater than this value are added, the oldest will be removed.
    int BloodstainMaxLivePoolEntriesPerArea = 100;

    // How many blood stains to insert into the live pool from the database
    // when the server starts. Saves the game looking empty until enough players
    // re-enter their messages.
    int BloodstainPrimeCountPerArea = 50;

    // Maximum number of ghoststo store per area in the cache.
    // If greater than this value are added, the oldest will be removed.
    int GhostMaxLivePoolEntriesPerArea = 100;

    // How many ghosts to insert into the live pool from the database
    // when the server starts. Saves the game looking empty until enough players
    // re-enter their messages.
    int GhostPrimeCountPerArea = 50;

    // This should be for all intents and purposes infinite. But you can limit
    // it if you so wish. Bare in mind that players may see signs that no longer
    // exist on the server.
    int SummonSignMaxEntriesPerArea = INT_MAX;

    // Parameters used for determining which signs a player can see.
    RuntimeConfigMatchingParameters SummonSignMatchingParameters = {
        0.9f, -10,                                                      // LowerLimit
        1.1f, +10,                                                      // UpperLimit
        { 1, 2, 3, 4, 6, 7, 8, 9, 10, 10, 10 },                         // WeaponLevelUpperLimit
        351,                                                            // RangeRemovalLevel
        true,                                                           // PasswordDisablesLimits
        false,                                                          // DisableLevelMatching
        false,                                                          // DisableWeaponLevelMatching
    };
    
    // Parameters used for determining way of the blue auto-summoning.
    RuntimeConfigMatchingParameters WayOfBlueMatchingParameters = {
        0.9f, -15,                                                      // LowerLimit
        1.1f, +15,                                                      // UpperLimit
        { 1, 2, 3, 4, 6, 7, 8, 9, 10, 10, 10 },                         // WeaponLevelUpperLimit
        351,                                                            // RangeRemovalLevel
        false,                                                          // PasswordDisablesLimits
        false,                                                          // DisableLevelMatching
        false,                                                          // DisableWeaponLevelMatching
    };
    
    // Parameters used for determining dark spirit invasion.
    RuntimeConfigMatchingParameters DarkSpiritInvasionMatchingParameters = {
        0.9f,   0,                                                      // LowerLimit
        1.1f, +20,                                                      // UpperLimit
        { 1, 2, 3, 4, 6, 7, 8, 9, 10, 10, 10 },                         // WeaponLevelUpperLimit
        351,                                                            // RangeRemovalLevel
        false,                                                          // PasswordDisablesLimits
        false,                                                          // DisableLevelMatching
        false,                                                          // DisableWeaponLevelMatching
    };

    // Parameters used for determining mound maker invasion.
    RuntimeConfigMatchingParameters MoundMakerInvasionMatchingParameters = {
        0.9f,   0,                                                      // LowerLimit
        1.15f, +20,                                                     // UpperLimit
        { 1, 2, 3, 4, 6, 7, 8, 9, 10, 10, 10 },                         // WeaponLevelUpperLimit
        351,                                                            // RangeRemovalLevel
        false,                                                          // PasswordDisablesLimits
        false,                                                          // DisableLevelMatching
        false,                                                          // DisableWeaponLevelMatching
    };
    
    // Parameters used for determining convenant auto invasion.
    RuntimeConfigMatchingParameters CovenantInvasionMatchingParameters = {
        0.8f, -20,                                                      // LowerLimit
        1.1f,  0,                                                       // UpperLimit
        { 1, 2, 3, 4, 6, 7, 8, 9, 10, 10, 10 },                         // WeaponLevelUpperLimit
        351,                                                            // RangeRemovalLevel
        false,                                                          // PasswordDisablesLimits
        false,                                                          // DisableLevelMatching
        false,                                                          // DisableWeaponLevelMatching
    };

public:

    bool Save(const std::filesystem::path& Path);
    bool Load(const std::filesystem::path& Path);
    bool Serialize(nlohmann::json& Json, bool Loading);

};