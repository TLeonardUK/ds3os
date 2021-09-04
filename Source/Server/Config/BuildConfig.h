/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

// Abstract class that just holds various build-time 
// configuration variables. Might be worth dumping this
// into a json file at some point in future so it can 
// be changed.
class BuildConfig
{
public:
    
    BuildConfig() = delete;

    // How many seconds without messages causes a client to timeout.
    inline static const double CLIENT_TIMEOUT = 60.0;

    // How many seconds without refresh before an authentication ticket expires.
    inline static const double AUTH_TICKET_TIMEOUT = 30.0;

    // Maximum length of a packet in an Frpg2PacketStream.
    inline static const int MAX_PACKET_LENGTH = 2048;

    // Maximum backlog of data in a packet streams send queue. Sending
    // packets beyond this will result in disconnect.
    inline static const int MAX_SEND_QUEUE_SIZE = 256 * 1024;

    // What application version we support (this is the app version shown on the menu without the dot and -1).
    // So 1.15 = 114
    inline static const int APP_VERSION = 114;

    // If true clients are disconnected if we are unable to handle the message they send.
    // Be careful with this, if we don't reply to some messages the client will deadlock.
#if defined(_DEBUG)
    inline static const bool DISCONNECT_ON_UNHANDLED_MESSAGE = false;
#else
    inline static const bool DISCONNECT_ON_UNHANDLED_MESSAGE = true;
#endif

};