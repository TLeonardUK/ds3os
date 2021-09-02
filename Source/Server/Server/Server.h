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

#include <memory>
#include <vector>
#include <filesystem>

#include "Core/Crypto/RSAKeyPair.h"

#include "Core/Network/NetIPAddress.h"

#include "Config/RuntimeConfig.h"

// Core of this application, manages all the 
// network services that ds3 uses. 

class Service;

class Server
{
public:
    Server();
    ~Server();

    bool Init();
    bool Term();
    void RunUntilQuit();

    const RuntimeConfig& GetConfig()    { return Config; }
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

};