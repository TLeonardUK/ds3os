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

// Wraps the parameters used during matching for the various 
// summoning systems.
struct RuntimeConfigSoulMemoryMatchingParameters
{
    inline static std::vector<int> k_defaultTiers = {
        9'999,
        19'999,
        29'999,
        39'999,
        49'999,
        69'999,
        89'999,
        109'999,
        129'999,
        149'999,
        179'999,
        209'999,
        239'999,
        269'999,
        299'999,
        349'999,
        399'999,
        449'999,
        499'999,
        599'999,
        699'999,
        799'999,
        899'999,
        999'999,
        1'099'999,
        1'199'999,
        1'299'999,
        1'399'999,
        1'499'999,
        1'749'999,
        1'999'999,
        2'249'999,
        2'499'999,
        2'749'999,
        2'999'999,
        4'999'999,
        6'999'999,
        8'999'999,
        11'999'999,
        14'999'999,
        19'999'999,
        29'999'999,
        44'999'999,
        999'999'999
    };

    // End value of each soul memory tier.
    std::vector<int> Tiers = k_defaultTiers;

    // Flat disables all the soul level matching.
    bool DisableSoulMemoryMatching = false;

    // How far below the hosts tier the client can be.
    int TiersBelow = 0;

    // How far above the hosts tier the client can be.
    int TiersAbove = 0;

    // How far below the hosts tier the client can be when using a password.
    int TiersBelowWithPassword = 0;

    // How far above the hosts tier the client can be when using a password.
    int TiersAboveWithPassword = 0;

    bool Serialize(nlohmann::json& Json, bool Loading);

    int CalculateTier(int SoulMemory) const;
    bool CheckMatch(int HostSoulMemory, int ClientSoulMemory, bool UsingPassword) const;
};

// Configuration saved and loaded at runtime by the server from a configuration file.
class RuntimeConfig
{
public:

    // Type of the game this server should run.
    // DS3, DS2
    std::string GameType = "DarkSouls3";

    // Id of the server thats generated on first run.
    std::string ServerId = "";

    // Name used in the server import file.
    std::string ServerName = "Dark Souls Server";

    // Description used in the server import file.
    std::string ServerDescription = "A custom Dark Souls server.";

    // Hostname of the server that should be used for connecting.
    // If none is supplied then this wil be the external ip of the server.
    std::string ServerHostname = "";

    // Hostname of the server that should be used for connecting if behind the same external ip.
    // If none is supplied then this will be the private ip of the server.
    std::string ServerPrivateHostname = "";

    // If Advertise is set this is the master server that it will be registered to.
    // Be careful changing this, typically only one server should exist.
    std::string MasterServerIp = "ds3os-master.timleonard.uk";

    // Port the master server lists for connections at MasterServerIp;
    int MasterServerPort = 50020;

    // If true this reduces the amount of logging output to the bare minimum, only showing warnings/errors.
    bool QuietLogging = false;

    // If true register the server of the master servers list so others can see
    // and join it.
    bool Advertise = true;

    // How many seconds between each update on the master server. You should keep this
    // as high as possible to avoid saturating the master server.
    float AdvertiseHearbeatTime = 30.0f;

    // If set this server supports being sharded into multiple sub-servers to help users from 
    // having to run servers themselves. This is generally only expected to be used on
    // the main ds3os servers.
    bool SupportSharding = false;

    // If set the user will need to enter a password to recieve the keys to enter the 
    // server when its advertised.
    std::string Password = "";

    // Comma seperated list of the mods that can be installed when using this server.
    // Note: This is unimplemented right now, this is for future work.
    std::string ModsWhitelist = "";

    // Comma seperated list of the mods that cannot be installed when using this server.
    // Note: This is unimplemented right now, this is for future work.
    std::string ModsBlacklist = "";

    // Comma seperated list of the mods that have to be installed when using this server.
    // Note: This is unimplemented right now, this is for future work.
    std::string ModsRequiredList = "";

    // Network port the login server listens for connections on.
    int LoginServerPort = 50050;

    // Network port the authentication server listens for connections on.
    int AuthServerPort = 50000;

    // Network port the game server listens for connections on.
    int GameServerPort = 50010;

