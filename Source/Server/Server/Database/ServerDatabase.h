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
    // Character interface
    // ----------------------------------------------------------------

    // Creates or updates a specific character owned by the player.
    bool CreateOrUpdateCharacter(uint32_t PlayerId, uint32_t CharacterId, const std::vector<uint8_t>& Data);

    // Finds a character owned by a specific player.
    std::shared_ptr<Character> FindCharacter(uint32_t PlayerId, uint32_t CharacterId);

    // Updates a characters quick match rank.
    bool UpdateCharacterQuickMatchRank(uint32_t PlayerId, uint32_t CharacterId, uint32_t DualRank, uint32_t DualXp, uint32_t BrawlRank, uint32_t BrawlXp);

    // ----------------------------------------------------------------
    // Blood message interface
    // ----------------------------------------------------------------

    // Finds the blood message in the database, or returns nullptr if it
    // doesn't exist.
    std::shared_ptr<BloodMessage> FindBloodMessage(uint32_t MessageId);

    // Gets the x most recent blood messages in the database.
    std::vector<std::shared_ptr<BloodMessage>> FindRecentBloodMessage(OnlineAreaId AreaId, int Count);

    // Creates a new blood message with the given data and returns a representation of it.
    std::shared_ptr<BloodMessage> CreateBloodMessage(OnlineAreaId AreaId, uint32_t PlayerId, const std::string& PlayerSteamId, uint32_t CharacterId, const std::vector<uint8_t>& Data);

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

    // ----------------------------------------------------------------
    // Rankings interface
    // ----------------------------------------------------------------

    // Registers a new score to a leaderboard.
    std::shared_ptr<Ranking> RegisterScore(uint32_t BoardId, uint32_t PlayerId, uint32_t CharcterId, uint32_t Score, const std::vector<uint8_t>& Data);

    // Gets a range of ranks from a leaderboard.
    std::vector<std::shared_ptr<Ranking>> GetRankings(uint32_t BoardId, uint32_t Offset, uint32_t Count);

    // Gets the ranking for a given player character.
    std::shared_ptr<Ranking> GetCharacterRanking(uint32_t BoardId, uint32_t PlayerId, uint32_t CharacterId);

    // Get the number of ranks in a given board.
    uint32_t GetRankingCount(uint32_t BoardId);

    // ----------------------------------------------------------------
    // Statistic interface
    // ----------------------------------------------------------------

    // Adds the given count to the statistic with the given name. If statistic
    // does not exist, it will be created.
    void AddStatistic(const std::string& Name, const std::string& Scope, int64_t Count);

    // Set the value of the statistic with the given name. If statistic
    // does not exist, it will be created.
    void SetStatistic(const std::string& Name, const std::string& Scope, int64_t Count);

    // Gets the value of the statistic with the given name. If statitic 
    // does not exist, 0 will be returned.
    int64_t GetStatistic(const std::string& Name, const std::string& Scope);

    // Some helper versions of the above functions that infer the scope.
    void AddGlobalStatistic(const std::string& Name, int64_t Count);
    void SetGlobalStatistic(const std::string& Name, int64_t Count);
    int64_t GetGlobalStatistic(const std::string& Name);

    // Some helper versions of the above functions that infer the scope.
    void AddPlayerStatistic(const std::string& Name, uint32_t PlayerId, int64_t Count);
    void SetPlayerStatistic(const std::string& Name, uint32_t PlayerId, int64_t Count);
    int64_t GetPlayerStatistic(const std::string& Name, uint32_t PlayerId);

    // ----------------------------------------------------------------
    // Sample interface
    // ----------------------------------------------------------------

    // This acts in the same way as a statistic except a new instance is created each call
    // to act as a sample at the current point in time.
    void AddMatchingSample(const std::string& Name, const std::string& Scope, int64_t Count, uint32_t Level, uint32_t WeaponLevel);

protected:

    using DatabaseValue = std::variant<std::string, int, uint32_t, float, std::vector<uint8_t>, int64_t>;

    typedef std::function<void(sqlite3_stmt* statement)> RowCallback;

    bool RunStatement(const std::string& sql, const std::vector<DatabaseValue>& Values, RowCallback Callback);

    bool CreateTables();

private:
    sqlite3* db_handle = nullptr;

};