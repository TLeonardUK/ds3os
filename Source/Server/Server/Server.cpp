/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/Server.h"
#include "Server/ServerManager.h"
#include "Server/Config/BuildConfig.h"
#include "Server/Database/ServerDatabase.h"
#include "Server/Service.h"
#include "Server/LoginService/LoginService.h"
#include "Server/AuthService/AuthService.h"
#include "Server/GameService/GameService.h"
#include "Server/GameService/GameClient.h"
#include "Server/WebUIService/WebUIService.h"
#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/File.h"
#include "Shared/Core/Utils/Strings.h"
#include "Shared/Core/Utils/Random.h"
#include "Shared/Core/Utils/DebugObjects.h"
#include "Shared/Core/Network/NetUtils.h"
#include "Shared/Core/Network/NetHttpRequest.h"

#include <thread>
#include <chrono>
#include <fstream>

#include "ThirdParty/nlohmann/json.hpp"
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include <steam/steam_api.h>
#include <steam/steam_gameserver.h>

Server::Server(const std::string& InServerId, const std::string& InServerName, const std::string& InServerPassword, GameType InType, ServerManager* InManager)
    : ServerId(InServerId)
    , Manager(InManager)
    , DefaultServerName(InServerName)
    , DefaultServerPassword(InServerPassword)
    , DefaultServerGameType(InType)
{
    // The keys are always shared between all servers so we can use the same login/auth servers.
//    std::filesystem::path BasePath = std::filesystem::current_path() / std::filesystem::path("Saved");
//    PrivateKeyPath = BasePath / std::filesystem::path("private.key");
//    PublicKeyPath = BasePath / std::filesystem::path("public.key");

    SavedPath = std::filesystem::current_path() / std::filesystem::path("Saved") / InServerId;
    if constexpr (BuildConfig::SUPPORT_LEGACY_IMPORT_FILES)
    {
        Ds3osconfigPath = SavedPath / std::filesystem::path("server.ds3osconfig");
    }
    ConfigPath = SavedPath / std::filesystem::path("config.json");
    DatabasePath = SavedPath / std::filesystem::path("database.sqlite");
    KeepAliveFilePath = SavedPath / std::filesystem::path("last_activity.time");
    PrivateKeyPath = SavedPath / std::filesystem::path("private.key");
    PublicKeyPath = SavedPath / std::filesystem::path("public.key");

    // Register all services we want to run.
    Services.push_back(std::make_shared<LoginService>(this, &PrimaryKeyPair));
    Services.push_back(std::make_shared<AuthService>(this, &PrimaryKeyPair));
    Services.push_back(std::make_shared<GameService>(this, &PrimaryKeyPair));
    Services.push_back(std::make_shared<WebUIService>(this));
}

Server::~Server()
{
}

