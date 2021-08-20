/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <filesystem>
#include <functional>
#include <variant>

#include "Server/Database/DatabaseTypes.h"

struct sqlite3;
struct sqlite3_stmt;

// Interface to the sqlite database.

class ServerDatabase
{
public:
    ServerDatabase();
    ~ServerDatabase();

    bool Open(const std::filesystem::path& path);
    bool Close();

    // ----------------------------------------------------------------
    // Player data interface
    // ----------------------------------------------------------------

    // Finds or creates a new player entry keyed to the steam id. If key does not
    // exist then create a new entry.
    // Return value is the new player's id.
    bool FindOrCreatePlayer(const std::string& SteamId, uint32_t& PlayerId);

    // ----------------------------------------------------------------
    // Blood message interface
    // ----------------------------------------------------------------

    // Finds the blood message in the database, or returns nullptr if it
    // doesn't exist.
    std::shared_ptr<BloodMessage> FindBloodMessage(uint32_t MessageId);

    // Gets the x most recent blood messages in the database.
    std::vector<std::shared_ptr<BloodMessage>> FindRecentBloodMessage(OnlineAreaId AreaId, int Count);

    // Creates a new blood message with the given data and returns a representation of it.
    std::shared_ptr<BloodMessage> CreateBloodMessage(OnlineAreaId AreaId, uint32_t PlayerId, const std::string& PlayerSteamId, const std::vector<uint8_t>& Data);

    // Removes a blood message from the database that is owned by the given player.
    bool RemoveOwnBloodMessage(uint32_t PlayerId, uint32_t MessageId);

    // Updates the evaluation ratings of a blood message.
    bool SetBloodMessageEvaluation(uint32_t MessageId, uint32_t Poor, uint32_t Good);

    // ----------------------------------------------------------------
    // Blood stain interface
    // ----------------------------------------------------------------

    // Finds the blood stain in the database, or returns nullptr if it
    // doesn't exist.
    std::shared_ptr<Bloodstain> FindBloodstain(uint32_t BloodstainId);

    // Gets the x most recent blood stains in the database.
    std::vector<std::shared_ptr<Bloodstain>> FindRecentBloodstains(OnlineAreaId AreaId, int Count);

    // Creates a new blood stain with the given data and returns a representation of it.
    std::shared_ptr<Bloodstain> CreateBloodstain(OnlineAreaId AreaId, uint32_t PlayerId, const std::string& PlayerSteamId, const std::vector<uint8_t>& Data, const std::vector<uint8_t>& GhostData);

    // ----------------------------------------------------------------
    // Ghosts interface
    // ----------------------------------------------------------------

    // Gets the x most recent ghosts in the database.
    std::vector<std::shared_ptr<Ghost>> FindRecentGhosts(OnlineAreaId AreaId, int Count);

    // Creates a new ghost with the given data and returns a representation of it.
    std::shared_ptr<Ghost> CreateGhost(OnlineAreaId AreaId, uint32_t PlayerId, const std::string& PlayerSteamId, const std::vector<uint8_t>& Data);

protected:

    using DatabaseValue = std::variant<std::string, int, uint32_t, float, std::vector<uint8_t>>;

    typedef std::function<void(sqlite3_stmt* statement)> RowCallback;

    bool RunStatement(const std::string& sql, const std::vector<DatabaseValue>& Values, RowCallback Callback);

    bool CreateTables();

private:
    sqlite3* db_handle = nullptr;

};