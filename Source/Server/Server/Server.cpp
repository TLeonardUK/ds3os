/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/Server.h"
#include "Server/Config/BuildConfig.h"
#include "Server/Database/ServerDatabase.h"
#include "Server/Service.h"
#include "Server/LoginService/LoginService.h"
#include "Server/AuthService/AuthService.h"
#include "Server/GameService/GameService.h"
#include "Server/GameService/GameClient.h"
#include "Server/WebUIService/WebUIService.h"
#include "Core/Utils/Logging.h"
#include "Core/Utils/File.h"
#include "Core/Utils/Strings.h"
#include "Core/Utils/Random.h"
#include "Core/Utils/DebugObjects.h"
#include "Core/Network/NetUtils.h"
#include "Core/Network/NetHttpRequest.h"

#include <thread>
#include <chrono>
#include <fstream>

#include "ThirdParty/nlohmann/json.hpp"
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

Server::Server()
{
    // TODO: Move this stuff into a RuntimeConfig type class.
    SavedPath = std::filesystem::current_path() / std::filesystem::path("Saved");
    PrivateKeyPath = SavedPath / std::filesystem::path("private.key");
    PublicKeyPath = SavedPath / std::filesystem::path("public.key");
    Ds3osconfigPath = SavedPath / std::filesystem::path("server.ds3osconfig");
    ConfigPath = SavedPath / std::filesystem::path("config.json");
    DatabasePath = SavedPath / std::filesystem::path("database.sqlite");

    // Register for Ctrl+C notifications, its the only way the server shuts down right now.
    CtrlSignalHandle = PlatformEvents::OnCtrlSignal.Register([=]() {
        Warning("Quit signal recieved, starting shutdown.");        
        QuitRecieved = true;
    });

    // Register all services we want to run.
    Services.push_back(std::make_shared<LoginService>(this, &PrimaryKeyPair));
    Services.push_back(std::make_shared<AuthService>(this, &PrimaryKeyPair));
    Services.push_back(std::make_shared<GameService>(this, &PrimaryKeyPair));
    Services.push_back(std::make_shared<WebUIService>(this));
}

Server::~Server()
{
    CtrlSignalHandle.reset();
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
        if (!Config.Save(ConfigPath))
        {
            Error("Failed to save configuration file: %s", ConfigPath.string().c_str());
            return false;
        }
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
    
#define WriteState(State, bEnabled) WriteLog(bEnabled ? ConsoleColor::Green : ConsoleColor::Red, "", "Log", "%-25s: %s", State, bEnabled ? "Enabled" : "Disabled");
    WriteState("Blood Messages", !Config.DisableBloodMessages);
    WriteState("Blood Stains", !Config.DisableBloodStains);
    WriteState("Blood Ghosts", !Config.DisableGhosts);
    WriteState("Invasions (Auto Summon)", !Config.DisableInvasionAutoSummon);
    WriteState("Invasions", !Config.DisableInvasions);
    WriteState("Coop (Auto Summon)", !Config.DisableCoopAutoSummon);
    WriteState("Coop", !Config.DisableCoop);
#undef WriteState

    if (!Config.DisableBloodMessages || !Config.DisableBloodStains || !Config.DisableGhosts)
    {
        Error(
            "\n\n"
            "=============================================== WARNING ===============================================\n"
            " Blood messages, stains, ghosts and other serialized data that is client-generated and client-parsed\n"
            " are vulnerable to CVE-2022-24125 and CVE-2022-24126.\n"
            "\n"
            " Until FROM SOFTWARE patch the client it is not advised to run with these enabled, unless you are only\n"
            " permitting access to the server to only people you trust.\n"
            "=======================================================================================================\n"
            "\n\n"
        );
    }

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
    if constexpr (!BuildConfig::SEND_MESSAGE_TO_PLAYERS_SANITY_CHECKS || !BuildConfig::NRSSR_SANITY_CHECKS)
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

void Server::RunUntilQuit()
{
    Success("Server is now running.");

    // We should really do this event driven ...
    // This suffices for now.
    while (!QuitRecieved)
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

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Server::SaveConfig()
{
    if (!Config.Save(ConfigPath))
    {
        Error("Failed to save configuration file: %s", ConfigPath.string().c_str());
    }
}

void Server::SendDiscordNotice(std::shared_ptr<GameClient> origin, DiscordNoticeType noticeType, const std::string& message, uint32_t extraId)
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

    PendingDiscordNotices.push({ origin, noticeType, message, extraId });
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

            bool attachSoulLevels = false;
            std::unordered_map<std::string, std::string> fields;
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
                    attachSoulLevels = true;
                    break;
                }
                case DiscordNoticeType::SummonSign:
                {
                    embed["color"] = "16316671"; // White
                    attachSoulLevels = true;
                    break;
                }
                case DiscordNoticeType::BossKilled:
                {
                    embed["color"] = "16766720"; // Gold
                    attachSoulLevels = true;

                    BossId bossId = static_cast<BossId>(Notice.extraId);
                    if (auto Iter = DiscordBossThumbnails.find(bossId); Iter != DiscordBossThumbnails.end())
                    {
                        thumbnailUrl = Iter->second;
                    }

                    break;
                }
            }

            if (attachSoulLevels)
            {
                fields["Soul Level"] = std::to_string(Notice.origin->GetPlayerState().GetSoulLevel());
                fields["Weapon Level"] = std::to_string(Notice.origin->GetPlayerState().GetMaxWeaponLevel());
            }

            if (!fields.empty())
            {
                auto fieldObj = nlohmann::json::array();

                for (auto& pair : fields)
                {
                    auto field = nlohmann::json::object();
                    field["name"] = pair.first;
                    field["value"] = pair.second;
                    field["inline"] = true;

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