bool Server::Init()
{
    Log("Initializing server ...");

    // Generate folder we are going to save everything into.
    if (!std::filesystem::is_directory(SavedPath))
    {
        if (!std::filesystem::create_directories(SavedPath))
        {
            Error("Failed to create save path: %s", SavedPath.string().c_str());
            return false;
        }
    }

    // Load configuration if it exists.
    bool IsNewServer = false;
    
    if (std::filesystem::exists(ConfigPath))
    {
        if (!Config.Load(ConfigPath))
        {
            Error("Failed to load configuration file: %s", ConfigPath.string().c_str());
            return false;
        }
    }
    else
    {
        // If not default server then generating some unique webui credentials.
        if (!IsDefaultServer())
        {
            Config.WebUIServerPassword = RandomPassword();
            Config.WebUIServerUsername = RandomName();
            Config.ServerName = DefaultServerName;
            Config.Password = DefaultServerPassword;
            Config.GameType = GameTypeStrings[(int)DefaultServerGameType];
        }

        if (!Config.Save(ConfigPath))
        {
            Error("Failed to save configuration file: %s", ConfigPath.string().c_str());
            return false;
        }
    }

    // Disable all but error messages if configured.
    if (Config.QuietLogging)
    {
        SetQuietLogging(true);
    }

    // Determine game type.
    if (!ParseGameType(Config.GameType.c_str(), ServerGameType))
    {
        Error("Unknown game type: %s", Config.GameType.c_str());
        return false;
    }

    // Setup steam for this server.
    if (IsDefaultServer())
    {
        std::filesystem::path appid_path = std::filesystem::current_path() / "steam_appid.txt";

        size_t appid = BuildConfig::GameConfig[(int)ServerGameType].STEAM_APPID;
        WriteTextToFile(appid_path, std::to_string(appid));

        Log("Initializing steam game server api.");

        if (!SteamGameServer_Init(0, Config.LoginServerPort, MASTERSERVERUPDATERPORT_USEGAMESOCKETSHARE, eServerModeAuthentication, "1.0.0.0"))
        {
            Error("Failed to initialize steam game server api.");
            return false;
        }

        Log("Initialized steam game server api.");
    }

    // Create game interface for this server.
    switch (ServerGameType)
    {
        case GameType::DarkSouls2:
        {
            GameInterface = std::make_unique<DS2_Game>();
            break;
        }
        case GameType::DarkSouls3:
        {
            GameInterface = std::make_unique<DS3_Game>();
            break;
        }
        default:
        {
            assert(false);
            return false;
        }
    }

    // Patch old server ip.
#ifdef _DEBUG
    //Config.MasterServerIp = "127.0.0.1";
    Config.MasterServerIp = "ds3os-master.timleonard.uk";
    //Config.ServerName = "Debugging Server";
    //Config.ServerDescription = "Used for debugging by Infini, don't use.";
#else
    if (Config.MasterServerIp == "timleonard.uk")
    {
        Config.MasterServerIp = "ds3os-master.timleonard.uk";
    }
#endif

    // If not the default server we grab a random free port for the game server.
    if (!IsDefaultServer())
    {
        Log("Selecting floating game ports.");
        Config.GameServerPort = Manager->GetFreeGamePort();
        Config.WebUIServerPort = Manager->GetFreeGamePort();
        Config.AuthServerPort = Manager->GetFreeGamePort();
        Config.LoginServerPort = Manager->GetFreeGamePort();
    }

    // Create a server id if one isn't specified already.
    if (Config.ServerId.empty())
    {
        Log("Generating new server id.");
        Config.ServerId = MakeGUID();
        SaveConfig();
    }

    // Generate server encryption keypair if it doesn't already exists.
    if (!std::filesystem::exists(PrivateKeyPath) ||
        !std::filesystem::exists(PublicKeyPath))
    {
        Log("Generating new key pair for server as none exists on disk.");
        if (!PrimaryKeyPair.Generate())
        {
            Error("Failed to generate rsa keypair.");
            return false;            
        }
        if (!PrimaryKeyPair.Save(PrivateKeyPath, PublicKeyPath))
        {
            Error("Failed to save rsa keypair to: %s and %s", PrivateKeyPath.string().c_str(), PublicKeyPath.string().c_str());
            return false;
        }

        Log("Generated rsa key pair successfully.");
    }
    else
    {
        if (!PrimaryKeyPair.Load(PrivateKeyPath))
        {
            Error("Failed to load rsa keypair from: %s and %s", PrivateKeyPath.string().c_str(), PublicKeyPath.string().c_str());
            return false;
        }

        Log("Loaded rsa key pair successfully.");
    }

    // Fill in IP information of server if not provided.
    if (Config.ServerHostname == "")
    {
        if (!::GetMachineIPv4(PublicIP, true))
        {
            Error("Failed to resolve public ip address of server.");
            return false;
        }
    }
    else
    {
        // Convert hostname into IP.
        if (!NetIPAddress::FromHostname(Config.ServerHostname, PublicIP))
        {
            Error("Failed to resolve ip from hostname '%s'.", Config.ServerHostname.c_str());
            return false;
        }
    }

    if (Config.ServerPrivateHostname == "")
    {
        if (!::GetMachineIPv4(PrivateIP, false))
        {
            Error("Failed to resolve private ip address of server.");
            return false;
        }
    }
    else
    {
        // Convert hostname into IP.
        if (!NetIPAddress::FromHostname(Config.ServerPrivateHostname, PrivateIP))
        {
            Error("Failed to resolve ip from hostname '%s'.", Config.ServerPrivateHostname.c_str());
            return false;
        }
    }

    Log("Public ip address: %s", PublicIP.ToString().c_str());
    Log("Private ip address: %s", PrivateIP.ToString().c_str());

    if constexpr (BuildConfig::SUPPORT_LEGACY_IMPORT_FILES)
    {
        // Write out the server import file with the latest configuration.
        nlohmann::json Output;
        Output["Name"]              = Config.ServerName;
        Output["Description"]       = Config.ServerDescription;
        Output["Hostname"]          = Config.ServerHostname.length() > 0 ? Config.ServerHostname : PublicIP.ToString();
        Output["PrivateHostname"]   = Config.ServerPrivateHostname.length() > 0 ? Config.ServerPrivateHostname : PrivateIP.ToString();
        Output["PublicKey"]         = PrimaryKeyPair.GetPublicString();
        Output["ModsWhitelist"]     = Config.ModsWhitelist;
        Output["ModsBlacklist"]     = Config.ModsBlacklist;
        Output["ModsRequiredList"]  = Config.ModsRequiredList;

        if (!WriteTextToFile(Ds3osconfigPath, Output.dump(4)))
        {
            Error("Failed to write ds3osconfig file to: %s", Ds3osconfigPath.string().c_str());
            return false;
        }
    }

    // Open connection to our database.
    if (!Database.Open(DatabasePath))
    {
        Error("Failed to open database at '%s'.", DatabasePath.string().c_str());
        return false;
    }

    // Initialize all our services.
    for (auto& Service : Services)
    {
        if (!Service->Init())
        {
            Error("Failed to initialize '%s' service.", Service->GetName().c_str());
            return false;
        }
    }
    
#if 0
#define WriteState(State, bEnabled) WriteLog(false, bEnabled ? ConsoleColor::Green : ConsoleColor::Red, "", "Log", "%-25s: %s", State, bEnabled ? "Enabled" : "Disabled");
    WriteState("Blood Messages", !Config.DisableBloodMessages);
    WriteState("Blood Stains", !Config.DisableBloodStains);
    WriteState("Blood Ghosts", !Config.DisableGhosts);
    WriteState("Invasions (Auto Summon)", !Config.DisableInvasionAutoSummon);
    WriteState("Invasions", !Config.DisableInvasions);
    WriteState("Coop (Auto Summon)", !Config.DisableCoopAutoSummon);
    WriteState("Coop", !Config.DisableCoop);
#undef WriteState
#endif

    return true;
}