    // Network port the admin web-ui server listens for connections on.
    int WebUIServerPort = 50005;

    // Start of the game server port range when hosting multiple servers.
    int StartGameServerPortRange = 50060;

    // Username to login into web-ui with.
    std::string WebUIServerUsername = "";

    // Password to login into web-ui with.
    std::string WebUIServerPassword = "";

    // Announcements that show up when a user joins the game.
    std::vector<RuntimeConfigAnnouncement> Announcements = {
        { "Welcome to DSOS", "\nYou have connected to an unofficial, work-in-progress, Dark Souls server. Stability is not guaranteed, but welcome!\n\nMore information on this project is available here:\nhttps://github.com/tleonarduk/ds3os" }
    };

    // How often (in seconds) between each database trim.
    double DatabaseTrimInterval = 60 * 60 * 8;

    // Maximum number of blood messages to store per area in the cache.
    // If greater than this value are added, the oldest will be removed.
    int BloodMessageMaxLivePoolEntriesPerArea = 50;

    // Maximum number of blood messages to store in database. More than this will be trimmed.
    int BloodMessageMaxDatabaseEntries = 50000;

    // How many blood messages to insert into the live pool from the database
    // when the server starts. Saves the game looking empty until enough players
    // re-enter their messages.
    int BloodMessagePrimeCountPerArea = 50;

    // Maximum number of blood stains to store per area in the cache.
    // If greater than this value are added, the oldest will be removed.
    int BloodstainMaxLivePoolEntriesPerArea = 50;

    // How many blood stains to insert into the live pool from the database
    // when the server starts. Saves the game looking empty until enough players
    // re-enter their messages.
    int BloodstainPrimeCountPerArea = 50;

    // Maximum number of bloodstains to store in database. More than this will be trimmed.
    int BloodstainMaxDatabaseEntries = 1000;

    // If set to true bloodstain will only be stored in the memory-cache, and not persistently
    // on disk. 
    bool BloodstainMemoryCacheOnly = true;

    // Maximum number of ghoststo store per area in the cache.
    // If greater than this value are added, the oldest will be removed.
    int GhostMaxLivePoolEntriesPerArea = 50;

    // Maximum number of ghosts to store in database. More than this will be trimmed.
    int GhostMaxDatabaseEntries = 1000;

    // How many ghosts to insert into the live pool from the database
    // when the server starts. Saves the game looking empty until enough players
    // re-enter their messages.
    int GhostPrimeCountPerArea = 50;

    // If set to true ghosts will only be stored in the memory-cache, and not persistently
    // on disk. This can reduce database size and query costs as they can be quite spammily created.
    bool GhostMemoryCacheOnly = true;

    // This should be for all intents and purposes infinite. But you can limit
    // it if you so wish. Bare in mind that players may see signs that no longer
    // exist on the server.
    int SummonSignMaxEntriesPerArea = std::numeric_limits<int>::max();

    // How much XP the user gets when winning an undead match.
    int QuickMatchWinXp = 2250;

    // How much XP the user gets when losing an undead match.
    int QuickMatchLoseXp = 200; // I've seen this come back as 750 from server when on first rank.

    // How much XP the user gets when drawing an undead match.
    int QuickMatchDrawXp = 200;

    // How much XP to rank up at each level. These values are quess-timated, we should figure
    // out the actual rank boundries.
    // Ranks: unranked, iron, bronze, silver, gold
    std::vector<int> QuickMatchRankXp = { 0, 9250, 15000, 20000, 30000 };

    // Disables all ability to invade.
    bool DisableInvasions = false;

    // Disables all ability to place summon signs.
    bool DisableCoop = false;

    // Disables bloodmessages.
    bool DisableBloodMessages = false;

    // Disables bloodstains.
    bool DisableBloodStains = false;

    // Disables ghosts
    bool DisableGhosts = false;

    // Disables all auto summoning for invasions (alrich faithful, watchdogs, etc)
    bool DisableInvasionAutoSummon = false;

    // Disables all auto summoning for coop (blue sentinels, etc)
    bool DisableCoopAutoSummon = false;

    // If enabled invasion attempts will search all locations, not just the filtered list
    // that is supplied by the client.
    bool IgnoreInvasionAreaFilter = false;

