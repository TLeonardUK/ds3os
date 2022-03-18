/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <vector>
#include <string>

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
    inline static const int MAX_PACKET_LENGTH = 8192;

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

    // CVE-2022-24125 (RequestSendMessageToPlayers abuse) and CVE-2022-24126 (NRSessionSearchResult RCE) serverside fixes.
    // These should only be disabled on a debug build for testing purposes.
#if defined(_DEBUG)
    constexpr inline static const bool SEND_MESSAGE_TO_PLAYERS_SANITY_CHECKS = false;
    constexpr inline static const bool NRSSR_SANITY_CHECKS = false;
#else
    constexpr inline static const bool SEND_MESSAGE_TO_PLAYERS_SANITY_CHECKS = true;
    constexpr inline static const bool NRSSR_SANITY_CHECKS = true;
#endif

    // Dumps a diasssembly of each message to the output.
    constexpr inline static const bool DISASSEMBLE_RECIEVED_MESSAGES = false;

    // Dumps a diasssembly of each message to the output.
    constexpr inline static const bool DISASSEMBLE_SENT_MESSAGES = false;

    // Prints reliable udp packet stream.
    constexpr inline static const bool EMIT_RELIABLE_UDP_PACKET_STREAM = false;

    // Writes messages that fail to deserialize to the local directory.
    constexpr inline static const bool DUMP_FAILED_DISASSEMBLED_PACKETS = true;

    // How many seconds of inactivity before a webui authentication token expires.
    inline static const double WEBUI_AUTH_TIMEOUT = 60.0 * 60.0;

    // If enabled we will store per-player stats in the database. Be warned this bloats the DB a -lot-
    // if we have many players.
    inline static const bool STORE_PER_PLAYER_STATISTICS = false;

    // Allows you to emulate CPU spikes, to track down disconnect issues.
    inline static const bool EMULATE_SPIKES = false;

    inline static const double SPIKE_INTERVAL_MIN = 1000.0 * 10.0;
    inline static const double SPIKE_INTERVAL_MAX = 1000.0 * 30.0;

    inline static const double SPIKE_LENGTH_MIN = 1000.0 * 5.0;
    inline static const double SPIKE_LENGTH_MAX = 1000.0 * 20.0;

    // Emulates dropped udp packets.
    inline static const bool EMULATE_DROPPED_PACKETS = false;

    inline static const double DROP_PACKET_PROBABILITY = 0.1f;

    // Emulates latency.
    inline static const bool EMULATE_LATENCY = false;

    inline static const double LATENCY_MINIMUM = 300.0f;
    inline static const double LATENCY_VARIANCE = 10.0f;

    // When running as a client emulator this is how many clients to spawn.
    inline static const size_t CLIENT_EMULATOR_COUNT = 128;

    constexpr inline static const bool AUTH_ENABLED = true;

    // How long with now data gathering requests before we timeout and stop
    // gathering data for perf.
    inline static const double WEBUI_GATHER_DATA_TIMEOUT = 10.0f; 

    // Minimum time between each data gather for the webui.
    inline static const double WEBUI_GATHER_DATA_MIN_INTERVAL = 3.0f;

    // Enables or disables player/global statistics gathering. This adds a good
    // bit of database overhead, so don't use it unless you are actually using
    // the statistics its generating.
    constexpr inline static const bool DATABASE_STAT_GATHERING_ENABLED = false;

};