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

class sqlite3;
class sqlite3_stmt;

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

protected:

    typedef std::function<void(sqlite3_stmt* statement)> RowCallback;

    bool RunStatement(const std::string& sql, const std::vector<std::string>& Values, RowCallback Callback);

    bool CreateTables();

private:
    sqlite3* db_handle = nullptr;

};