    // How frequently (in seconds) the clients should send PlayerStatus updates. Increase this to 
    // reduce network bandwidth. Client clamps this to a minimum of 5.
    float PlayerStatusUploadInterval = 15.0f;

    // How much delay (in seconds) should be placed on RequestUpdatePlayerCharacter calls. Clamped to 60->50000
    float PlayerCharacterUpdateSendDelay = 600.0f;

    // How much delay (in seconds) should be placed on RequestUpdatePlayerStatus calls. Clamped to 60->50000
    float PlayerStatusUploadSendDelay = 300.0f;

    // ----------------------------------------------------------------
    // DS3 matching paramters
    // ----------------------------------------------------------------

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

    // Parameters used for determining who can play together in undead match.
    RuntimeConfigMatchingParameters UndeadMatchMatchingParameters = {
        0.9f, -10,                                                      // LowerLimit
        1.1f, +10,                                                      // UpperLimit
        { 1, 2, 3, 4, 6, 7, 8, 9, 10, 10, 10 },                         // WeaponLevelUpperLimit
        351,                                                            // RangeRemovalLevel
        true,                                                           // PasswordDisablesLimits
        false,                                                          // DisableLevelMatching
        false,                                                          // DisableWeaponLevelMatching
    };    

    // ----------------------------------------------------------------
    // DS2 matching paramters
    // ----------------------------------------------------------------

    RuntimeConfigSoulMemoryMatchingParameters DS2_WhiteSoapstoneMatchingParameters = {
        RuntimeConfigSoulMemoryMatchingParameters::k_defaultTiers,
        false,
        3, 1, 
        6, 4
    };

    RuntimeConfigSoulMemoryMatchingParameters DS2_SmallWhiteSoapstoneMatchingParameters = {
        RuntimeConfigSoulMemoryMatchingParameters::k_defaultTiers,
        false,
        4, 2,
        7, 5
    };

    RuntimeConfigSoulMemoryMatchingParameters DS2_RedSoapstoneMatchingParameters = {
        RuntimeConfigSoulMemoryMatchingParameters::k_defaultTiers,
        false,
        5, 2,
        5, 2
    };

    // Guessed
    RuntimeConfigSoulMemoryMatchingParameters DS2_MirrorKnightMatchingParameters = {
        RuntimeConfigSoulMemoryMatchingParameters::k_defaultTiers,
        false,
        5, 2,
        5, 2
    };

    RuntimeConfigSoulMemoryMatchingParameters DS2_DragonEyeMatchingParameters = {
        RuntimeConfigSoulMemoryMatchingParameters::k_defaultTiers,
        false,
        5, 5,
        5, 5
    };

    RuntimeConfigSoulMemoryMatchingParameters DS2_RedEyeOrbMatchingParameters = {
        RuntimeConfigSoulMemoryMatchingParameters::k_defaultTiers,
        false,
        0, 4,
        0, 4
    };

    RuntimeConfigSoulMemoryMatchingParameters DS2_BlueEyeOrbMatchingParameters = {
        RuntimeConfigSoulMemoryMatchingParameters::k_defaultTiers,
        false,
        3, 3,
        3, 3
    };

    RuntimeConfigSoulMemoryMatchingParameters DS2_BellKeeperMatchingParameters = {
        RuntimeConfigSoulMemoryMatchingParameters::k_defaultTiers,
        false,
        1, 3,
        1, 3
    };

    // Guessed
    RuntimeConfigSoulMemoryMatchingParameters DS2_RatMatchingParameters = {
        RuntimeConfigSoulMemoryMatchingParameters::k_defaultTiers,
        false,
        5, 2,
        5, 2
    };

    // Guessed
    RuntimeConfigSoulMemoryMatchingParameters DS2_BlueSentinelMatchingParameters = {
        RuntimeConfigSoulMemoryMatchingParameters::k_defaultTiers,
        false,
        4, 2,
        7, 5
    };

    // Guessed
    RuntimeConfigSoulMemoryMatchingParameters DS2_ArenaMatchingParameters = {
        RuntimeConfigSoulMemoryMatchingParameters::k_defaultTiers,
        false,
        5, 5,
        5, 5
    };

