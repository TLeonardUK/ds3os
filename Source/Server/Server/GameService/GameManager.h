/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <string>

class GameClient;
struct Frpg2ReliableUdpMessage;

 // Base class for all managers that handle specific areas
 // of functionality for the game service - matchmaking, bloodstains 
 // and alike.

class GameManager
{
public:
    virtual ~GameManager() {};

    virtual bool Init() { return true; };
    virtual bool Term() { return true; };
    virtual void Poll() { };

    // Called when a client disconnects.
    virtual void OnClientDisconnected(GameClient* Client) { };

    // Called when a client connects.
    virtual void OnClientConnected(GameClient* Client) { };

    // Called when a game client recieves a message.
    // Returns true if an error occured and the client should be disconnected.
    virtual bool OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message) { return false; }

    // Returns a general descriptive name of the manager for logging.
    virtual std::string GetName() = 0;

};