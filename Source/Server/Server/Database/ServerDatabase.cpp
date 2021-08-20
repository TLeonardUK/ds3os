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
        "CREATE TABLE IF NOT EXISTS Players("                                       \
        "   PlayerId            INTEGER PRIMARY KEY AUTOINCREMENT,"                 \
        "   PlayerSteamId       CHAR(50)"                                           \
        ");"
    );
    tables.push_back(
        "CREATE TABLE IF NOT EXISTS BloodMessages("                                 \
        "   MessageId           INTEGER PRIMARY KEY AUTOINCREMENT,"                 \
        "   OnlineAreaId        INTEGER,"                                           \
        "   PlayerId            INTEGER,"                                           \
        "   PlayerSteamId       CHAR(50),"                                          \
        "   RatingPoor          INTEGER,"                                           \
        "   RatingGood          INTEGER,"                                           \
        "   Data                BLOB,"                                              \
        "   CreatedTime         TEXT"                                               \
        ");"
    );
    tables.push_back(
        "CREATE TABLE IF NOT EXISTS Bloodstains("                                   \
        "   BloodstainId        INTEGER PRIMARY KEY AUTOINCREMENT,"                 \
        "   OnlineAreaId        INTEGER,"                                           \
        "   PlayerId            INTEGER,"                                           \
        "   PlayerSteamId       CHAR(50),"                                          \
        "   Data                BLOB,"                                              \
        "   GhostData           BLOB,"                                              \
        "   CreatedTime         TEXT"                                               \
        ");"
    );
    tables.push_back(
        "CREATE TABLE IF NOT EXISTS Ghosts("                                        \
        "   GhostId             INTEGER PRIMARY KEY AUTOINCREMENT,"                 \
        "   OnlineAreaId        INTEGER,"                                           \
        "   PlayerId            INTEGER,"                                           \
        "   PlayerSteamId       CHAR(50),"                                          \
        "   Data                BLOB,"                                              \
        "   CreatedTime         TEXT"                                               \
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

bool ServerDatabase::RunStatement(const std::string& sql, const std::vector<DatabaseValue>& Values, RowCallback Callback)
{
    sqlite3_stmt* statement = nullptr;
    if (int result = sqlite3_prepare_v2(db_handle, sql.c_str(), (int)sql.length(), &statement, nullptr); result != SQLITE_OK)
    {
        Error("sqlite3_prepare_v2 failed with error: %s", sqlite3_errstr(result));
        return false;
    }
    for (int i = 0; i < Values.size(); i++)
    {
        const DatabaseValue& Value = Values[i];
        if (auto CastValue = std::get_if<std::string>(&Value))
        {
            if (int result = sqlite3_bind_text(statement, i + 1, CastValue->c_str(), (int)CastValue->length(), SQLITE_STATIC); result != SQLITE_OK)
            {
                Error("sqlite3_bind_text failed with error: %s", sqlite3_errstr(result));
                sqlite3_finalize(statement);
                return false;
            }
        }
        else if (auto CastValue = std::get_if<int>(&Value))
        {
            if (int result = sqlite3_bind_int(statement, i + 1, *CastValue); result != SQLITE_OK)
            {
                Error("sqlite3_bind_int failed with error: %s", sqlite3_errstr(result));
                sqlite3_finalize(statement);
                return false;
            }
        }
        else if (auto CastValue = std::get_if<uint32_t>(&Value))
        {
            // Eeeeeh, this is a shitty way to handle this, but it technically doesn't truncate the value.
            if (int result = sqlite3_bind_int64(statement, i + 1, *CastValue); result != SQLITE_OK)
            {
                Error("sqlite3_bind_int failed with error: %s", sqlite3_errstr(result));
                sqlite3_finalize(statement);
                return false;
            }
        }
        else if (auto CastValue = std::get_if<float>(&Value))
        {
            if (int result = sqlite3_bind_double(statement, i + 1, *CastValue); result != SQLITE_OK)
            {
                Error("sqlite3_bind_double failed with error: %s", sqlite3_errstr(result));
                sqlite3_finalize(statement);
                return false;
            }
        }
        else if (auto CastValue = std::get_if<std::vector<uint8_t>>(&Value))
        {
            if (int result = sqlite3_bind_blob(statement, i + 1, CastValue->data(), (int)CastValue->size(), SQLITE_STATIC); result != SQLITE_OK)
            {
                Error("sqlite3_bind_int failed with error: %s", sqlite3_errstr(result));
                sqlite3_finalize(statement);
                return false;
            }
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

    if (!RunStatement("SELECT PlayerId FROM Players WHERE PlayerSteamId = ?1", { SteamId }, [&PlayerId](sqlite3_stmt* statement) {
            PlayerId = sqlite3_column_int(statement, 0);
        }))
    {
        return false;
    }

    if (PlayerId == 0)
    {
        if (!RunStatement("INSERT INTO Players(PlayerSteamId) VALUES(?1)", { SteamId }, nullptr))
        {
            return false;
        }      
        PlayerId = (uint32_t)sqlite3_last_insert_rowid(db_handle);
    }

    return true;
}

std::shared_ptr<BloodMessage> ServerDatabase::FindBloodMessage(uint32_t MessageId)
{
    std::shared_ptr<BloodMessage> Result = nullptr;

    RunStatement("SELECT MessageId, OnlineAreaId, PlayerId, PlayerSteamId, RatingPoor, RatingGood, Data FROM BloodMessages WHERE MessageId = ?1", { MessageId }, [&Result](sqlite3_stmt* statement) {
        Result = std::make_shared<BloodMessage>();
        Result->MessageId       = sqlite3_column_int(statement, 0);
        Result->OnlineAreaId    = (OnlineAreaId)sqlite3_column_int(statement, 1);
        Result->PlayerId        = sqlite3_column_int(statement, 2);
        Result->PlayerSteamId   = (const char*)sqlite3_column_text(statement, 3);
        Result->RatingPoor      = sqlite3_column_int(statement, 4);
        Result->RatingGood      = sqlite3_column_int(statement, 5);
        const uint8_t* data_blob = (const uint8_t *)sqlite3_column_blob(statement, 6);
        Result->Data.assign(data_blob, data_blob + sqlite3_column_bytes(statement, 6));
    });

    return Result;
}

std::vector<std::shared_ptr<BloodMessage>> ServerDatabase::FindRecentBloodMessage(OnlineAreaId AreaId, int Count)
{
    std::vector<std::shared_ptr<BloodMessage>> Result;

    RunStatement("SELECT MessageId, OnlineAreaId, PlayerId, PlayerSteamId, RatingPoor, RatingGood, Data FROM BloodMessages WHERE OnlineAreaId = ?1 ORDER BY rowid DESC LIMIT ?2", { (uint32_t)AreaId, Count }, [&Result](sqlite3_stmt* statement) {
        std::shared_ptr<BloodMessage> Message = std::make_shared<BloodMessage>();
        Message->MessageId = sqlite3_column_int(statement, 0);
        Message->OnlineAreaId = (OnlineAreaId)sqlite3_column_int(statement, 1);
        Message->PlayerId = sqlite3_column_int(statement, 2);
        Message->PlayerSteamId = (const char*)sqlite3_column_text(statement, 3);
        Message->RatingPoor = sqlite3_column_int(statement, 4);
        Message->RatingGood = sqlite3_column_int(statement, 5);
        const uint8_t* data_blob = (const uint8_t*)sqlite3_column_blob(statement, 6);
        Message->Data.assign(data_blob, data_blob + sqlite3_column_bytes(statement, 6));
        Result.push_back(Message);
    });

    return Result;
}

std::shared_ptr<BloodMessage> ServerDatabase::CreateBloodMessage(OnlineAreaId AreaId, uint32_t PlayerId, const std::string& PlayerSteamId, const std::vector<uint8_t>& Data)
{
    if (!RunStatement("INSERT INTO BloodMessages(OnlineAreaId, PlayerId, PlayerSteamId, RatingPoor, RatingGood, Data, CreatedTime) VALUES(?1, ?2, ?3, ?4, ?5, ?6, datetime('now'))", { (uint32_t)AreaId, PlayerId, PlayerSteamId, 0, 0, Data }, nullptr))
    {
        return nullptr;
    }

    std::shared_ptr<BloodMessage> Result = std::make_shared<BloodMessage>();
    Result->MessageId = (uint32_t)sqlite3_last_insert_rowid(db_handle);
    Result->OnlineAreaId = AreaId;
    Result->PlayerId = PlayerId;
    Result->PlayerSteamId = PlayerSteamId;
    Result->RatingPoor = 0;
    Result->RatingGood = 0;
    Result->Data = Data;

    return Result;
}

bool ServerDatabase::RemoveOwnBloodMessage(uint32_t PlayerId, uint32_t MessageId)
{
    if (!RunStatement("DELETE FROM BloodMessages WHERE MessageId = ?1 AND PlayerId = ?2", { MessageId, PlayerId }, nullptr))
    {
        return false;
    }

    return sqlite3_total_changes(db_handle) > 0;
}

bool ServerDatabase::SetBloodMessageEvaluation(uint32_t MessageId, uint32_t Poor, uint32_t Good)
{
    if (!RunStatement("UPDATE BloodMessages SET RatingPoor = ?1, RatingGood = ?2 WHERE MessageId = ?3", { Poor, Good, MessageId }, nullptr))
    {
        return false;
    }

    return sqlite3_total_changes(db_handle) > 0;
}

std::shared_ptr<Bloodstain> ServerDatabase::FindBloodstain(uint32_t BloodstainId)
{
    std::shared_ptr<Bloodstain> Result;
  
    RunStatement("SELECT BloodstainId, OnlineAreaId, PlayerId, PlayerSteamId, Data, GhostData FROM Bloodstains WHERE BloodstainId = ?1", { BloodstainId }, [&Result](sqlite3_stmt* statement) {
        Result = std::make_shared<Bloodstain>();
        Result->BloodstainId = sqlite3_column_int(statement, 0);
        Result->OnlineAreaId = (OnlineAreaId)sqlite3_column_int(statement, 1);
        Result->PlayerId = sqlite3_column_int(statement, 2);
        Result->PlayerSteamId = (const char*)sqlite3_column_text(statement, 3);

        const uint8_t* data_blob = (const uint8_t*)sqlite3_column_blob(statement, 4);
        Result->Data.assign(data_blob, data_blob + sqlite3_column_bytes(statement, 4));

        const uint8_t* ghost_data_blob = (const uint8_t*)sqlite3_column_blob(statement, 5);
        Result->GhostData.assign(ghost_data_blob, ghost_data_blob + sqlite3_column_bytes(statement, 5));
    });

    return Result;
}

std::vector<std::shared_ptr<Bloodstain>> ServerDatabase::FindRecentBloodstains(OnlineAreaId AreaId, int Count)
{
    std::vector<std::shared_ptr<Bloodstain>> Result;

    RunStatement("SELECT BloodstainId, OnlineAreaId, PlayerId, PlayerSteamId, Data, GhostData FROM Bloodstains WHERE OnlineAreaId = ?1 ORDER BY rowid DESC LIMIT ?2", { (uint32_t)AreaId, Count }, [&Result](sqlite3_stmt* statement) {
        std::shared_ptr<Bloodstain> Stain = std::make_shared<Bloodstain>();
        Stain->BloodstainId = sqlite3_column_int(statement, 0);
        Stain->OnlineAreaId = (OnlineAreaId)sqlite3_column_int(statement, 1);
        Stain->PlayerId = sqlite3_column_int(statement, 2);
        Stain->PlayerSteamId = (const char*)sqlite3_column_text(statement, 3);

        const uint8_t* data_blob = (const uint8_t*)sqlite3_column_blob(statement, 4);
        Stain->Data.assign(data_blob, data_blob + sqlite3_column_bytes(statement, 4));

        const uint8_t* ghost_data_blob = (const uint8_t*)sqlite3_column_blob(statement, 5);
        Stain->GhostData.assign(ghost_data_blob, ghost_data_blob + sqlite3_column_bytes(statement, 5));

        Result.push_back(Stain);
    });

    return Result;
}

std::shared_ptr<Bloodstain> ServerDatabase::CreateBloodstain(OnlineAreaId AreaId, uint32_t PlayerId, const std::string& PlayerSteamId, const std::vector<uint8_t>& Data, const std::vector<uint8_t>& GhostData)
{
    if (!RunStatement("INSERT INTO Bloodstains(OnlineAreaId, PlayerId, PlayerSteamId, Data, GhostData, CreatedTime) VALUES(?1, ?2, ?3, ?4, ?5, datetime('now'))", { (uint32_t)AreaId, PlayerId, PlayerSteamId, Data, GhostData }, nullptr))
    {
        return nullptr;
    }

    std::shared_ptr<Bloodstain> Result = std::make_shared<Bloodstain>();
    Result->BloodstainId = (uint32_t)sqlite3_last_insert_rowid(db_handle);
    Result->OnlineAreaId = AreaId;
    Result->PlayerId = PlayerId;
    Result->PlayerSteamId = PlayerSteamId;
    Result->Data = Data;
    Result->GhostData = GhostData;

    return Result;
}

std::vector<std::shared_ptr<Ghost>> ServerDatabase::FindRecentGhosts(OnlineAreaId AreaId, int Count)
{
    std::vector<std::shared_ptr<Ghost>> Result;

    RunStatement("SELECT GhostId, OnlineAreaId, PlayerId, PlayerSteamId, Data FROM Ghosts WHERE OnlineAreaId = ?1 ORDER BY rowid DESC LIMIT ?2", { (uint32_t)AreaId, Count }, [&Result](sqlite3_stmt* statement) {
        std::shared_ptr<Ghost> Entry = std::make_shared<Ghost>();
        Entry->GhostId = sqlite3_column_int(statement, 0);
        Entry->OnlineAreaId = (OnlineAreaId)sqlite3_column_int(statement, 1);
        Entry->PlayerId = sqlite3_column_int(statement, 2);
        Entry->PlayerSteamId = (const char*)sqlite3_column_text(statement, 3);

        const uint8_t* data_blob = (const uint8_t*)sqlite3_column_blob(statement, 4);
        Entry->Data.assign(data_blob, data_blob + sqlite3_column_bytes(statement, 4));

        Result.push_back(Entry);
    });

    return Result;
}

std::shared_ptr<Ghost> ServerDatabase::CreateGhost(OnlineAreaId AreaId, uint32_t PlayerId, const std::string& PlayerSteamId, const std::vector<uint8_t>& Data)
{
    if (!RunStatement("INSERT INTO Ghosts(OnlineAreaId, PlayerId, PlayerSteamId, Data, CreatedTime) VALUES(?1, ?2, ?3, ?4, datetime('now'))", { (uint32_t)AreaId, PlayerId, PlayerSteamId, Data }, nullptr))
    {
        return nullptr;
    }

    std::shared_ptr<Ghost> Result = std::make_shared<Ghost>();
    Result->GhostId = (uint32_t)sqlite3_last_insert_rowid(db_handle);
    Result->OnlineAreaId = AreaId;
    Result->PlayerId = PlayerId;
    Result->PlayerSteamId = PlayerSteamId;
    Result->Data = Data;

    return Result;
}
