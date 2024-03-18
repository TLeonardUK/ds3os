/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/ServerManager.h"
#include "Server/Server.h"
#include "Config/BuildConfig.h"
#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/File.h"
#include "Shared/Core/Utils/Strings.h"
#include "Shared/Core/Utils/Random.h"
#include "Shared/Core/Utils/DebugCounter.h"
#include "Shared/Core/Utils/DebugObjects.h"
#include "Shared/Core/Utils/DebugTimer.h"

#include <thread>
#include <chrono>
#include <fstream>

ServerManager::ServerManager()
{
    // Register for Ctrl+C notifications, its the only way the server shuts down right now.
    CtrlSignalHandle = PlatformEvents::OnCtrlSignal.Register([=]() {
        Warning("Quit signal recieved, starting shutdown.");        
        QuitRecieved = true;
    });
}

ServerManager::~ServerManager()
{
    CtrlSignalHandle.reset();
}

bool ServerManager::Init()
{
    Log("Initializing server manager ...");

    // Bring up all the servers that we have configuration for.
    SavedPath = std::filesystem::current_path() / std::filesystem::path("Saved");

    // Always bring up the default server first.
    StartServer("default");

    for (auto const& DirEntry : std::filesystem::directory_iterator { SavedPath })
    {
        if (DirEntry.is_directory())
        {
            std::string ServerId = DirEntry.path().filename().string();
            if (ServerId != "default")
            {
                Log("Bringing server '%s' online.", ServerId.c_str());

                if (!StartServer(ServerId))
                {
                    return false;
                }
            }
        }
    }

    return true;
}

bool ServerManager::Term()
{
    Log("Terminating server manager ...");

    std::vector<std::string> ServerIds;
    for (auto& Server : ServerInstances)
    {
        ServerIds.push_back(Server->GetId());
    }

    bool Success = true;

    for (auto& Id : ServerIds)
    {
        Success |= StopServer(Id);
    }

    return Success;
}

void ServerManager::RunUntilQuit()
{
    Success("Server manager is now running.");

    // We should really do this event driven ...
    // This suffices for now.
    while (!QuitRecieved)
    {
        std::scoped_lock lock(m_mutex);

        {
            DebugTimerScope Scope(Debug::AllServerUpdateTime);
            for (auto& Server : ServerInstances)
            {
                Server->Poll();
            }
        }

        if (GetSeconds() > NextServerPruneTime)
        {
            PruneOldServers();
            NextServerPruneTime = GetSeconds() + 60.0f;
        }

        // Execute all callbacks.
        {
            std::scoped_lock lock(CallbackMutex);
            for (auto& callback : Callbacks)
            {
                callback();
            }

            Callbacks.clear();
        }

        // Emit some statistics periodically.
        double Elapsed = GetSeconds() - LastStatsPrint;
        if (Elapsed > 30.0f)
        {
            size_t PlayerCount = 0;
            for (auto& Server : ServerInstances)
            {
                PlayerCount += Server->GetService<GameService>()->GetClients().size();
            }

            WriteLog(true, ConsoleColor::Grey, "", "Log", "%zi players | %zi servers | %.2f ms update | connections auth %.2f login %.2f game %.2f p/s | tcp in %.2f out %.2f kb/s | udp in %.2f out %.2f kb/s | database queries %.2f p/s ",
                PlayerCount,
                ServerInstances.size(),
                Debug::AllServerUpdateTime.GetAverage() * 1000.0f,
                Debug::AuthConnections.GetAverageRate(),
                Debug::LoginConnections.GetAverageRate(),
                Debug::GameConnections.GetAverageRate(),
                (Debug::TcpBytesRecieved.GetAverageRate()) / 1024.0f,
                (Debug::TcpBytesSent.GetAverageRate()) / 1024.0f,
                (Debug::UdpBytesRecieved.GetAverageRate()) / 1024.0f,
                (Debug::UdpBytesSent.GetAverageRate()) / 1024.0f,
                Debug::DatabaseQueries.GetAverageRate()
            );

            LastStatsPrint = GetSeconds();
        }

        DebugCounter::PollAll();
        DebugTimer::PollAll();

        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

bool ServerManager::NewServer(const std::string& Name, const std::string& Password, GameType InGameType, std::string& OutServerId)
{
    std::scoped_lock lock(m_mutex);
    OutServerId = MakeGUID();
    return StartServer(OutServerId, Name, Password, InGameType);
}


void ServerManager::QueueCallback(std::function<void()> callback)
{
    std::scoped_lock lock(CallbackMutex);

    Callbacks.push_back(callback);
}

bool ServerManager::StartServer(const std::string& ServerId, const std::string& Name, const std::string& Password, GameType InGameType)
{
    std::scoped_lock lock(m_mutex);

    Log("Starting server %s ...", ServerId.c_str());

    std::unique_ptr<Server> Instance = std::make_unique<Server>(ServerId, Name, Password, InGameType, this);
    Server* InstancePtr = Instance.get();
    ServerInstances.push_back(std::move(Instance));

    if (!InstancePtr->Init())
    {
        ServerInstances.pop_back();
        return false;
    }

    return true;
}

bool ServerManager::StopServer(const std::string& ServerId, bool Permanent)
{
    std::scoped_lock lock(m_mutex);

    Log("Stopping server %s ...", ServerId.c_str());

    bool Success = true;

    for (auto iter = ServerInstances.begin(); iter != ServerInstances.end(); /* empty */)
    {
        Server* Instance = (*iter).get();
        if (Instance->GetId() == ServerId)
        {
            Success |= Instance->Term();

            if (Permanent)
            {
                // Delete the folder the server configuration is stored in.
                std::filesystem::path path = Instance->GetSavedPath();
                std::filesystem::remove_all(path);
            }

            iter = ServerInstances.erase(iter);
        }
        else
        {
            iter++;
        }
    }

    return Success;
}

int ServerManager::GetFreeGamePort()
{
    std::scoped_lock lock(m_mutex);

    Server* Default = GetDefaultServer();
    int Port = Default->GetConfig().StartGameServerPortRange;

    while (true)
    {
        bool InUse = false;

        for (auto& Server : ServerInstances)
        {
            if (Server->GetConfig().GameServerPort == Port ||
                Server->GetConfig().AuthServerPort == Port ||
                Server->GetConfig().LoginServerPort == Port ||
                Server->GetConfig().WebUIServerPort == Port)
            {
                InUse = true;
                break;
            }
        }

        if (!InUse)
        {
            break;
        }
        else
        {
            Port++;
        }
    }

    return Port;
}

Server* ServerManager::GetDefaultServer()
{
    std::scoped_lock lock(m_mutex);

    for (auto& Server : ServerInstances)
    {
        if (Server->IsDefaultServer())
        {
            return Server.get();
        }
    }

    return nullptr;
}

Server* ServerManager::FindServer(const std::string& Id)
{
    std::scoped_lock lock(m_mutex);

    for (auto& Server : ServerInstances)
    {
        if (Server->GetId() == Id)
        {
            return Server.get();
        }
    }

    return nullptr;
}

void ServerManager::PruneOldServers()
{
    std::scoped_lock lock(m_mutex);

    std::vector<std::string> IdsToPrune;

    for (auto& Server : ServerInstances)
    {
        if (!Server->IsDefaultServer())
        {
            if (Server->GetSecondsSinceLastActivity() > BuildConfig::SERVER_TIMEOUT)
            {
                IdsToPrune.push_back(Server->GetId());   
            }
        }
    }

    for (const std::string& IdsToPrune : IdsToPrune)
    {
        StopServer(IdsToPrune, true);
    }
}