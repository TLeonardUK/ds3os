// Dark Souls 3 - Open Server

#include "Server/GameService/GameClient.h"
#include "Server/GameService/GameService.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"

#include "Server/Server.h"

#include "Platform/Platform.h"

#include "Core/Utils/Logging.h"
#include "Core/Network/NetConnection.h"

#include "Config/BuildConfig.h"
#include "Config/RuntimeConfig.h"

#include "Protobuf/Frpg2RequestMessage.pb.h"

GameClient::GameClient(GameService* OwningService, std::shared_ptr<NetConnection> InConnection, const std::vector<uint8_t>& CwcKey, uint64_t InAuthToken)
    : Service(OwningService)
    , Connection(InConnection)
    , AuthToken(InAuthToken)
{
    LastMessageRecievedTime = GetSeconds();

    MessageStream = std::make_shared<Frpg2ReliableUdpMessageStream>(InConnection, CwcKey, AuthToken);
}

bool GameClient::Poll()
{
    // Has this client timed out?
    double TimeSinceLastMessage = GetSeconds() - LastMessageRecievedTime;
    if (TimeSinceLastMessage >= BuildConfig::CLIENT_TIMEOUT)
    {
        Warning("[%s] Client timed out.", GetName().c_str());
        return true;
    }

    // Client disconnected.
    if (Connection->Pump())
    {
        Warning("[%s] Disconnecting client as connection was in an error state.", GetName().c_str());
        return true;
    }
    if (!Connection->IsConnected())
    {
        Warning("[%s] Client disconnected.", GetName().c_str());
        return true;
    }

    // Pump the message stream and handle any messages that come in.
    if (MessageStream->Pump())
    {
        Warning("[%s] Disconnecting client as message stream closed.", GetName().c_str());
        return true;
    }

    // Process all packets.
    Frpg2ReliableUdpMessage Message;
    while (MessageStream->Recieve(&Message))
    {
        if (HandleMessage(Message))
        {
            Warning("[%s] Disconnecting client as failed to handle message.", GetName().c_str());
            return true;
        }

        // TODO: Find a better way to do this that doesn't break our abstraction.
        MessageStream->HandledPacket(Message.AckSequenceIndex);
    }

    // Update lat recieved time.
    LastMessageRecievedTime = MessageStream->GetLastActivityTime();

    // Keep authentication token alive while client is..
    Service->RefreshAuthToken(AuthToken);

    return false;
}

bool GameClient::HandleMessage(const Frpg2ReliableUdpMessage& Message)
{
    // TODO: Do have a proper register/dispatch system for this that will handle responses/ack's.
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestWaitForUserLogin)
    {
        Frpg2RequestMessage::RequestWaitForUserLogin* Request = (Frpg2RequestMessage::RequestWaitForUserLogin*)Message.Protobuf.get();
        SteamId = Request->steam_id();

        Frpg2RequestMessage::RequestWaitForUserLoginResponse Response;
        Response.set_steam_id(SteamId);
        Response.set_unknown_1(6872049); // TODO: Find out what this value is meant to be.

        if (!MessageStream->Send(&Response, &Message))
        {
            Warning("[%s] Disconnecting client as failed to send RequestWaitForUserLoginResponse response.", GetName().c_str());
            return true;
        }
        // This happens right after RequestWaitForUserLoginResponse.
        // I'm guessing this is configuring how often the 
        // player writes their info the server in different situations.
        // TODO: Figure out what these values actually do.
        static std::vector<uint32_t> UploadInfoConfigValues = { 
            2, 100, 102, 105, 107, 108, 109, 110, 111, 112, 114, 116, 117, 118, 119, 120, 121, 122, 
            123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 139, 140, 141, 142, 
            143, 144, 304, 310, 311, 312, 313, 314, 400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 
            410, 411, 600, 601, 602, 603, 604, 605, 700, 701, 702, 703, 704, 705, 706, 707, 708, 709, 
            710, 711, 712, 713, 714, 715, 716, 717, 718, 719, 720, 721, 722, 800, 803, 804, 805, 806, 
            807, 900, 901, 902, 1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 
            1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019, 1020, 1021, 1022, 1023, 1024, 1025, 1026, 
            1027, 1028, 1029, 1030, 1031, 1032, 1033, 1034, 1035, 1036, 1037, 1038, 1039, 1040, 1041, 
            1042, 1043, 1044, 1045, 1046, 1047, 1048, 1049, 1050, 1051, 1052, 1053, 1054, 1055, 1056, 
            1057, 1058, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 
            1059, 1059, 1059
        };

        Frpg2RequestMessage::PlayerInfoUploadConfigPushMessage UploadInfoPushMessage;
        UploadInfoPushMessage.set_unknown_1(908);
        UploadInfoPushMessage.set_unknown_3(600);
        UploadInfoPushMessage.set_unknown_4(300);

        Frpg2RequestMessage::PlayerInfoUploadConfigPushMessageList* UploadInfoPushMessageList = UploadInfoPushMessage.mutable_unknown_2();
        for (uint32_t Value : UploadInfoConfigValues)
        {
            UploadInfoPushMessageList->add_unknown_1(Value);
        }
        UploadInfoPushMessageList->set_unknown_2(5);

        if (!MessageStream->Send(&UploadInfoPushMessage))
        {
            Warning("[%s] Disconnecting client as failed to send UploadInfoPushMessage response.", GetName().c_str());
            return true;
        }

        return false;
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetAnnounceMessageList)
    {
        Frpg2RequestMessage::RequestGetAnnounceMessageList* Request = (Frpg2RequestMessage::RequestGetAnnounceMessageList*)Message.Protobuf.get();

        Frpg2RequestMessage::RequestGetAnnounceMessageListResponse Response;

        Frpg2RequestMessage::AnnounceMessageDataList* Notices = Response.mutable_notices();

        Frpg2RequestMessage::AnnounceMessageDataList* Changes = Response.mutable_changes();

        for (int i = 0; i < 200; i++)
        {
            Frpg2RequestMessage::AnnounceMessageData* Data = Changes->add_items();
            Data->set_unknown_1(20);
            Data->set_order(1);
            Data->set_unknown_2(1);
            Data->set_header("Hello World");
            Data->set_message("Connected to Dark Souls 3 Open server.\nhttp://github.com/tleonarduk/ds3os");

            Frpg2PlayerData::DateTime* DateTime = Data->mutable_datetime();
            DateTime->set_year(2021);
            DateTime->set_month(8);
            DateTime->set_day(10);
            DateTime->set_hours(18);
            DateTime->set_minutes(30);
            DateTime->set_seconds(0);
            DateTime->set_tzdiff(1);
        }

        if (!MessageStream->Send(&Response, &Message))
        {
            Warning("[%s] Disconnecting client as failed to send RequestWaitForUserLoginResponse response.", GetName().c_str());
            return true;
        }

        return false;
    }

    return true;
}

std::string GameClient::GetName()
{
    return Connection->GetName();
}