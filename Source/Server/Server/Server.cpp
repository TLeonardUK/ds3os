/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/Server.h"
#include "Server/Service.h"
#include "Server/LoginService/LoginService.h"
#include "Server/AuthService/AuthService.h"
#include "Server/GameService/GameService.h"
#include "Core/Utils/Logging.h"
#include "Core/Utils/File.h"

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

    // Write out the server import file with the latest configuration.
    nlohmann::json Output;
    Output["Name"]          = Config.ServerName;
    Output["Description"]   = Config.ServerDescription;
    Output["Hostname"]      = Config.ServerHostname;
    Output["PublicKey"]     = PrimaryKeyPair.GetPublicString();

    if (!WriteTextToFile(Ds3osconfigPath, Output.dump(4)))
    {
        Error("Failed to write ds3osconfig file to: %s", Ds3osconfigPath.string().c_str());
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

    return true;
}

bool Server::Term()
{
    Log("Terminating server ...");

    for (auto& Service : Services)
    {
        if (!Service->Term())
        {
            Error("Failed to terminate '%s' service.", Service->GetName().c_str());
            return false;
        }
    }

    return true;
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

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}