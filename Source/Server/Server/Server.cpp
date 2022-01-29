/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/Server.h"
#include "Server/Database/ServerDatabase.h"
#include "Server/Service.h"
#include "Server/LoginService/LoginService.h"
#include "Server/AuthService/AuthService.h"
#include "Server/GameService/GameService.h"
#include "Core/Utils/Logging.h"
#include "Core/Utils/File.h"
#include "Core/Utils/Strings.h"
#include "Core/Network/NetUtils.h"
#include "Core/Network/NetHttpRequest.h"

#include <thread>
#include <chrono>
#include <fstream>

#include <nlohmann/json.hpp>
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
            LogError("Failed to create save path: %s", SavedPath.string().c_str());
            return false;
        }
    }

    // Load configuration if it exists.
    if (std::filesystem::exists(ConfigPath))
    {
        if (!Config.Load(ConfigPath))
        {
            LogError("Failed to load configuration file: %s", ConfigPath.string().c_str());
            return false;
        }
    }
    else
    {
        if (!Config.Save(ConfigPath))
        {
            LogError("Failed to save configuration file: %s", ConfigPath.string().c_str());
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
            LogError("Failed to generate rsa keypair.");
            return false;            
        }
        if (!PrimaryKeyPair.Save(PrivateKeyPath, PublicKeyPath))
        {
            LogError("Failed to save rsa keypair to: %s and %s", PrivateKeyPath.string().c_str(), PublicKeyPath.string().c_str());
            return false;
        }

        Log("Generated rsa key pair successfully.");
    }
    else
    {
        if (!PrimaryKeyPair.Load(PrivateKeyPath))
        {
            LogError("Failed to load rsa keypair from: %s and %s", PrivateKeyPath.string().c_str(), PublicKeyPath.string().c_str());
            return false;
        }

        Log("Loaded rsa key pair successfully.");
    }

    // Fill in IP information of server if not provided.
    if (Config.ServerHostname == "")
    {
        if (!::GetMachineIPv4(PublicIP, true))
        {
            LogError("Failed to resolve public ip address of server.");
            return false;
        }
    }
    else
    {
        // Convert hostname into IP.
        if (!NetIPAddress::FromHostname(Config.ServerHostname, PublicIP))
        {
            LogError("Failed to resolve ip from hostname '%s'.", Config.ServerHostname.c_str());
            return false;
        }
    }

    if (Config.ServerPrivateHostname == "")
    {
        if (!::GetMachineIPv4(PrivateIP, false))
        {
            LogError("Failed to resolve private ip address of server.");
            return false;
        }
    }
    else
    {
        // Convert hostname into IP.
        if (!NetIPAddress::FromHostname(Config.ServerPrivateHostname, PrivateIP))
        {
            LogError("Failed to resolve ip from hostname '%s'.", Config.ServerPrivateHostname.c_str());
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
        LogError("Failed to write ds3osconfig file to: %s", Ds3osconfigPath.string().c_str());
        return false;
    }

    // Open connection to our database.
    if (!Database.Open(DatabasePath))
    {
        LogError("Failed to open database at '%s'.", DatabasePath.string().c_str());
        return false;
    }

    // Initialize all our services.
    for (auto& Service : Services)
    {
        if (!Service->Init())
        {
            LogError("Failed to initialize '%s' service.", Service->GetName().c_str());
            return false;
        }
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
            LogError("Failed to terminate '%s' service.", Service->GetName().c_str());
            return false;
        }
    }

    if (!Database.Close())
    {
        LogError("Failed to close database.");
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
        Body["PrivateHostname"] = Config.ServerPrivateHostname.length() > 0 ? Config.ServerPrivateHostname : PublicIP.ToString();
        Body["Description"] = Config.ServerDescription;
        Body["Name"] = Config.ServerName;
        Body["PublicKey"] = PrimaryKeyPair.GetPublicString();
        Body["PlayerCount"] = (int)GetService<GameService>()->GetClients().size();
        Body["Password"] = Config.Password;
        Body["ModsWhiteList"] = Config.ModsWhitelist;
        Body["ModsBlackList"] = Config.ModsBlacklist;
        Body["ModsRequiredList"] = Config.ModsRequiredList;        

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
        for (auto& Service : Services)
        {
            Service->Poll();
        }

        PollServerAdvertisement();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}