bool Server::Term()
{
    Log("Terminating server ...");

    CancelServerAdvertisement();

    for (auto& Service : Services)
    {
        if (!Service->Term())
        {
            Error("Failed to terminate '%s' service.", Service->GetName().c_str());
            return false;
        }
    }

    if (!Database.Close())
    {
        Error("Failed to close database.");
        return false;
    }

    if (IsDefaultServer())
    {
        SteamGameServer_Shutdown();
    }

    return true;
}

bool Server::ParseServerAdvertisementResponse(std::shared_ptr<NetHttpResponse> Response, nlohmann::json& json)
{
    try
    {
        std::string JsonResponse;
        JsonResponse.assign((char*)Response->GetBody().data(), Response->GetBody().size());

        json = nlohmann::json::parse(JsonResponse);
        if (!json.contains("status"))
        {
            Warning("Recieved error when trying to advertise server on master server. Malformed output.");
            return false;
        }
        else if (json["status"] != "success")
        {
            if (json.contains("message"))
            {
                std::string message = json["message"];
                Warning("Recieved error when trying to advertise server on master server. Error message: %s", message.c_str());
                return false;
            }
            else
            {
                Warning("Recieved error when trying to advertise server on master server. No error message provided.");
                return false;
            }
        }
    }
    catch (nlohmann::json::parse_error)
    {
        Log("Unable to parse result when trying to advertise server on master server. Response was not valid json. This isn't an issue, the master server is probably just down or overloaded.");
        return false;
    }

    return true;
}

