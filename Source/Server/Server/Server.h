/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Server/Database/ServerDatabase.h"

#include "Shared/Platform/Platform.h"

#include "Shared/Core/Crypto/RSAKeyPair.h"

#include "Shared/Core/Network/NetIPAddress.h"

#include "Shared/Game/GameType.h"

#include "Config/RuntimeConfig.h"

#include "Server.DarkSouls3/Server/DS3_Game.h"
#include "Server.DarkSouls2/Server/DS2_Game.h"

#include <memory>
#include <vector>
#include <filesystem>
#include <queue>
#include <unordered_map>

// Core of this application, manages all the 
// network services that ds3 uses. 

class Service;
class NetHttpRequest;
class NetHttpResponse;
class GameClient;
class ServerManager;
class Game;

enum class DiscordNoticeType
{
    AntiCheat,
    Bell,
    DiedToBoss,
    KilledBoss,
    UndeadMatch,
    SummonSign,
    SummonSignPvP,
    PvPKill,
    BonfireLit
};

class Server
{
public:
    Server(const std::string& InServerId, const std::string& InServerName, const std::string& InServerPassword, GameType InType, ServerManager* InManager);
    virtual ~Server();

    bool Init();
    bool Term();
    void Poll();

    void SaveConfig();

    const RuntimeConfig& GetConfig()    { return Config; }
    RuntimeConfig& GetMutableConfig()   { return Config; }
    ServerDatabase& GetDatabase()       { return Database; }

    NetIPAddress GetPublicIP()          { return PublicIP; }
    NetIPAddress GetPrivateIP()         { return PrivateIP; }

    std::string GetId()                 { return ServerId; }
    bool IsDefaultServer()              { return ServerId == "default"; }
    ServerManager& GetManager()         { return *Manager; }

    GameType GetGameType()              { return ServerGameType; }
    Game& GetGameInterface()            { return *GameInterface; }

    std::filesystem::path GetSavedPath(){ return SavedPath; }

    template <typename T>
    std::shared_ptr<T> GetService()
    {
        for (auto Service : Services)
        {
            if (std::shared_ptr<T> Result = std::dynamic_pointer_cast<T>(Service))
            {
                return Result;
            }
        }

        return nullptr;
    }

    struct DiscordField
    {
        std::string name;
        std::string value;
        bool is_inline;
    };

    void SendDiscordNotice(
        std::shared_ptr<GameClient> origin, 
        DiscordNoticeType noticeType, 
        const std::string& message, 
        uint32_t extraId = 0,
        std::vector<DiscordField> customFields = {});

    size_t GetSecondsSinceLastActivity();

protected:

    struct PendingDiscordNotice
    {
        std::shared_ptr<GameClient> origin;
        DiscordNoticeType type;
        std::string message;
        uint32_t extraId;
        std::vector<DiscordField> customFields;
    };

    void PollDiscordNotices();

    void CancelServerAdvertisement();
    void PollServerAdvertisement();
    bool ParseServerAdvertisementResponse(std::shared_ptr<NetHttpResponse> Response, nlohmann::json& json);

private:

    RuntimeConfig Config;

    std::vector<std::shared_ptr<Service>> Services;

    std::unique_ptr<Game> GameInterface;

    GameType ServerGameType;
    GameType DefaultServerGameType;

    ServerManager* Manager;

    ServerDatabase Database;

    double NextKeepAliveTime = 0.0;

    std::string ServerId;

    std::string DefaultServerName;
    std::string DefaultServerPassword;

    std::filesystem::path SavedPath;
    std::filesystem::path ConfigPath;
    std::filesystem::path PrivateKeyPath;
    std::filesystem::path PublicKeyPath;
    std::filesystem::path Ds3osconfigPath;
    std::filesystem::path DatabasePath;
    std::filesystem::path KeepAliveFilePath;

    NetIPAddress PublicIP;
    NetIPAddress PrivateIP;

    RSAKeyPair PrimaryKeyPair;

    double NextSpikeTime = 0.0;

    double LastMasterServerUpdate = 0.0;
    std::shared_ptr<NetHttpRequest> MasterServerUpdateRequest;

    std::queue<PendingDiscordNotice> PendingDiscordNotices;
    std::shared_ptr<NetHttpRequest> DiscordNoticeRequest;

    constexpr static inline double k_DiscordOriginCooldownMin = 10.0f;

    std::unordered_map<uint32_t, double> DiscordOriginCooldown;

    static inline std::string BonfireThumbnail          = "https://i.imgur.com/zyHeayN.png";
    static inline std::string RedSoapstoneThumbnail     = "https://i.imgur.com/I251YBN.png";
    static inline std::string WhiteSoapstoneThumbnail   = "https://i.imgur.com/kOXzqso.png";

};