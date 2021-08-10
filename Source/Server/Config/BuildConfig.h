// Dark Souls 3 - Open Server

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

};