void Server::CancelServerAdvertisement()
{
    if (!Config.Advertise)
    {
        return;
    }

    Log("Canceling advertisement on master server.");

    std::shared_ptr<NetHttpRequest> Request = std::make_shared<NetHttpRequest>();
    Request->SetMethod(NetHttpMethod::METHOD_DELETE);
    Request->SetUrl(StringFormat("http://%s:%i/api/v1/servers", Config.MasterServerIp.c_str(), Config.MasterServerPort));
    if (!Request->Send())
    {
        Warning("Recieved error when trying to advertise server on master server. Failed to start request.");
    }
    else
    {
        if (std::shared_ptr<NetHttpResponse> Response = Request->GetResponse(); Response && Response->GetWasSuccess())
        {
            nlohmann::json json;
            ParseServerAdvertisementResponse(Response, json);
        }
    }

    Request = nullptr;
}

void Server::PollServerAdvertisement()
{
    if (!Config.Advertise)
    {
        return;
    }
    if constexpr (!BuildConfig::SEND_MESSAGE_TO_PLAYERS_SANITY_CHECKS)
    {
        Warning("Security fixes for RequestSendMessageToPlayers or the NRSSR RCE exploit are disabled. As such, the server will not be listed publicly.");
        Config.Advertise = false;
        return;
    }

    // Waiting for current advertisement to finish.
    if (MasterServerUpdateRequest)
    {
        if (!MasterServerUpdateRequest->InProgress())
        {
            if (std::shared_ptr<NetHttpResponse> Response = MasterServerUpdateRequest->GetResponse(); Response && Response->GetWasSuccess())
            {
                nlohmann::json json;
                ParseServerAdvertisementResponse(Response, json);
            }

            LastMasterServerUpdate = GetSeconds();
            MasterServerUpdateRequest = nullptr;
        }
    }

    // Is it time to kick off a new one?
    else if (GetSeconds() - LastMasterServerUpdate > Config.AdvertiseHearbeatTime)
    {
        nlohmann::json Body;
        Body["ServerId"] = Config.ServerId;
        Body["Hostname"] = Config.ServerHostname.length() > 0 ? Config.ServerHostname : PublicIP.ToString();
        Body["PrivateHostname"] = Config.ServerPrivateHostname.length() > 0 ? Config.ServerPrivateHostname : PrivateIP.ToString();
        Body["Description"] = Config.ServerDescription;
        Body["Name"] = Config.ServerName;
        Body["PublicKey"] = PrimaryKeyPair.GetPublicString();
        Body["PlayerCount"] = (int)GetService<GameService>()->GetClients().size();
        Body["Password"] = Config.Password;
        Body["ModsWhiteList"] = Config.ModsWhitelist;
        Body["ModsBlackList"] = Config.ModsBlacklist;
        Body["ModsRequiredList"] = Config.ModsRequiredList;        
        Body["ServerVersion"] = BuildConfig::MASTER_SERVER_CLIENT_VERSION;
        Body["AllowSharding"] = Config.SupportSharding;
        Body["WebAddress"] = Config.SupportSharding ? StringFormat("http://%s:%i", ((std::string)Body["Hostname"]).c_str(), Config.WebUIServerPort) : "";
        Body["Port"] = Config.LoginServerPort;
        Body["IsShard"] = !IsDefaultServer();
        Body["GameType"] = GameTypeStrings[(int)ServerGameType];

        MasterServerUpdateRequest = std::make_shared<NetHttpRequest>();
        MasterServerUpdateRequest->SetMethod(NetHttpMethod::POST);
        MasterServerUpdateRequest->SetBody(Body.dump(4));
        MasterServerUpdateRequest->SetUrl(StringFormat("http://%s:%i/api/v1/servers", Config.MasterServerIp.c_str(), Config.MasterServerPort));
        if (!MasterServerUpdateRequest->SendAsync())
        {
            Warning("Recieved error when trying to advertise server on master server. Failed to start request.");
            MasterServerUpdateRequest = nullptr;
        }
    }
}

