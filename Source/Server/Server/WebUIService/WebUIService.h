/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Server/Service.h"

#include "ThirdParty/civetweb/include/civetweb.h"
#include "ThirdParty/civetweb/include/CivetServer.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>

class Server;
class WebUIHandler;

// The webui service hosts a very simple webserver that can be used to 
// monitor the status of the server.

class WebUIService 
    : public Service
{
public:
    WebUIService(Server* OwningServer);
    virtual ~WebUIService();

    virtual bool Init() override;
    virtual bool Term() override;
    virtual void Poll() override;

    virtual std::string GetName() override;

    Server* GetServer() { return ServerInstance; }
    std::shared_ptr<CivetServer> GetWebServer() { return WebServer; }

public:
    bool CheckAuthToken(const std::string& Token);
    std::string AddAuthToken();
    bool IsAuthenticated(const mg_connection* Connection);
    void ClearExpiredTokens();

    void GatherData();

private:
    struct AuthToken
    {
        std::string Token;
        double ExpireTime;
    };

    Server* ServerInstance;

    std::shared_ptr<CivetServer> WebServer;

    std::vector<std::shared_ptr<WebUIHandler>> Handlers;

    std::recursive_mutex StateMutex;
    std::unordered_map<std::string, AuthToken> AuthTokens;

};