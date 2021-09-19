/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Boot/BootManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Core/Utils/Logging.h"
#include "Core/Utils/Strings.h"

BootManager::BootManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

MessageHandleResult BootManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestWaitForUserLogin)
    {
        return Handle_RequestWaitForUserLogin(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetAnnounceMessageList)
    {
        return Handle_RequestGetAnnounceMessageList(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult BootManager::Handle_RequestWaitForUserLogin(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& State = Client->GetPlayerState();

    Frpg2RequestMessage::RequestWaitForUserLogin* Request = (Frpg2RequestMessage::RequestWaitForUserLogin*)Message.Protobuf.get();
    State.SteamId = Request->steam_id();

    // Resolve steam id to player id. If no player recorded with it, create a new one.
    if (!ServerInstance->GetDatabase().FindOrCreatePlayer(State.SteamId, State.PlayerId))
    {
        WarningS(Client->GetName().c_str(), "Failed to find or create player with steam id '%s' in database.", State.SteamId.c_str());
        return MessageHandleResult::Error;
    }

    LogS(Client->GetName().c_str(), "Steam id '%s' has logged in as player %i.", State.SteamId.c_str(), State.PlayerId);

    // Send back response with our new player id.
    Frpg2RequestMessage::RequestWaitForUserLoginResponse Response;
    Response.set_steam_id(State.SteamId);
    Response.set_player_id(State.PlayerId); 
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestWaitForUserLoginResponse response.");
        return MessageHandleResult::Error;
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
    UploadInfoPushMessage.set_push_message_id(Frpg2RequestMessage::PushMessageId::PushID_PlayerInfoUploadConfigPushMessage);
    UploadInfoPushMessage.set_unknown_3(600);
    UploadInfoPushMessage.set_unknown_4(300);

    Frpg2RequestMessage::PlayerInfoUploadConfigPushMessageList* UploadInfoPushMessageList = UploadInfoPushMessage.mutable_unknown_2();
    for (uint32_t Value : UploadInfoConfigValues)
    {
        UploadInfoPushMessageList->add_unknown_1(Value);
    }
    UploadInfoPushMessageList->set_unknown_2(5);

    if (!Client->MessageStream->Send(&UploadInfoPushMessage))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send UploadInfoPushMessage response.");
        return MessageHandleResult::Error;
    }

    std::string TypeStatisticKey = StringFormat("Player/TotalLogins");
    Database.AddGlobalStatistic(TypeStatisticKey, 1);
    Database.AddPlayerStatistic(TypeStatisticKey, State.PlayerId, 1);

    return MessageHandleResult::Handled;
}

MessageHandleResult BootManager::Handle_RequestGetAnnounceMessageList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestGetAnnounceMessageList* Request = (Frpg2RequestMessage::RequestGetAnnounceMessageList*)Message.Protobuf.get();
    Ensure(Request->max_entries() == 100);

    Frpg2RequestMessage::RequestGetAnnounceMessageListResponse Response;
    Frpg2RequestMessage::AnnounceMessageDataList* Notices = Response.mutable_notices();
    Frpg2RequestMessage::AnnounceMessageDataList* Changes = Response.mutable_changes();

    int Index = 0;
    for (const RuntimeConfigAnnouncement& Announcement : ServerInstance->GetConfig().Announcements)
    {
        Frpg2RequestMessage::AnnounceMessageData* Data = Changes->add_items();
        Data->set_unknown_1(1);
        Data->set_index(Index++);
        Data->set_unknown_2(1);
        Data->set_header(Announcement.Header);
        Data->set_message(Announcement.Body);

        // The datetime isn't displayed anywhere and makes no difference to ordering, so
        // lets just auto generate it :shrugs:
        Frpg2PlayerData::DateTime* DateTime = Data->mutable_datetime();
        DateTime->set_year(2021);
        DateTime->set_month(1);
        DateTime->set_day(1);
        DateTime->set_hours(0);
        DateTime->set_minutes(0);
        DateTime->set_seconds(0);
        DateTime->set_tzdiff(0);
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestWaitForUserLoginResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string BootManager::GetName()
{
    return "Boot";
}