void Server::Poll()
{
    {
        DebugTimerScope Scope(Debug::UpdateTime);

        for (auto& Service : Services)
        {
            Service->Poll();
        }

        PollDiscordNotices();
        PollServerAdvertisement();
    }

    // Emulate frametime spikes.
    if constexpr (BuildConfig::EMULATE_SPIKES)
    {
        if (GetSeconds() > NextSpikeTime)
        {
            double DurationMs = FRandRange(BuildConfig::SPIKE_LENGTH_MIN, BuildConfig::SPIKE_LENGTH_MAX);
            double IntervalMs = FRandRange(BuildConfig::SPIKE_INTERVAL_MIN, BuildConfig::SPIKE_INTERVAL_MAX);

            Log("Emulating spike of %.2f ms", DurationMs);

            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<size_t>(DurationMs)));

            NextSpikeTime = GetSeconds() + (IntervalMs / 1000.0);
        }
    }

    // Write last activity time periodically to disk to keep this server instance alive.
    if (GetSeconds() > NextKeepAliveTime)
    {
        if (!(int)GetService<GameService>()->GetClients().empty() || 
            !std::filesystem::exists(KeepAliveFilePath))
        {
            const auto TimePoint = std::chrono::system_clock::now();
            const size_t SecondsSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(TimePoint.time_since_epoch()).count();

            WriteTextToFile(KeepAliveFilePath, StringFormat("%llu", SecondsSinceEpoch));
        }

        NextKeepAliveTime = GetSeconds() + 60.0f;
    }
}

size_t Server::GetSecondsSinceLastActivity()
{
    if (std::filesystem::exists(KeepAliveFilePath))
    {
        std::string Output;
        if (ReadTextFromFile(KeepAliveFilePath, Output))
        {
            const auto TimePoint = std::chrono::system_clock::now();
            const size_t SecondsSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(TimePoint.time_since_epoch()).count();

            const size_t LastActivity = std::stoull(Output.c_str());

            return (SecondsSinceEpoch - LastActivity);
        }
    }
    return 0;
}

void Server::SaveConfig()
{
    if (!Config.Save(ConfigPath))
    {
        Error("Failed to save configuration file: %s", ConfigPath.string().c_str());
    }
}

void Server::SendDiscordNotice(
    std::shared_ptr<GameClient> origin, 
    DiscordNoticeType noticeType, 
    const std::string& message,
    uint32_t extraId,
    std::vector<DiscordField> customFields)
{
    if (Config.DiscordWebHookUrl.empty())
    {
        return;
    }

    uint32_t PlayerId = origin->GetPlayerState().GetPlayerId();

    if (auto Iter = DiscordOriginCooldown.find(PlayerId); Iter != DiscordOriginCooldown.end())
    {
        float Elapsed = GetSeconds() - Iter->second;
        if (Elapsed < k_DiscordOriginCooldownMin)
        {
            return;
        }
    }

    DiscordOriginCooldown[PlayerId] = GetSeconds();

    PendingDiscordNotices.push({ origin, noticeType, message, extraId, customFields });
}

