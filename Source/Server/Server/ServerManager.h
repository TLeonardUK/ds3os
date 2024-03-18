/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Server/Server.h"

#include "Shared/Platform/Platform.h"

#include <memory>
#include <vector>
#include <filesystem>
#include <queue>
#include <unordered_map>
#include <functional>

// The server manager essentially manages multiple server shards running on the same machine.

class ServerManager
{
public:
    ServerManager();
    ~ServerManager();

    bool Init();
    bool Term();
    void RunUntilQuit();

    int GetFreeGamePort();

    Server* GetDefaultServer();
    Server* FindServer(const std::string& Id);

    bool NewServer(const std::string& Name, const std::string& Password, GameType InGameType, std::string& OutServerId);

    void QueueCallback(std::function<void()> callback);

private:
    bool StartServer(const std::string& ServerId, const std::string& Name = "", const std::string& Password = "", GameType InGameType = GameType::Unknown);
    bool StopServer(const std::string& ServerId, bool Permanent = false);

    void PruneOldServers();

private:

    bool QuitRecieved = false;

    double NextServerPruneTime = 0.0f;

    std::recursive_mutex m_mutex;

    PlatformEvents::CtrlSignalEvent::DelegatePtr CtrlSignalHandle = nullptr;

    std::filesystem::path SavedPath;

    std::vector<std::unique_ptr<Server>> ServerInstances;

    std::mutex CallbackMutex;
    std::vector<std::function<void()>> Callbacks;

    double LastStatsPrint = GetSeconds();

};