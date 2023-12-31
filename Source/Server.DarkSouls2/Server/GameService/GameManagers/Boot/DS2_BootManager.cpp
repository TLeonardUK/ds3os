/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Boot/DS2_BootManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/GameService/GameService.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Server/Streams/DS2_Frpg2ReliableUdpMessage.h"
#include "Server.DarkSouls2/Protobuf/DS2_Protobufs.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"

#include "Shared/Core/Network/NetConnection.h"

DS2_BootManager::DS2_BootManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

MessageHandleResult DS2_BootManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestWaitForUserLogin))
    {
        return Handle_RequestWaitForUserLogin(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestGetAnnounceMessageList))
    {
        return Handle_RequestGetAnnounceMessageList(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult DS2_BootManager::Handle_RequestWaitForUserLogin(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& State = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestWaitForUserLogin* Request = (DS2_Frpg2RequestMessage::RequestWaitForUserLogin*)Message.Protobuf.get();
    
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
    DS2_Frpg2RequestMessage::RequestWaitForUserLoginResponse Response;
    Response.set_steam_id(State.GetSteamId());
    Response.set_player_id(State.GetPlayerId()); 
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestWaitForUserLoginResponse response.");
        return MessageHandleResult::Error;
    }

    // Configure what data we want the client to upload in thier status messages.
    static std::vector<uint32_t> PlayerStatusFieldMask = {         
        0, 1, 2, 3, 4, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 
        112, 113, 114, 115, 200, 201, 202, 203, 204, 205, 206, 300, 301, 302, 303, 
        304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 400, 401, 402, 
        403, 404, 405, 406, 407, 408, 500, 501, 502, 503, 504, 505, 506, 507, 508, 
        509, 510, 511, 512, 513, 514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 
        600, 601, 602, 603, 604, 605, 700, 701, 702, 703, 800, 801, 802, 803, 804, 805
    };

    DS2_Frpg2RequestMessage::PlayerInfoUploadConfigPushMessage UploadInfoPushMessage;
    UploadInfoPushMessage.set_push_message_id(DS2_Frpg2RequestMessage::PushMessageId::PushID_PlayerInfoUploadConfigPushMessage);
    UploadInfoPushMessage.set_player_character_update_send_delay(ServerInstance->GetConfig().PlayerCharacterUpdateSendDelay);
    UploadInfoPushMessage.set_player_status_send_delay(ServerInstance->GetConfig().PlayerStatusUploadSendDelay);

    DS2_Frpg2RequestMessage::PlayerStatusUploadConfig* UploadInfoPushMessageList = UploadInfoPushMessage.mutable_config();
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

MessageHandleResult DS2_BootManager::Handle_RequestGetAnnounceMessageList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    auto& Config = ServerInstance->GetConfig();

    DS2_Frpg2RequestMessage::RequestGetAnnounceMessageList* Request = (DS2_Frpg2RequestMessage::RequestGetAnnounceMessageList*)Message.Protobuf.get();
    Ensure(Request->max_entries() == 10);

    DS2_Frpg2RequestMessage::RequestGetAnnounceMessageListResponse Response;
    DS2_Frpg2RequestMessage::AnnounceMessageDataList* Notices = Response.mutable_notices();
    DS2_Frpg2RequestMessage::AnnounceMessageDataList* Changes = Response.mutable_changes();

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
        DS2_Frpg2RequestMessage::AnnounceMessageData* Data = Changes->add_items();
        Data->set_unknown_1(1);
        Data->set_index(Index++);
        Data->set_unknown_2(1);
        Data->set_header(Announcement.Header);
        Data->set_message(Announcement.Body);

        // The datetime isn't displayed anywhere and makes no difference to ordering, so
        // lets just auto generate it :shrugs:
        DS2_Frpg2PlayerData::DateTime* DateTime = Data->mutable_datetime();
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

std::string DS2_BootManager::GetName()
{
    return "Boot";
}
