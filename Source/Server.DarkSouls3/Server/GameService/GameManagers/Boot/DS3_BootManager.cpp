/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Boot/DS3_BootManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/GameService/GameService.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/DS3_Frpg2ReliableUdpMessage.h"
#include "Server.DarkSouls3/Protobuf/DS3_Protobufs.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"

#include "Shared/Core/Network/NetConnection.h"

DS3_BootManager::DS3_BootManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

MessageHandleResult DS3_BootManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestWaitForUserLogin))
    {
        return Handle_RequestWaitForUserLogin(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestGetAnnounceMessageList))
    {
        return Handle_RequestGetAnnounceMessageList(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult DS3_BootManager::Handle_RequestWaitForUserLogin(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& State = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestWaitForUserLogin* Request = (DS3_Frpg2RequestMessage::RequestWaitForUserLogin*)Message.Protobuf.get();
    
    std::string SteamId = Request->steam_id();

#ifdef _DEBUG

    // In debug if a stream id is already signed in we make a duplicate player profile, this allows us to have multiple of the 
    // same steam id connected without screwin up logic everywhere else.
    std::shared_ptr<GameService> Service = ServerInstance->GetService<GameService>();
    std::string BaseSteamId = SteamId;
    size_t ProfileInstance = 1;
    while (Service->FindClientBySteamId(SteamId) != nullptr)
    {
        SteamId = StringFormat("%s_%i", BaseSteamId.c_str(), ProfileInstance++);
    }

#endif

    State.SetSteamId(SteamId);

    // Resolve steam id to player id. If no player recorded with it, create a new one.
    uint32_t NewPlayerId = 0;
    if (!ServerInstance->GetDatabase().FindOrCreatePlayer(State.GetSteamId(), NewPlayerId))
    {
        WarningS(Client->GetName().c_str(), "Failed to find or create player with steam id '%s' in database.", State.GetSteamId().c_str());
        return MessageHandleResult::Error;
    }

    State.SetPlayerId(NewPlayerId);

    LogS(Client->GetName().c_str(), "Steam id '%s' has logged in as player %i.", State.GetSteamId().c_str(), State.GetPlayerId());
    LogS(Client->GetName().c_str(), "Renaming connection to '%s'.", State.GetSteamId().c_str());

    Client->Connection->Rename(State.GetSteamId());

    // Send back response with our new player id.
    DS3_Frpg2RequestMessage::RequestWaitForUserLoginResponse Response;
    Response.set_steam_id(State.GetSteamId());
    Response.set_player_id(State.GetPlayerId()); 
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestWaitForUserLoginResponse response.");
        return MessageHandleResult::Error;
    }

    // Configure what data we want the client to upload in thier status messages.
    static std::vector<uint32_t> PlayerStatusFieldMask = { 
        
        // This is what the official server normally asks for. But this includes flags the 
        // client doesn't actually recognize, I'm assuming those are ones only available in debug builds or
        // are no longer used.

        /*2, 100, 102, 105, 107, 108, 109, 110, 111, 112, 114, 116, 117, 118, 119, 120, 121, 122, 
        123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 139, 140, 141, 142, 
        143, 144, 304, 310, 311, 312, 313, 314, 400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 
        410, 411, 600, 601, 602, 603, 604, 605, 700, 701, 702, 703, 704, 705, 706, 707, 708, 709, 
        710, 711, 712, 713, 714, 715, 716, 717, 718, 719, 720, 721, 722, 800, 803, 804, 805, 806, 
        807, 900, 901, 902, 1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 
        1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019, 1020, 1021, 1022, 1023, 1024, 1025, 1026, 
        1027, 1028, 1029, 1030, 1031, 1032, 1033, 1034, 1035, 1036, 1037, 1038, 1039, 1040, 1041, 
        1042, 1043, 1044, 1045, 1046, 1047, 1048, 1049, 1050, 1051, 1052, 1053, 1054, 1055, 1056, 
        1057, 1058, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 
        1059, 1059, 1059*/

        // This is all the flags the game understands. If anyone is incredibly bored it might be worth
        // documenting what fields each of these map to.

        0x2,  0x0, 0x64, 0x24, 0x66, 0x10, 0x69, 0x5, 0x6b, 0x3, 0x6c, 0x2, 0x6d, 0x6, 0x6e, 
        0x12, 0x6f, 0x4, 0x70, 0x14, 0x72, 0x1, 0x74, 0x7, 0x75, 0x8, 0x76, 0x9, 0x77, 0xf, 
        0x78, 0xc, 0x79, 0x11, 0x7a, 0xd, 0x7b, 0xe, 0x7c, 0xf, 0x7d, 0x13, 0x7e, 0x15, 0x7f, 
        0x16, 0x80, 0x17, 0x81, 0x18, 0x82, 0x19, 0x83, 0x1a, 0x84, 0x1b, 0x85, 0x1c, 0x86, 
        0x1d, 0x87, 0x1e, 0x88, 0x1f, 0x8b, 0x20, 0x8c, 0xb, 0x8d, 0x21, 0x8e, 0x22, 0x8f, 
        0x23, 0x90, 0x25, 0x130, 0x90, 0x136, 0x91, 0x137, 0x92, 0x138, 0x93, 0x139, 0x94, 
        0x13a, 0x95, 0x190, 0x26, 0x191, 0x27, 0x192, 0x28, 0x193, 0x29, 0x194, 0x2a, 0x195, 
        0x2b, 0x196, 0x2c, 0x197, 0x2d, 0x198, 0x2e, 0x199, 0x2f, 0x19a, 0x30, 0x19b, 0x31, 
        0x384, 0x51, 0x258, 0x32, 0x259, 0x33, 0x25a, 0x34, 0x25b, 0x35, 0x25c, 0x36, 0x25d, 
        0x37, 0x2bC, 0x38, 0x2bd, 0x39, 0x2be, 0x3a, 0x2bf, 0x3b, 0x2c0, 0x3c, 0x2c1, 0x3d, 
        0x2c2, 0x3e, 0x2c3, 0x3f, 0x2c4, 0x40, 0x2c5, 0x41, 0x2c6, 0x42, 0x2c7, 0x43, 0x2c8, 
        0x44, 0x2c9, 0x45, 0x2ca, 0x46, 0x2cb, 0x47, 0x2cc, 0x48, 0x2cd, 0x49, 0x2ce, 0x4a, 
        0x2cf, 0x96, 0x2d0, 0x97, 0x2d1, 0x98, 0x2d2, 0x99, 0x320, 0x4b, 0x323, 0x4c, 0x324, 
        0x4d, 0x325, 0x4e, 0x326, 0x4f, 0x327, 0x50, 0x385, 0x52, 0x386, 0x53, 0x3e8, 0x54, 
        0x3e9, 0x55, 0x3ea, 0x56, 0x3eb, 0x57, 0x3ec, 0x58, 0x3ed, 0x59, 0x3ee, 0x5a, 0x3ef, 
        0x5b, 0x3f0, 0x5c, 0x3f1, 0x5d, 0x3f2, 0x5e, 0x3f3, 0x5f, 0x3f4, 0x60, 0x3f5, 0x61, 
        0x3f6, 0x62, 0x3f7, 0x63, 0x3f8, 0x64, 0x3f9, 0x65, 0x3fa, 0x66, 0x3fb, 0x67, 0x3fc, 
        0x68, 0x3fd, 0x69, 0x3fe, 0x6c, 0x3ff, 0x6d, 0x400, 0x6e, 0x401, 0x6f, 0x402, 0x70, 
        0x403, 0x71, 0x404, 0x72, 0x405, 0x73, 0x406, 0x74, 0x407, 0x75, 0x408, 0x76, 0x409, 
        0x77, 0x40a, 0x78, 0x40b, 0x79, 0x40c, 0x7a, 0x40d, 0x7b, 0x40e, 0x7c, 0x40f, 0x7d, 
        0x410, 0x7e, 0x411, 0x7f, 0x412, 0x80, 0x413, 0x81, 0x414, 0x82, 0x415, 0x6a, 0x416, 
        0x6b, 0x417, 0x83, 0x418, 0x84, 0x419, 0x85, 0x41a, 0x86, 0x41b, 0x87, 0x41c, 0x88, 
        0x41d, 0x89, 0x41e, 0x8a, 0x41f, 0x8b, 0x420, 0x8c, 0x421, 0x8d, 0x422, 0x8e, 0x423, 
        0x8f
    };

    DS3_Frpg2RequestMessage::PlayerInfoUploadConfigPushMessage UploadInfoPushMessage;
    UploadInfoPushMessage.set_push_message_id(DS3_Frpg2RequestMessage::PushMessageId::PushID_PlayerInfoUploadConfigPushMessage);
    UploadInfoPushMessage.set_player_character_update_send_delay(ServerInstance->GetConfig().PlayerCharacterUpdateSendDelay);
    UploadInfoPushMessage.set_player_status_send_delay(ServerInstance->GetConfig().PlayerStatusUploadSendDelay);

    DS3_Frpg2RequestMessage::PlayerStatusUploadConfig* UploadInfoPushMessageList = UploadInfoPushMessage.mutable_config();
    for (uint32_t Value : PlayerStatusFieldMask)
    {
        UploadInfoPushMessageList->add_player_data_mask(Value);
    }
    UploadInfoPushMessageList->set_upload_interval(ServerInstance->GetConfig().PlayerStatusUploadInterval);

    if (!Client->MessageStream->Send(&UploadInfoPushMessage))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send UploadInfoPushMessage response.");
        return MessageHandleResult::Error;
    }

    std::string TypeStatisticKey = StringFormat("Player/TotalLogins");
    Database.AddGlobalStatistic(TypeStatisticKey, 1);
    Database.AddPlayerStatistic(TypeStatisticKey, State.GetPlayerId(), 1);

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_BootManager::Handle_RequestGetAnnounceMessageList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    auto& Config = ServerInstance->GetConfig();

    DS3_Frpg2RequestMessage::RequestGetAnnounceMessageList* Request = (DS3_Frpg2RequestMessage::RequestGetAnnounceMessageList*)Message.Protobuf.get();
    Ensure(Request->max_entries() == 100);

    DS3_Frpg2RequestMessage::RequestGetAnnounceMessageListResponse Response;
    DS3_Frpg2RequestMessage::AnnounceMessageDataList* Notices = Response.mutable_notices();
    DS3_Frpg2RequestMessage::AnnounceMessageDataList* Changes = Response.mutable_changes();

    float Score = ServerInstance->GetDatabase().GetAntiCheatPenaltyScore(Client->GetPlayerState().GetSteamId());

    std::vector<RuntimeConfigAnnouncement> Announcements;
    if (ServerInstance->GetDatabase().IsPlayerBanned(Client->GetPlayerState().GetSteamId()))
    {
        RuntimeConfigAnnouncement Announcement;
        Announcement.Header = "BANNED";
        Announcement.Body = Config.BanAnnouncementMessage;
        Announcements.push_back(Announcement);

        Client->Banned = true;
        Client->DisconnectTime = GetSeconds() + 2.0f;
    }
    else if (Config.AntiCheatApplyPenalties && Score > Config.AntiCheatWarningThreshold)
    {
        RuntimeConfigAnnouncement Announcement;
        Announcement.Header = "WARNING";
        Announcement.Body = Config.WarningAnnouncementMessage;
        Announcements.push_back(Announcement);
    }
    else
    {
        Announcements = ServerInstance->GetConfig().Announcements;
    }

    int Index = 0;

    // Temporary warning for the exploit disclosure 
    for (const RuntimeConfigAnnouncement& Announcement : Announcements)
    {
        DS3_Frpg2RequestMessage::AnnounceMessageData* Data = Changes->add_items();
        Data->set_unknown_1(1);
        Data->set_index(Index++);
        Data->set_unknown_2(1);
        Data->set_header(Announcement.Header);
        Data->set_message(Announcement.Body);

        // The datetime isn't displayed anywhere and makes no difference to ordering, so
        // lets just auto generate it :shrugs:
        DS3_Frpg2PlayerData::DateTime* DateTime = Data->mutable_datetime();
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

std::string DS3_BootManager::GetName()
{
    return "Boot";
}
