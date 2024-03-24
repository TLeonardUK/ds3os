/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/Database/ServerDatabase.h"
#include "Config/BuildConfig.h"
#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/DebugObjects.h"
#include "ThirdParty/sqlite/sqlite3.h"

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

    Trim();

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
        "   CellId              INTEGER,"                                           \
        "   PlayerId            INTEGER,"                                           \
        "   PlayerSteamId       CHAR(50),"                                          \
        "   CharacterId         INTEGER,"                                           \
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
        "   CellId              INTEGER,"                                           \
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
        "   CellId              INTEGER,"                                           \
        "   PlayerId            INTEGER,"                                           \
        "   PlayerSteamId       CHAR(50),"                                          \
        "   Data                BLOB,"                                              \
        "   CreatedTime         TEXT"                                               \
        ");"
    );
    tables.push_back(
        "CREATE TABLE IF NOT EXISTS Rankings("                                      \
        "   ScoreId             INTEGER PRIMARY KEY AUTOINCREMENT,"                 \
        "   BoardId             INTEGER,"                                           \
        "   PlayerId            INTEGER,"                                           \
        "   CharacterId         INTEGER,"                                           \
        "   Score               INTEGER,"                                           \
        "   Rank                INTEGER,"                                           \
        "   SerialRank          INTEGER,"                                           \
        "   Data                BLOB,"                                              \
        "   CreatedTime         TEXT"                                               \
        ");"
    );
    tables.push_back(
        "CREATE TABLE IF NOT EXISTS Characters("                                    \
        "   Id                  INTEGER PRIMARY KEY AUTOINCREMENT,"                 \
        "   PlayerId            INTEGER,"                                           \
        "   CharacterId         INTEGER,"                                           \
        "   Data                BLOB,"                                              \
        "   QuickMatchDuelRank  INTEGER,"                                           \
        "   QuickMatchDuelXp    INTEGER,"                                           \
        "   QuickMatchBrawlRank INTEGER,"                                           \
        "   QuickMatchBrawlXp   INTEGER,"                                           \
        "   CreatedTime         TEXT"                                               \
        ");"
    );
    tables.push_back(
        "CREATE TABLE IF NOT EXISTS Statistics("                                    \
        "   Name                STRING,"                                            \
        "   Scope               STRING,"                                            \
        "   Value               INTEGER,"                                           \
        "   PRIMARY KEY (Name, Scope)"                                              \
        ");"
    );
    tables.push_back(
        "CREATE TABLE IF NOT EXISTS MatchingSamples("                               \
        "   SampleId            INTEGER PRIMARY KEY AUTOINCREMENT,"                 \
        "   Name                STRING,"                                            \
        "   Scope               STRING,"                                            \
        "   Count               INTEGER,"                                           \
        "   Level               INTEGER,"                                           \
        "   WeaponLevel         INTEGER,"                                           \
        "   CreatedTime         TEXT"                                               \
        ");"
    );
    tables.push_back(
        "CREATE TABLE IF NOT EXISTS Bans("                                          \
        "   BanId               INTEGER PRIMARY KEY AUTOINCREMENT,"                 \
        "   PlayerSteamId       CHAR(50)"                                           \
        ");"
    );
    tables.push_back(
        "CREATE TABLE IF NOT EXISTS AntiCheatStates("                               \
        "   StateId             INTEGER PRIMARY KEY AUTOINCREMENT,"                 \
        "   PlayerSteamId       CHAR(50),"                                          \
        "   Score               REAL"                                               \
        ");"
    );
    tables.push_back(
        "CREATE TABLE IF NOT EXISTS AntiCheatLogs("                                 \
        "   LogId               INTEGER PRIMARY KEY AUTOINCREMENT,"                 \
        "   PlayerSteamId       CHAR(50),"                                          \
        "   Score               REAL,"                                              \
        "   TriggerName         CHAR(1024),"                                        \
        "   ExtraInfo           CHAR(1024)"                                         \
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
    DebugTimerScope Scope(Debug::DatabaseQueryTime);
    Debug::DatabaseQueries.Add(1.0f);

    sqlite3_stmt* statement = nullptr;
    if (int result = sqlite3_prepare_v2(db_handle, sql.c_str(), (int)sql.length(), &statement, nullptr); result != SQLITE_OK)
    {
        Error("sqlite3_prepare_v2 (%s) failed with error: %s", sql.c_str(), sqlite3_errstr(result));
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
        else if (auto CastValue = std::get_if<int64_t>(&Value))
        {
            if (int result = sqlite3_bind_int64(statement, i + 1, *CastValue); result != SQLITE_OK)
            {
                Error("sqlite3_bind_int failed with error: %s", sqlite3_errstr(result));
                sqlite3_finalize(statement);
                return false;
            }
        }
        else if (auto CastValue = std::get_if<uint64_t>(&Value))
        {
            if (int result = sqlite3_bind_int64(statement, i + 1, *CastValue); result != SQLITE_OK)
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

size_t ServerDatabase::GetTotalPlayers()
{
    uint32_t Result = 0;

    RunStatement("SELECT COUNT(*) FROM Players", { }, [&Result](sqlite3_stmt* statement) {
        Result = sqlite3_column_int(statement, 0);
    });

    return Result;
}

void ServerDatabase::BanPlayer(const std::string& SteamId)
{
    if (IsPlayerBanned(SteamId))
    {
        return;
    }

    RunStatement("INSERT INTO Bans(PlayerSteamId) VALUES(?1)", { SteamId }, nullptr);
}

void ServerDatabase::UnbanPlayer(const std::string& SteamId)
{
    RunStatement("DELETE FROM Bans WHERE PlayerSteamId = ?1", { SteamId }, nullptr);
    RunStatement("UPDATE AntiCheatStates SET Score = 0 WHERE PlayerSteamId = ?1", { SteamId }, nullptr);
}

bool ServerDatabase::IsPlayerBanned(const std::string& SteamId)
{
    uint32_t Result = 0;

    RunStatement("SELECT COUNT(*) FROM Bans WHERE PlayerSteamId=?1", { SteamId }, [&Result](sqlite3_stmt* statement) {
        Result = sqlite3_column_int(statement, 0);
    });

    return Result > 0;
}

std::vector<std::string> ServerDatabase::GetBannedSteamIds()
{
    std::vector<std::string> Result;

    RunStatement("SELECT PlayerSteamId FROM Bans", { }, [&Result](sqlite3_stmt* statement) {
        Result.push_back((const char*)sqlite3_column_text(statement, 0));
    });

    return Result;
}

std::shared_ptr<BloodMessage> ServerDatabase::FindBloodMessage(uint32_t MessageId)
{
    std::shared_ptr<BloodMessage> Result = nullptr;

    RunStatement("SELECT MessageId, OnlineAreaId, CellId, PlayerId, PlayerSteamId, CharacterId, RatingPoor, RatingGood, Data FROM BloodMessages WHERE MessageId = ?1", { MessageId }, [&Result](sqlite3_stmt* statement) {
        Result = std::make_shared<BloodMessage>();
        Result->MessageId       = sqlite3_column_int(statement, 0);
        Result->OnlineAreaId    = (uint32_t)sqlite3_column_int(statement, 1);
        Result->CellId          = (uint64_t)sqlite3_column_int64(statement, 2);
        Result->PlayerId        = sqlite3_column_int(statement, 3);
        Result->PlayerSteamId   = (const char*)sqlite3_column_text(statement, 4);
        Result->CharacterId     = sqlite3_column_int(statement, 5);
        Result->RatingPoor      = sqlite3_column_int(statement, 6);
        Result->RatingGood      = sqlite3_column_int(statement, 7);
        const uint8_t* data_blob = (const uint8_t *)sqlite3_column_blob(statement, 8);
        Result->Data.assign(data_blob, data_blob + sqlite3_column_bytes(statement, 8));
    });

    return Result;
}

std::vector<std::shared_ptr<BloodMessage>> ServerDatabase::FindRecentBloodMessage(uint32_t AreaId, uint64_t CellId, int Count)
{
    std::vector<std::shared_ptr<BloodMessage>> Result;

    RunStatement("SELECT MessageId, OnlineAreaId, CellId, PlayerId, PlayerSteamId, CharacterId, RatingPoor, RatingGood, Data FROM BloodMessages WHERE OnlineAreaId = ?1 AND CellId = ?2 ORDER BY rowid DESC LIMIT ?3", { (uint32_t)AreaId, CellId, Count }, [&Result](sqlite3_stmt* statement) {
        std::shared_ptr<BloodMessage> Message = std::make_shared<BloodMessage>();
        Message->MessageId = sqlite3_column_int(statement, 0);
        Message->OnlineAreaId = (uint32_t)sqlite3_column_int(statement, 1);
        Message->CellId = (uint64_t)sqlite3_column_int64(statement, 2);
        Message->PlayerId = sqlite3_column_int(statement, 3);
        Message->PlayerSteamId = (const char*)sqlite3_column_text(statement, 4);
        Message->CharacterId = sqlite3_column_int(statement, 5);
        Message->RatingPoor = sqlite3_column_int(statement, 6);
        Message->RatingGood = sqlite3_column_int(statement, 7);
        const uint8_t* data_blob = (const uint8_t*)sqlite3_column_blob(statement, 8);
        Message->Data.assign(data_blob, data_blob + sqlite3_column_bytes(statement, 8));
        Result.push_back(Message);
    });

    return Result;
}

std::vector<std::shared_ptr<BloodMessage>> ServerDatabase::FindRecentBloodMessage(uint32_t AreaId, int Count)
{
    std::vector<std::shared_ptr<BloodMessage>> Result;

    RunStatement("SELECT MessageId, OnlineAreaId, CellId, PlayerId, PlayerSteamId, CharacterId, RatingPoor, RatingGood, Data FROM BloodMessages WHERE OnlineAreaId = ?1 ORDER BY rowid DESC LIMIT ?2", { (uint32_t)AreaId, Count }, [&Result](sqlite3_stmt* statement) {
        std::shared_ptr<BloodMessage> Message = std::make_shared<BloodMessage>();
        Message->MessageId = sqlite3_column_int(statement, 0);
        Message->OnlineAreaId = (uint32_t)sqlite3_column_int(statement, 1);
        Message->CellId = (uint64_t)sqlite3_column_int64(statement, 2);
        Message->PlayerId = sqlite3_column_int(statement, 3);
        Message->PlayerSteamId = (const char*)sqlite3_column_text(statement, 4);
        Message->CharacterId = sqlite3_column_int(statement, 5);
        Message->RatingPoor = sqlite3_column_int(statement, 6);
        Message->RatingGood = sqlite3_column_int(statement, 7);
        const uint8_t* data_blob = (const uint8_t*)sqlite3_column_blob(statement, 8);
        Message->Data.assign(data_blob, data_blob + sqlite3_column_bytes(statement, 8));
        Result.push_back(Message);
    });

    return Result;
}

std::vector<std::shared_ptr<BloodMessage>> ServerDatabase::GetAllBloodMessages()
{
    std::vector<std::shared_ptr<BloodMessage>> Result;

    RunStatement("SELECT MessageId, OnlineAreaId, CellId, PlayerId, PlayerSteamId, CharacterId, RatingPoor, RatingGood, Data FROM BloodMessages", {  }, [&Result](sqlite3_stmt* statement) {
        std::shared_ptr<BloodMessage> Message = std::make_shared<BloodMessage>();
        Message->MessageId = sqlite3_column_int(statement, 0);
        Message->OnlineAreaId = (uint32_t)sqlite3_column_int(statement, 1);
        Message->CellId = (uint64_t)sqlite3_column_int64(statement, 2);
        Message->PlayerId = sqlite3_column_int(statement, 3);
        Message->PlayerSteamId = (const char*)sqlite3_column_text(statement, 4);
        Message->CharacterId = sqlite3_column_int(statement, 5);
        Message->RatingPoor = sqlite3_column_int(statement, 6);
        Message->RatingGood = sqlite3_column_int(statement, 7);
        const uint8_t* data_blob = (const uint8_t*)sqlite3_column_blob(statement, 8);
        Message->Data.assign(data_blob, data_blob + sqlite3_column_bytes(statement, 8));
        Result.push_back(Message);
    });

    return Result;
}

std::shared_ptr<BloodMessage> ServerDatabase::CreateBloodMessage(uint32_t AreaId, uint64_t CellId, uint32_t PlayerId, const std::string& PlayerSteamId, uint32_t CharacterId, const std::vector<uint8_t>& Data)
{
    if (!RunStatement("INSERT INTO BloodMessages(OnlineAreaId, CellId, PlayerId, PlayerSteamId, CharacterId, RatingPoor, RatingGood, Data, CreatedTime) VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, datetime('now'))", { (uint32_t)AreaId, (uint64_t)CellId, PlayerId, PlayerSteamId, CharacterId, 0, 0, Data }, nullptr))
    {
        return nullptr;
    }

    std::shared_ptr<BloodMessage> Result = std::make_shared<BloodMessage>();
    Result->MessageId = (uint32_t)sqlite3_last_insert_rowid(db_handle);
    Result->OnlineAreaId = AreaId;
    Result->CellId = CellId;
    Result->CharacterId = CharacterId;
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

    return sqlite3_changes(db_handle) > 0;
}

bool ServerDatabase::SetBloodMessageEvaluation(uint32_t MessageId, uint32_t Poor, uint32_t Good)
{
    if (!RunStatement("UPDATE BloodMessages SET RatingPoor = ?1, RatingGood = ?2 WHERE MessageId = ?3", { Poor, Good, MessageId }, nullptr))
    {
        return false;
    }

    return sqlite3_changes(db_handle) > 0;
}

void ServerDatabase::TrimBloodMessages(size_t MaxEntries)
{
    TrimTable("BloodMessages", "MessageId", MaxEntries);
}

std::shared_ptr<Bloodstain> ServerDatabase::FindBloodstain(uint32_t BloodstainId)
{
    std::shared_ptr<Bloodstain> Result;
  
    RunStatement("SELECT BloodstainId, OnlineAreaId, CellId, PlayerId, PlayerSteamId, Data, GhostData FROM Bloodstains WHERE BloodstainId = ?1", { BloodstainId }, [&Result](sqlite3_stmt* statement) {
        Result = std::make_shared<Bloodstain>();
        Result->BloodstainId = sqlite3_column_int(statement, 0);
        Result->OnlineAreaId = (uint32_t)sqlite3_column_int(statement, 1);
        Result->CellId = (uint64_t)sqlite3_column_int64(statement, 2);
        Result->PlayerId = sqlite3_column_int(statement, 3);
        Result->PlayerSteamId = (const char*)sqlite3_column_text(statement, 4);

        const uint8_t* data_blob = (const uint8_t*)sqlite3_column_blob(statement, 5);
        Result->Data.assign(data_blob, data_blob + sqlite3_column_bytes(statement, 5));

        const uint8_t* ghost_data_blob = (const uint8_t*)sqlite3_column_blob(statement, 6);
        Result->GhostData.assign(ghost_data_blob, ghost_data_blob + sqlite3_column_bytes(statement, 6));
    });

    return Result;
}

std::vector<std::shared_ptr<Bloodstain>> ServerDatabase::FindRecentBloodstains(uint32_t AreaId, uint64_t CellId, int Count)
{
    std::vector<std::shared_ptr<Bloodstain>> Result;

    RunStatement("SELECT BloodstainId, OnlineAreaId, CellId, PlayerId, PlayerSteamId, Data, GhostData FROM Bloodstains WHERE OnlineAreaId = ?1 AND CellId = ?2 ORDER BY rowid DESC LIMIT ?3", { (uint32_t)AreaId, CellId, Count }, [&Result](sqlite3_stmt* statement) {
        std::shared_ptr<Bloodstain> Stain = std::make_shared<Bloodstain>();
        Stain->BloodstainId = sqlite3_column_int(statement, 0);
        Stain->OnlineAreaId = (uint32_t)sqlite3_column_int(statement, 1);
        Stain->CellId = (uint64_t)sqlite3_column_int64(statement, 2);
        Stain->PlayerId = sqlite3_column_int(statement, 3);
        Stain->PlayerSteamId = (const char*)sqlite3_column_text(statement, 4);

        const uint8_t* data_blob = (const uint8_t*)sqlite3_column_blob(statement, 5);
        Stain->Data.assign(data_blob, data_blob + sqlite3_column_bytes(statement, 5));

        const uint8_t* ghost_data_blob = (const uint8_t*)sqlite3_column_blob(statement, 6);
        Stain->GhostData.assign(ghost_data_blob, ghost_data_blob + sqlite3_column_bytes(statement, 6));

        Result.push_back(Stain);
    });

    return Result;
}

std::vector<std::shared_ptr<Bloodstain>> ServerDatabase::FindRecentBloodstains(uint32_t AreaId, int Count)
{
    std::vector<std::shared_ptr<Bloodstain>> Result;

    RunStatement("SELECT BloodstainId, OnlineAreaId, CellId, PlayerId, PlayerSteamId, Data, GhostData FROM Bloodstains WHERE OnlineAreaId = ?1 ORDER BY rowid DESC LIMIT ?2", { (uint32_t)AreaId, Count }, [&Result](sqlite3_stmt* statement) {
        std::shared_ptr<Bloodstain> Stain = std::make_shared<Bloodstain>();
        Stain->BloodstainId = sqlite3_column_int(statement, 0);
        Stain->OnlineAreaId = (uint32_t)sqlite3_column_int(statement, 1);
        Stain->CellId = (uint64_t)sqlite3_column_int64(statement, 2);
        Stain->PlayerId = sqlite3_column_int(statement, 3);
        Stain->PlayerSteamId = (const char*)sqlite3_column_text(statement, 4);

        const uint8_t* data_blob = (const uint8_t*)sqlite3_column_blob(statement, 5);
        Stain->Data.assign(data_blob, data_blob + sqlite3_column_bytes(statement, 5));

        const uint8_t* ghost_data_blob = (const uint8_t*)sqlite3_column_blob(statement, 6);
        Stain->GhostData.assign(ghost_data_blob, ghost_data_blob + sqlite3_column_bytes(statement, 6));

        Result.push_back(Stain);
    });

    return Result;
}

std::shared_ptr<Bloodstain> ServerDatabase::CreateBloodstain(uint32_t AreaId, uint64_t CellId, uint32_t PlayerId, const std::string& PlayerSteamId, const std::vector<uint8_t>& Data, const std::vector<uint8_t>& GhostData)
{
    if (!RunStatement("INSERT INTO Bloodstains(OnlineAreaId, CellId, PlayerId, PlayerSteamId, Data, GhostData, CreatedTime) VALUES(?1, ?2, ?3, ?4, ?5, ?6, datetime('now'))", { (uint32_t)AreaId, CellId, PlayerId, PlayerSteamId, Data, GhostData }, nullptr))
    {
        return nullptr;
    }

    std::shared_ptr<Bloodstain> Result = std::make_shared<Bloodstain>();
    Result->BloodstainId = (uint32_t)sqlite3_last_insert_rowid(db_handle);
    Result->OnlineAreaId = AreaId;
    Result->CellId = CellId;
    Result->PlayerId = PlayerId;
    Result->PlayerSteamId = PlayerSteamId;
    Result->Data = Data;
    Result->GhostData = GhostData;

    return Result;
}

void ServerDatabase::TrimBloodStains(size_t MaxEntries)
{
    TrimTable("Bloodstains", "BloodstainId", MaxEntries);
}

std::vector<std::shared_ptr<Ghost>> ServerDatabase::FindRecentGhosts(uint32_t AreaId, uint64_t CellId, int Count)
{
    std::vector<std::shared_ptr<Ghost>> Result;

    RunStatement("SELECT GhostId, OnlineAreaId, CellId, PlayerId, PlayerSteamId, Data FROM Ghosts WHERE OnlineAreaId = ?1 AND CellId = ?2 ORDER BY rowid DESC LIMIT ?3", { (uint32_t)AreaId, CellId, Count }, [&Result](sqlite3_stmt* statement) {
        std::shared_ptr<Ghost> Entry = std::make_shared<Ghost>();
        Entry->GhostId = sqlite3_column_int(statement, 0);
        Entry->OnlineAreaId = (uint32_t)sqlite3_column_int(statement, 1);
        Entry->CellId = (uint64_t)sqlite3_column_int64(statement, 2);
        Entry->PlayerId = sqlite3_column_int(statement, 3);
        Entry->PlayerSteamId = (const char*)sqlite3_column_text(statement, 4);

        const uint8_t* data_blob = (const uint8_t*)sqlite3_column_blob(statement, 5);
        Entry->Data.assign(data_blob, data_blob + sqlite3_column_bytes(statement, 5));

        Result.push_back(Entry);
    });

    return Result;
}

std::vector<std::shared_ptr<Ghost>> ServerDatabase::FindRecentGhosts(uint32_t AreaId,int Count)
{
    std::vector<std::shared_ptr<Ghost>> Result;

    RunStatement("SELECT GhostId, OnlineAreaId, CellId, PlayerId, PlayerSteamId, Data FROM Ghosts WHERE OnlineAreaId = ?1 ORDER BY rowid DESC LIMIT ?2", { (uint32_t)AreaId, Count }, [&Result](sqlite3_stmt* statement) {
        std::shared_ptr<Ghost> Entry = std::make_shared<Ghost>();
        Entry->GhostId = sqlite3_column_int(statement, 0);
        Entry->OnlineAreaId = (uint32_t)sqlite3_column_int(statement, 1);
        Entry->CellId = (uint64_t)sqlite3_column_int64(statement, 2);
        Entry->PlayerId = sqlite3_column_int(statement, 3);
        Entry->PlayerSteamId = (const char*)sqlite3_column_text(statement, 4);

        const uint8_t* data_blob = (const uint8_t*)sqlite3_column_blob(statement, 5);
        Entry->Data.assign(data_blob, data_blob + sqlite3_column_bytes(statement, 5));

        Result.push_back(Entry);
    });

    return Result;
}

std::shared_ptr<Ghost> ServerDatabase::CreateGhost(uint32_t AreaId, uint64_t CellId, uint32_t PlayerId, const std::string& PlayerSteamId, const std::vector<uint8_t>& Data)
{
    if (!RunStatement("INSERT INTO Ghosts(OnlineAreaId, CellId, PlayerId, PlayerSteamId, Data, CreatedTime) VALUES(?1, ?2, ?3, ?4, ?5, datetime('now'))", { (uint32_t)AreaId, CellId, PlayerId, PlayerSteamId, Data }, nullptr))
    {
        return nullptr;
    }

    std::shared_ptr<Ghost> Result = std::make_shared<Ghost>();
    Result->GhostId = (uint32_t)sqlite3_last_insert_rowid(db_handle);
    Result->OnlineAreaId = AreaId;
    Result->CellId = CellId;
    Result->PlayerId = PlayerId;
    Result->PlayerSteamId = PlayerSteamId;
    Result->Data = Data;

    return Result;
}

void ServerDatabase::TrimGhosts(size_t MaxEntries)
{
    TrimTable("Ghosts", "GhostId", MaxEntries);
}

std::shared_ptr<Ranking> ServerDatabase::RegisterScore(uint32_t BoardId, uint32_t PlayerId, uint32_t CharacterId, uint32_t Score, const std::vector<uint8_t>& Data)
{
    // Delete existing ranking.
    if (!RunStatement("DELETE FROM Rankings WHERE BoardId = ?1 AND PlayerId = ?2 AND CharacterId = ?3", { BoardId, PlayerId, CharacterId }, nullptr))
    {
        return nullptr;
    }

    // Insert new ranking.
    if (!RunStatement("INSERT INTO Rankings(BoardId, PlayerId, CharacterId, Score, Data, CreatedTime) VALUES(?1, ?2, ?3, ?4, ?5, datetime('now'))", { BoardId, PlayerId, CharacterId, Score, Data }, nullptr))
    {
        return nullptr;
    }

    uint32_t NewRankingId = (uint32_t)sqlite3_last_insert_rowid(db_handle);
    uint32_t NewRank = 1;
    uint32_t NewSerialRank = 1;

    // Update rankings of all socre entries. 
    // TODO: If we get any reasonable frequency of ranking registrations we should do these lazily 
    //       at a periodic interval to keep load down.
    
    uint32_t CurrentRank = 1;
    uint32_t CurrentRankScore = 0;
    uint32_t CurrentSerialRank = 1;

    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> ScoreRanks;

    RunStatement("SELECT ScoreId, Score FROM Rankings WHERE BoardId = ?1 ORDER BY Score DESC, CreatedTime ASC", { BoardId }, [&CurrentRank, &CurrentRankScore, &CurrentSerialRank, &ScoreRanks, NewRankingId, &NewRank, &NewSerialRank] (sqlite3_stmt* statement) {
        uint32_t ScoreId = sqlite3_column_int(statement, 0);
        uint32_t Score = sqlite3_column_int(statement, 1);
        uint32_t ScoreRank = 0;
        uint32_t ScoreSerialRank = 0;

        if (CurrentSerialRank == 1)
        {
            CurrentRankScore = Score;
        }
        else if (Score < CurrentRankScore)
        {
            CurrentRank++;
            CurrentRankScore = Score;
        }

        if (ScoreId == NewRankingId)
        {
            NewRank = CurrentRank;
            NewSerialRank = CurrentSerialRank;
        }

        ScoreRanks.push_back(std::make_tuple(ScoreId, CurrentRank, CurrentSerialRank));

        CurrentSerialRank++;       
    });

    for (auto& tuple : ScoreRanks)
    {
        if (!RunStatement("UPDATE Rankings SET Rank = ?1, SerialRank = ?2 WHERE ScoreId = ?3", { std::get<1>(tuple), std::get<2>(tuple), std::get<0>(tuple) }, nullptr))
        {
            return nullptr;
        }
    }

    // Create and return a ranking to the caller.
    std::shared_ptr<Ranking> Result = std::make_shared<Ranking>();
    Result->Id = NewRankingId;
    Result->BoardId = BoardId;
    Result->PlayerId = PlayerId;
    Result->CharacterId = CharacterId;
    Result->Score = Score;
    Result->Data = Data;
    Result->Rank = NewRank; 
    Result->SerialRank = NewSerialRank;

    return Result;
}

std::vector<std::shared_ptr<Ranking>> ServerDatabase::GetRankings(uint32_t BoardId, uint32_t Offset, uint32_t Count)
{
    std::vector<std::shared_ptr<Ranking>> Result;

    RunStatement("SELECT ScoreId, PlayerId, CharacterId, Rank, SerialRank, Score, Data FROM Rankings WHERE BoardId = ?1 ORDER BY SerialRank ASC LIMIT ?2 OFFSET ?3", { BoardId, Count, Offset - 1 }, [&Result, BoardId](sqlite3_stmt* statement) {
        std::shared_ptr<Ranking> Entry = std::make_shared<Ranking>();
        Entry->Id = sqlite3_column_int(statement, 0);
        Entry->BoardId = BoardId;
        Entry->PlayerId = sqlite3_column_int(statement, 1);
        Entry->CharacterId = sqlite3_column_int(statement, 2);
        Entry->Rank = sqlite3_column_int(statement, 3);
        Entry->SerialRank = sqlite3_column_int(statement, 4);
        Entry->Score = sqlite3_column_int(statement, 5);

        const uint8_t* data_blob = (const uint8_t*)sqlite3_column_blob(statement, 6);
        Entry->Data.assign(data_blob, data_blob + sqlite3_column_bytes(statement, 6));

        Result.push_back(Entry);
    });

    return Result;
}

std::shared_ptr<Ranking> ServerDatabase::GetCharacterRanking(uint32_t BoardId, uint32_t PlayerId, uint32_t CharacterId)
{
    std::shared_ptr<Ranking> Result;

    RunStatement("SELECT ScoreId, Rank, SerialRank, Score, Data FROM Rankings WHERE BoardId = ?1 AND PlayerId = ?2 AND CharacterId = ?3 LIMIT 1", { BoardId, PlayerId, CharacterId }, [&Result, BoardId, PlayerId, CharacterId](sqlite3_stmt* statement) {
        Result = std::make_shared<Ranking>();
        Result->Id = sqlite3_column_int(statement, 0);
        Result->BoardId = BoardId;
        Result->PlayerId = PlayerId;
        Result->CharacterId = CharacterId;
        Result->Rank = sqlite3_column_int(statement, 1);
        Result->SerialRank = sqlite3_column_int(statement, 2);
        Result->Score = sqlite3_column_int(statement, 3);

        const uint8_t* data_blob = (const uint8_t*)sqlite3_column_blob(statement, 4);
        Result->Data.assign(data_blob, data_blob + sqlite3_column_bytes(statement, 4));
    });

    return Result;
}

uint32_t ServerDatabase::GetRankingCount(uint32_t BoardId)
{
    uint32_t Result = 0;

    RunStatement("SELECT COUNT(*) FROM Rankings WHERE BoardId = ?1", { BoardId }, [&Result](sqlite3_stmt* statement) {
        Result = sqlite3_column_int(statement, 0);
    });

    return Result;
}

bool ServerDatabase::CreateOrUpdateCharacter(uint32_t PlayerId, uint32_t CharacterId, const std::vector<uint8_t>& Data)
{
    if (!RunStatement("UPDATE Characters SET Data = ?3 WHERE PlayerId = ?1 AND CharacterId = ?2", { PlayerId, CharacterId, Data }, nullptr))
    {
        return false;
    }

    if (sqlite3_changes(db_handle) == 0)
    {
        if (!RunStatement("INSERT INTO Characters(PlayerId, CharacterId, Data, CreatedTime) VALUES(?1, ?2, ?3, datetime('now'))", { PlayerId, CharacterId, Data }, nullptr))
        {
            return false;
        }
    }

    return true;
}

std::shared_ptr<Character> ServerDatabase::FindCharacter(uint32_t PlayerId, uint32_t CharacterId)
{
    std::shared_ptr<Character> Result;

    RunStatement("SELECT Id, Data, QuickMatchDuelRank, QuickMatchDuelXp, QuickMatchBrawlRank, QuickMatchBrawlXp FROM Characters WHERE PlayerId = ?1 AND CharacterId = ?2 LIMIT 1", { PlayerId, CharacterId }, [&Result, PlayerId, CharacterId](sqlite3_stmt* statement) {
        Result = std::make_shared<Character>();
        Result->Id = sqlite3_column_int(statement, 0);
        Result->PlayerId = PlayerId;
        Result->CharacterId = CharacterId;

        const uint8_t* data_blob = (const uint8_t*)sqlite3_column_blob(statement, 1);
        Result->Data.assign(data_blob, data_blob + sqlite3_column_bytes(statement, 1));

        Result->QuickMatchDuelRank = sqlite3_column_int(statement, 2);
        Result->QuickMatchDuelXp = sqlite3_column_int(statement, 3);
        Result->QuickMatchBrawlRank = sqlite3_column_int(statement, 4);
        Result->QuickMatchBrawlXp = sqlite3_column_int(statement, 5);
    });

    return Result;
}

bool ServerDatabase::UpdateCharacterQuickMatchRank(uint32_t PlayerId, uint32_t CharacterId, uint32_t DualRank, uint32_t DualXp, uint32_t BrawlRank, uint32_t BrawlXp)
{
    std::shared_ptr<Character> Result;

    if (!RunStatement("UPDATE Characters SET QuickMatchDuelRank = ?1, QuickMatchDuelXp = ?2, QuickMatchBrawlRank = ?3, QuickMatchBrawlXp = ?4  WHERE PlayerId = ?5 AND CharacterId = ?6", { 
            DualRank,
            DualXp,
            BrawlRank,
            BrawlXp,
            PlayerId,
            CharacterId
        }, nullptr))
    {
        return false;
    }

    return true;
}

void ServerDatabase::AddMatchingSample(const std::string& Name, const std::string& Scope, int64_t Count, uint32_t Level, uint32_t WeaponLevel)
{
    RunStatement("INSERT INTO MatchingSamples(Name, Scope, Count, Level, WeaponLevel, CreatedTime) VALUES(?1, ?2, ?3, ?4, ?5, datetime('now'))", { Name, Scope, Count, Level, WeaponLevel }, nullptr);
}

void ServerDatabase::AddStatistic(const std::string& Name, const std::string& Scope, int64_t Count)
{
    if constexpr (!BuildConfig::DATABASE_STAT_GATHERING_ENABLED)
    {
        return;
    }

    if (!RunStatement("UPDATE Statistics SET Value = Value + ?3 WHERE Name = ?1 AND Scope = ?2", { Name, Scope, Count }, nullptr))
    {
        return;
    }

    if (sqlite3_changes(db_handle) == 0)
    {
        if (!RunStatement("INSERT INTO Statistics(Name, Scope, Value) VALUES(?1, ?2, ?3)", { Name, Scope, Count }, nullptr))
        {
            return;
        }
    }
}

void ServerDatabase::SetStatistic(const std::string& Name, const std::string& Scope, int64_t Count)
{
    if constexpr (!BuildConfig::DATABASE_STAT_GATHERING_ENABLED)
    {
        return;
    }

    if (!RunStatement("UPDATE Statistics SET Value = ?3 WHERE Name = ?1 AND Scope = ?2", { Name, Scope, Count }, nullptr))
    {
        return;
    }

    if (sqlite3_changes(db_handle) == 0)
    {
        if (!RunStatement("INSERT INTO Statistics(Name, Scope, Value) VALUES(?1, ?2, ?3)", { Name, Scope, Count }, nullptr))
        {
            return;
        }
    }
}

int64_t ServerDatabase::GetStatistic(const std::string& Name, const std::string& Scope)
{
    if constexpr (!BuildConfig::DATABASE_STAT_GATHERING_ENABLED)
    {
        return 0;
    }

    int64_t Result;

    RunStatement("SELECT Value FROM Statistics WHERE Name = ?1 AND Scope = ?2 LIMIT 1", { Name, Scope }, [&Result](sqlite3_stmt* statement) {
        Result = sqlite3_column_int64(statement, 0);
    });

    return Result;
}

void ServerDatabase::AddGlobalStatistic(const std::string& Name, int64_t Count)
{
    return AddStatistic(Name, "Global", Count);
}

void ServerDatabase::SetGlobalStatistic(const std::string& Name, int64_t Count)
{
    return SetStatistic(Name, "Global", Count);
}

int64_t ServerDatabase::GetGlobalStatistic(const std::string& Name)
{
    return GetStatistic(Name, "Global");
}

void ServerDatabase::AddPlayerStatistic(const std::string& Name, uint32_t PlayerId, int64_t Count)
{
    if constexpr (!BuildConfig::DATABASE_STAT_GATHERING_ENABLED)
    {
        return;
    }

    if constexpr (!BuildConfig::STORE_PER_PLAYER_STATISTICS)
    {
        return;
    }

    char Scope[64];
    snprintf(Scope, 64, "Player/%u", PlayerId);

    AddStatistic(Name, Scope, Count);
}

void ServerDatabase::SetPlayerStatistic(const std::string& Name, uint32_t PlayerId, int64_t Count)
{
    if constexpr (!BuildConfig::DATABASE_STAT_GATHERING_ENABLED)
    {
        return;
    }

    if constexpr (!BuildConfig::STORE_PER_PLAYER_STATISTICS)
    {
        return;
    }

    char Scope[64];
    snprintf(Scope, 64, "Player/%u", PlayerId);

    return SetStatistic(Name, Scope, Count);
}

int64_t ServerDatabase::GetPlayerStatistic(const std::string& Name, uint32_t PlayerId)
{
    if constexpr (!BuildConfig::DATABASE_STAT_GATHERING_ENABLED)
    {
        return 0;
    }

    if constexpr (!BuildConfig::STORE_PER_PLAYER_STATISTICS)
    {
        return 0;
    }

    char Scope[64];
    snprintf(Scope, 64, "Player/%u", PlayerId);

    return GetStatistic(Name, Scope);
}

float ServerDatabase::GetAntiCheatPenaltyScore(const std::string& SteamId)
{
    float Result = 0.0f;

    RunStatement("SELECT Score FROM AntiCheatStates WHERE PlayerSteamId = ?1 LIMIT 1", { SteamId }, [&Result](sqlite3_stmt* statement) {
        Result = (float)sqlite3_column_double(statement, 0);
    });

    return Result;
}

void ServerDatabase::AddAntiCheatPenaltyScore(const std::string& SteamId, float Amount)
{
    if (!RunStatement("UPDATE AntiCheatStates SET Score = Score + ?2 WHERE PlayerSteamId = ?1", { SteamId, Amount }, nullptr))
    {
        return;
    }

    if (sqlite3_changes(db_handle) == 0)
    {
        if (!RunStatement("INSERT INTO AntiCheatStates(PlayerSteamId, Score) VALUES(?1, ?2)", { SteamId, Amount }, nullptr))
        {
            return;
        }
    }
}

void ServerDatabase::LogAntiCheatTrigger(const std::string& SteamId, const std::string& TriggerName, float Penalty, const std::string& ExtraInfo)
{
    if (!RunStatement("INSERT INTO AntiCheatLogs(PlayerSteamId, Score, TriggerName, ExtraInfo) VALUES(?1, ?2, ?3, ?4)", { SteamId, Penalty, TriggerName, ExtraInfo }, nullptr))
    {
        return;
    }
}

std::vector<AntiCheatLog> ServerDatabase::GetAntiCheatLogs(const std::string& SteamId)
{
    std::vector<AntiCheatLog> Result;

    RunStatement("SELECT Score, TriggerName, ExtraInfo FROM AntiCheatLogs WHERE PlayerSteamId = ?1", { SteamId }, [&Result, SteamId](sqlite3_stmt* statement) {
        AntiCheatLog Log;
        Log.SteamId = SteamId;
        Log.Score = (float)sqlite3_column_double(statement, 0);
        Log.TriggerName = (const char*)sqlite3_column_text(statement, 1);
        Log.Extra = (const char*)sqlite3_column_text(statement, 2);

        Result.push_back(Log);
    });

    return Result;
}

void ServerDatabase::TrimTable(const std::string& TableName, const std::string& IdColumn, size_t MaxEntries)
{
    size_t TotalEntries = 0;

    RunStatement("SELECT COUNT(*) FROM " + TableName, { }, [&TotalEntries](sqlite3_stmt* statement) {
        TotalEntries = sqlite3_column_int(statement, 0);
    });

    if (TotalEntries <= MaxEntries)
    {
        return;
    }

    size_t ToRemove = TotalEntries - MaxEntries;

    RunStatement("DELETE FROM " + TableName + " WHERE " + IdColumn + " IN (SELECT " + IdColumn + " FROM " + TableName + " ORDER BY " + IdColumn + " ASC LIMIT ?1)", { (int32_t)ToRemove }, nullptr);
}

void ServerDatabase::Trim()
{
    if constexpr (!BuildConfig::STORE_PER_PLAYER_STATISTICS)
    {
        RunStatement("DELETE FROM Statistics WHERE Scope LIKE \"Player/%\"", {}, nullptr);
    }
}
