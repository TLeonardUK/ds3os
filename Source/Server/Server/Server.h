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

#include "Platform/Platform.h"

#include "Core/Crypto/RSAKeyPair.h"

#include "Core/Network/NetIPAddress.h"

#include "Config/RuntimeConfig.h"

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

enum class DiscordNoticeType
{
    AntiCheat,
    Bell,
    DiedToBoss,
    KilledBoss,
    UndeadMatch,
    SummonSign,
    PvPKill,
};

class Server
{
public:
    Server();
    ~Server();

    bool Init();
    bool Term();
    void RunUntilQuit();

    void SaveConfig();

    const RuntimeConfig& GetConfig()    { return Config; }
    RuntimeConfig& GetMutableConfig()   { return Config; }
    ServerDatabase& GetDatabase()       { return Database; }

    NetIPAddress GetPublicIP()          { return PublicIP; }
    NetIPAddress GetPrivateIP()         { return PrivateIP; }

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

    bool QuitRecieved = false;

    PlatformEvents::CtrlSignalEvent::DelegatePtr CtrlSignalHandle = nullptr;

    RuntimeConfig Config;

    std::vector<std::shared_ptr<Service>> Services;

    ServerDatabase Database;

    std::filesystem::path SavedPath;
    std::filesystem::path ConfigPath;
    std::filesystem::path PrivateKeyPath;
    std::filesystem::path PublicKeyPath;
    std::filesystem::path Ds3osconfigPath;
    std::filesystem::path DatabasePath;

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

    static inline std::unordered_map<BossId, std::string> DiscordBossThumbnails = {
        { BossId::Spear_of_the_Church,              "https://i.imgur.com/mIOPfle.jpeg" },
        { BossId::Vordt_of_the_Boreal_Valley,       "https://i.imgur.com/Jzp2qJb.png" },
        { BossId::Oceiros_the_Consumed_King,        "https://i.imgur.com/bWnnWbq.png" },
        { BossId::Dancer_of_the_Boreal_Valley,      "https://i.imgur.com/EKcsNcx.png" },
        { BossId::Dragonslayer_Armour,              "https://i.imgur.com/EdFD3kY.png" },
        { BossId::Curse_rotted_Greatwood,           "https://i.imgur.com/BrPD6Ac.png" },
        { BossId::Ancient_Wyvern,                   "https://i.imgur.com/UDvq8KJ.png" },
        { BossId::King_of_the_Storm,                "https://i.imgur.com/8m67IlX.png" },
        { BossId::Nameless_King,                    "https://i.imgur.com/8m67IlX.png" },
        { BossId::Abyss_Watchers,                   "https://i.imgur.com/D7R7kEZ.png" },
        { BossId::Crystal_Sage,                     "https://i.imgur.com/vDundzi.png" },
        { BossId::Lorian_Elder_Prince,              "https://i.imgur.com/J2jflUK.png" },
        { BossId::Lothric_Younger_Prince,           "https://i.imgur.com/J2jflUK.png" },
        { BossId::Lorian_Elder_Prince_2,            "https://i.imgur.com/J2jflUK.png" },
        { BossId::Deacons_of_the_Deep,              "https://i.imgur.com/eH6g1TO.png" },
        { BossId::Aldrich_Devourer_Of_Gods,         "https://i.imgur.com/FczwbhQ.png" },
        { BossId::Pontiff_Sulyvahn,                 "https://i.imgur.com/VwOi0qc.png" },
        { BossId::High_Lord_Wolnir,                 "https://i.imgur.com/xdYcHi1.png" },
        { BossId::Old_Demon_King,                   "https://i.imgur.com/UGO8MEm.png" },
        { BossId::Yhorm_the_Giant,                  "https://i.imgur.com/KpJc8p2.png" },
        { BossId::Iudex_Gundyr,                     "https://i.imgur.com/eZ2KtGU.png" },
        { BossId::Champion_Gundyr,                  "https://i.imgur.com/ZX4KwhL.png" },
        { BossId::Soul_Of_Cinder,                   "https://i.imgur.com/KRKJAow.jpeg" },
        { BossId::Blackflame_Friede,                "https://i.imgur.com/Ja8rb4n.jpeg" },
        { BossId::Sister_Friede,                    "https://i.imgur.com/Ja8rb4n.jpeg" },
        { BossId::Father_Ariandel_And_Friede,       "https://i.imgur.com/Ja8rb4n.jpeg" },
        { BossId::Gravetender_Greatwolf,            "https://i.imgur.com/k5ngpvT.jpeg" },
        { BossId::Demon_From_Below,                 "https://i.imgur.com/D4NtdCf.jpeg" },
        { BossId::Demon_In_Pain,                    "https://i.imgur.com/D4NtdCf.jpeg" },
        { BossId::Halflight,                        "https://i.imgur.com/mIOPfle.jpeg" },
        { BossId::Darkeater_Midir,                  "https://i.imgur.com/uuMfxNQ.jpeg" },
        { BossId::Slave_Knight_Gael,                "https://i.imgur.com/5mmzjvy.jpeg" }
    };

};