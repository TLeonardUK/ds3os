// Dark Souls 3 - Open Server

#pragma once

#include <string>

// Configuration saved and loaded at runtime by the server from a configuration file.
class RuntimeConfig
{
public:

    // Name used in the server import file.
    std::string ServerName = "Dark Souls 3 Server";

    // Description used in the server import file.
    std::string ServerDescription = "A custom Dark Souls 3 server.";

    // Hostname of the server that should be used for connecting.
    std::string ServerHostname = "127.0.0.1";

    // IP of the server that should be used for connecting. 
    // TODO: If none is supplied then the IP will be the resolved IP of ServerHostname.
    // TODO: Private ip of server will be returned rather than this if client is on same subnet.
    std::string ServerIP = "127.0.0.1";

    // Network port the login server listens for connections on.
    int LoginServerPort = 50050;

    // Network port the authentication server listens for connections on.
    int AuthServerPort = 50000;

    // Network port the game server listens for connections on.
    int GameServerPort = 50010;

};