void Server::PollDiscordNotices()
{
    if (!DiscordNoticeRequest)
    {
        if (!PendingDiscordNotices.empty())
        {
            PendingDiscordNotice Notice = PendingDiscordNotices.front();
            PendingDiscordNotices.pop();

            auto embeds = nlohmann::json::array();

            uint64_t SteamId64;
            sscanf(Notice.origin->GetPlayerState().GetSteamId().c_str(), "%016llx", &SteamId64);

            auto author = nlohmann::json::object();
            auto embed = nlohmann::json::object();

            std::string thumbnailUrl = "";

            switch (Notice.type)
            {
                case DiscordNoticeType::AntiCheat:
                {
                    embed["color"] = "14423100"; // Red
                    break;
                }
                case DiscordNoticeType::Bell:
                {
                    embed["color"] = "8900346"; // Blue
                    break;
                }
                case DiscordNoticeType::UndeadMatch:
                {
                    embed["color"] = "3329330"; // Green
                    break;
                }
                case DiscordNoticeType::SummonSign:
                {
                    embed["color"] = "16316671"; // White
                    thumbnailUrl = WhiteSoapstoneThumbnail;
                    break;
                }
                case DiscordNoticeType::SummonSignPvP:
                {
                    embed["color"] = "14423100"; // Red
                    thumbnailUrl = RedSoapstoneThumbnail;
                    break;
                }
                case DiscordNoticeType::PvPKill:
                {
                    embed["color"] = "14423100"; // Red
                    break;
                }
                case DiscordNoticeType::BonfireLit:
                {
                    embed["color"] = "16747520"; // Orange
                    thumbnailUrl = BonfireThumbnail;
                    break;
                }
                case DiscordNoticeType::DiedToBoss:
                case DiscordNoticeType::KilledBoss:
                {
                    if (Notice.type  == DiscordNoticeType::KilledBoss)
                    {
                        embed["color"] = "16766720"; // Gold
                    }
                    else
                    {
                        embed["color"] = "14423100"; // Red
                    }

                    thumbnailUrl = GameInterface->GetBossDiscordThumbnailUrl(Notice.extraId);

                    break;
                }
            }

            if (!Notice.customFields.empty())
            {
                auto fieldObj = nlohmann::json::array();

                for (auto& pair : Notice.customFields)
                {
                    auto field = nlohmann::json::object();
                    field["name"] = pair.name;
                    field["value"] = pair.value;
                    field["inline"] = pair.is_inline;

                    fieldObj.push_back(field);
                }

                embed["fields"] = fieldObj;
            }

            author["name"] = Notice.origin->GetPlayerState().GetCharacterName();
            author["url"] = StringFormat("https://steamcommunity.com/profiles/%s", std::to_string(SteamId64).c_str());

            embed["author"] = author;
            embed["description"] = Notice.message;

            if (!thumbnailUrl.empty())
            {
                embed["thumbnail"] = nlohmann::json::object();
                embed["thumbnail"]["url"] = thumbnailUrl;
            }

            embeds.push_back(embed);

            nlohmann::json Body;
            Body["embeds"] = embeds;

            Log("Sending notification from %s: %s", Notice.origin->GetPlayerState().GetCharacterName().c_str(), Notice.message.c_str());

            std::string FormattedBody = Body.dump(4);

            DiscordNoticeRequest = std::make_shared<NetHttpRequest>();
            DiscordNoticeRequest->SetMethod(NetHttpMethod::POST);
            DiscordNoticeRequest->SetUrl(Config.DiscordWebHookUrl);
            DiscordNoticeRequest->SetBody(FormattedBody);
            if (!DiscordNoticeRequest->SendAsync())
            {
                Warning("Recieved error when trying to send discord notification.");
            }
        }
    }
    else if (!DiscordNoticeRequest->InProgress())
    {
        if (std::shared_ptr<NetHttpResponse> Response = DiscordNoticeRequest->GetResponse(); Response)
        {
            if (!Response->GetWasSuccess())
            {
                Warning("Recieved error when trying to send discord notification.");
            }
            else
            {
                Log("Discord notice successfully sent.");
            }
        }
        else
        {
            Warning("No response recieved to discord notification.");
        }

        DiscordNoticeRequest = nullptr;
    }
}