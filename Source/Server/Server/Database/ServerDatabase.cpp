/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/Database/ServerDatabase.h"
#include "Core/Utils/Logging.h"
#include <sqlite3.h> 

ServerDatabase::ServerDatabase()
{
}

ServerDatabase::~ServerDatabase()
{
}

bool ServerDatabase::Open(const std::filesystem::path& path)
{
    if (int result = sqlite3_open(path.string().c_str(), &db_handle); result != SQLITE_OK)
    {
        Error("sqlite_open failed with error: %s", sqlite3_errmsg(db_handle));
        return false;
    }

    Log("Opened sqlite database succesfully.");

    if (!CreateTables())
    {
        Log("Failed to create database tables.");
        return false;
    }

    return true;
}

bool ServerDatabase::Close()
{
    if (db_handle)
    {
        sqlite3_close(db_handle);
        db_handle = nullptr;
    }

    return true;
}

bool ServerDatabase::CreateTables()
{
    std::vector<std::string> tables;    
    tables.push_back(
        "CREATE TABLE IF NOT EXISTS players("                               \
        "   id             INTEGER PRIMARY KEY AUTOINCREMENT,"              \
        "   steam_id       CHAR(50)                             NOT NULL"   \
        ");"
    );

    for (const std::string& statement : tables)
    {
        char* errorMessage = nullptr;
        if (int result = sqlite3_exec(db_handle, statement.c_str(), nullptr, 0, &errorMessage); result != SQLITE_OK)
        {
            Error("Failed to create tables for server database with error: %s", errorMessage);
            sqlite3_free(errorMessage);
            return false;
        }

    }

    return true;
}

bool ServerDatabase::RunStatement(const std::string& sql, const std::vector<std::string>& Values, RowCallback Callback)
{
    sqlite3_stmt* statement = nullptr;
    if (int result = sqlite3_prepare_v2(db_handle, sql.c_str(), sql.length(), &statement, nullptr); result != SQLITE_OK)
    {
        Error("sqlite3_prepare_v2 failed with error: %s", sqlite3_errstr(result));
        return false;
    }
    for (int i = 0; i < Values.size(); i++)
    {
        if (int result = sqlite3_bind_text(statement, i + 1, Values[i].c_str(), Values[i].length(), SQLITE_STATIC); result != SQLITE_OK)
        {
            Error("sqlite3_bind_text failed with error: %s", sqlite3_errstr(result));
            sqlite3_finalize(statement);
            return false;
        }
    }
    while (true)
    {
        int result = sqlite3_step(statement);
        if (result == SQLITE_ROW)
        {
            if (Callback)
            {
                Callback(statement);
            }
        }
        else if (result == SQLITE_DONE)
        {
            break;
        }
        else
        {
            Error("sqlite3_step failed with error: %s", sqlite3_errstr(result));
            return false;
        }
    }
    if (int result = sqlite3_finalize(statement); result != SQLITE_OK)
    {
        Error("sqlite3_finalize failed with error: %s", sqlite3_errstr(result));
        return false;
    }
    return true;
}

bool ServerDatabase::FindOrCreatePlayer(const std::string& SteamId, uint32_t& PlayerId)
{
    PlayerId = 0;

    if (!RunStatement("SELECT id FROM players WHERE steam_id = ?1", { SteamId }, [&PlayerId](sqlite3_stmt* statement) {
            PlayerId = sqlite3_column_int(statement, 0);
        }))
    {
        return false;
    }

    if (PlayerId == 0)
    {
        if (!RunStatement("INSERT INTO players(steam_id) VALUES(?1); SELECT last_insert_rowid()", { SteamId }, nullptr))
        {
            return false;
        }      
        PlayerId = sqlite3_last_insert_rowid(db_handle);
    }

    return true;
}