    RuntimeConfigSoulMemoryMatchingParameters DS2_MatchingAreaMatchingParameters = {
        RuntimeConfigSoulMemoryMatchingParameters::k_defaultTiers,
        false,
        5, 5,
        5, 5
    };

    // If enabled player behaviour will be scanned for cheating, and appropriate penalties applied.
    bool AntiCheatEnabled = false;

    // If disabled anti-cheat will not apply penalties, allows human to judge suitable-ness for banning.
    bool AntiCheatApplyPenalties = false;


    // Message shown to a user in announcements if they have been flagged as cheating.
    std::string AntiCheatWarningMessage = "Your account has been flagged for unfair play, if this continues you may be disconnected or banned from the server.";

    // Message shown to a user in announcements if they have been flagged as cheating.
    std::string AntiCheatDisconnectMessage = "Your account has been flagged for unfair play, you will be disconnected from the server.";

    // Message shown in-game just before banning the user.
    std::string AntiCheatBanMessage = "Your account has been flagged for unfair play, you have been banned from the server.";

    // Message shown when user logs in when they are banned.
    std::string BanAnnouncementMessage = "Your account has been banned from this server.";

    // Message shown when user logs in when they have a penalty score above the warning level.
    std::string WarningAnnouncementMessage = "Your account has been flagged for unfair play. Further unfair behaviour will lead to disconnection or bans.";


    // If set the player will recieve ingame management messages periodically when they mean the threshold.
    bool AntiCheatSendWarningMessageInGame = true;

    // How often the user will see a message if AntiCheatSendWarningMessageInGame is enabled.
    float AntiCheatSendWarningMessageInGameInterval = 120.0f;


    // How high the players penalty score has to be to cause them to start seeing the warning message.
    float AntiCheatWarningThreshold = 10;

    // How high the players penalty score has to be to cause them to be disconnected when detected.
    float AntiCheatDisconnectThreshold = 25;

    // How high the players penalty score has to be to cause them to be permanently banned.
    float AntiCheatBanThreshold = 100;


    // How much gets added to the players penalty score when the anti-cheat data supplied by the client is unexpected.
    float AntiCheatScore_ClientFlagged = 25;

    // How much gets added to the players penalty score when impossible stats are detected (eg. 99 in each stat, but level 1).
    float AntiCheatScore_ImpossibleStats = 25;

    // How much gets added to the players penalty score when they have an impossible name (eg. blank)
    float AntiCheatScore_ImpossibleName = 10;

    // How much gets added to the players penalty score when a potential security exploit is detected.
    float AntiCheatScore_Exploit = 100;


    // Unimplemented

    // How much gets added to the players penalty score when the delta between their stats on one update and another is impossible (eg. going from level 1 to 100).
    float AntiCheatScore_ImpossibleStatDelta = 20;

    // How much gets added to the players penalty score when they recieve and impossible number of items in one go (eg. 100x titanite slabs).
    float AntiCheatScore_ImpossibleGetItemQuantity = 20;

    // How much gets added to the players penalty score when their level is impossible with the play time they have.
    float AntiCheatScore_ImpossiblePlayTime = 10;

    // How much gets added to the players penalty score when they are in an impossible location (eg. at end of the game without triggering any of the mandatory boss events).
    float AntiCheatScore_ImpossibleLocation = 10;

    // How much gets added to the players penalty score when disconnects occur during multiplayer.
    float AntiCheatScore_UnfairDisconnect = 5;


    // If supplied various notifications can be made to discord.
    std::string DiscordWebHookUrl = "";

    // Send notifications when anti-cheat flags occur.
    bool SendDiscordNotice_AntiCheat = true;

    // Send notifications when sign placed.
    bool SendDiscordNotice_SummonSign = true;

    // Send notifications when a quick match begins.
    bool SendDiscordNotice_QuickMatch = true;

    // Send notifications when a bell is rung.
    bool SendDiscordNotice_Bell = true;

    // Send notifications when a boss is killed.
    bool SendDiscordNotice_Boss = true;

    // Send notifications when players kill each other.
    bool SendDiscordNotice_PvP = true;

public:

    bool Save(const std::filesystem::path& Path);
    bool Load(const std::filesystem::path& Path);
    bool Serialize(nlohmann::json& Json, bool Loading);

};
