/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Logging/DS3_LoggingManager.h"
#include "Server/GameService/GameService.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/DS3_Frpg2ReliableUdpMessage.h"
#include "Server/GameService/Utils/DS3_GameIds.h"
#include "Server.DarkSouls3/Protobuf/DS3_Protobufs.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"
#include "Server/Database/ServerDatabase.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"

DS3_LoggingManager::DS3_LoggingManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

MessageHandleResult DS3_LoggingManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestNotifyProtoBufLog))
    {
        return Handle_RequestNotifyProtoBufLog(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestNotifyKillEnemy))
    {
        return Handle_RequestNotifyKillEnemy(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestNotifyDisconnectSession))
    {
        return Handle_RequestNotifyDisconnectSession(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestNotifyRegisterCharacter))
    {
        return Handle_RequestNotifyRegisterCharacter(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestNotifyDie))
    {
        return Handle_RequestNotifyDie(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestNotifyKillBoss))
    {
        return Handle_RequestNotifyKillBoss(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestNotifyJoinMultiplay))
    {
        return Handle_RequestNotifyJoinMultiplay(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestNotifyLeaveMultiplay))
    {
        return Handle_RequestNotifyLeaveMultiplay(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestNotifySummonSignResult))
    {
        return Handle_RequestNotifySummonSignResult(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestNotifyCreateSignResult))
    {
        return Handle_RequestNotifyCreateSignResult(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestNotifyBreakInResult))
    {
        return Handle_RequestNotifyBreakInResult(Client, Message);
    }
        
    return MessageHandleResult::Unhandled;
}

MessageHandleResult DS3_LoggingManager::Handle_RequestNotifyProtoBufLog(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestNotifyProtoBufLog* Request = (DS3_Frpg2RequestMessage::RequestNotifyProtoBufLog*)Message.Protobuf.get();

    switch (Request->type())
    {
        case DS3_Frpg2RequestMessage::LogType::UseMagicLog:         Handle_UseMagicLog(Client, Request);            break;
        case DS3_Frpg2RequestMessage::LogType::ActGestureLog:       Handle_ActGestureLog(Client, Request);          break;
        case DS3_Frpg2RequestMessage::LogType::UseItemLog:          Handle_UseItemLog(Client, Request);             break;
        case DS3_Frpg2RequestMessage::LogType::PurchaseItemLog:     Handle_PurchaseItemLog(Client, Request);        break;
        case DS3_Frpg2RequestMessage::LogType::GetItemLog:          Handle_GetItemLog(Client, Request);             break;
        case DS3_Frpg2RequestMessage::LogType::DropItemLog:         Handle_DropItemLog(Client, Request);            break;
        case DS3_Frpg2RequestMessage::LogType::LeaveItemLog:        Handle_LeaveItemLog(Client, Request);           break;
        case DS3_Frpg2RequestMessage::LogType::SaleItemLog:         Handle_SaleItemLog(Client, Request);            break;
        case DS3_Frpg2RequestMessage::LogType::StrengthenWeaponLog: Handle_StrengthenWeaponLog(Client, Request);    break;
        case DS3_Frpg2RequestMessage::LogType::VisitResultLog:      Handle_VisitResultLog(Client, Request);         break;

        // There are other log types, but none we particularly care about handling, so just ignore them.
    }

    DS3_Frpg2RequestMessage::EmptyResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send EmptyResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

void DS3_LoggingManager::Handle_UseMagicLog(GameClient* Client, DS3_Frpg2RequestMessage::RequestNotifyProtoBufLog* Request)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_FpdLogMessage::UseMagicLog Log;
    if (!Log.ParseFromArray(Request->data().data(), (int)Request->data().size()))
    {
        WarningS(Client->GetName().c_str(), "Failed to parse UseMagicLog.");
        return;
    }

    uint32_t TotalCount = 0;
    for (int i = 0; i < Log.use_magic_info_list_size(); i++)
    {
        const DS3_FpdLogMessage::UseMagicLog_Use_magic_info_list& Item = Log.use_magic_info_list(i);

        std::string StatisticKey = StringFormat("Magic/TotalUsed/Id=%u", Item.spell_id());
        Database.AddGlobalStatistic(StatisticKey, Item.count());
        Database.AddPlayerStatistic(StatisticKey, Player.GetPlayerId(), Item.count());

        TotalCount += Item.count();
    }

    std::string TotalStatisticKey = StringFormat("Magic/TotalUsed");
    Database.AddGlobalStatistic(TotalStatisticKey, TotalCount);
    Database.AddPlayerStatistic(TotalStatisticKey, Player.GetPlayerId(), TotalCount);
}

void DS3_LoggingManager::Handle_ActGestureLog(GameClient* Client, DS3_Frpg2RequestMessage::RequestNotifyProtoBufLog* Request)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_FpdLogMessage::ActGestureLog Log;
    if (!Log.ParseFromArray(Request->data().data(), (int)Request->data().size()))
    {
        WarningS(Client->GetName().c_str(), "Failed to parse ActGestureLog.");
        return;
    }

    uint32_t TotalCount = 0;
    for (int i = 0; i < Log.use_gesture_info_list_size(); i++)
    {
        const DS3_FpdLogMessage::ActGestureLog_Use_gesture_info_list& Item = Log.use_gesture_info_list(i);

        std::string StatisticKey = StringFormat("Gesture/TotalUsed/Id=%u", Item.guesture_id());
        Database.AddGlobalStatistic(StatisticKey, Item.count());
        Database.AddPlayerStatistic(StatisticKey, Player.GetPlayerId(), Item.count());

        TotalCount += Item.count();
    }

    std::string TotalStatisticKey = StringFormat("Gesture/TotalUsed");
    Database.AddGlobalStatistic(TotalStatisticKey, TotalCount);
    Database.AddPlayerStatistic(TotalStatisticKey, Player.GetPlayerId(), TotalCount);
}

void DS3_LoggingManager::Handle_UseItemLog(GameClient* Client, DS3_Frpg2RequestMessage::RequestNotifyProtoBufLog* Request)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_FpdLogMessage::UseItemLog Log;
    if (!Log.ParseFromArray(Request->data().data(), (int)Request->data().size()))
    {
        WarningS(Client->GetName().c_str(), "Failed to parse UseItemLog.");
        return;
    }

    uint32_t TotalCount = 0;
    for (int i = 0; i < Log.use_item_info_list_size(); i++)
    {
        const DS3_FpdLogMessage::UseItemLog_Use_item_info_list& Item = Log.use_item_info_list(i);

        std::string StatisticKey = StringFormat("Item/TotalUsed/Id=%u", Item.item_id());
        Database.AddGlobalStatistic(StatisticKey, Item.count());
        Database.AddPlayerStatistic(StatisticKey, Player.GetPlayerId(), Item.count());

        TotalCount += Item.count();
    }

    std::string TotalStatisticKey = StringFormat("Item/TotalUsed");
    Database.AddGlobalStatistic(TotalStatisticKey, TotalCount);
    Database.AddPlayerStatistic(TotalStatisticKey, Player.GetPlayerId(), TotalCount);
}

void DS3_LoggingManager::Handle_PurchaseItemLog(GameClient* Client, DS3_Frpg2RequestMessage::RequestNotifyProtoBufLog* Request)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_FpdLogMessage::PurchaseItemLog Log;
    if (!Log.ParseFromArray(Request->data().data(), (int)Request->data().size()))
    {
        WarningS(Client->GetName().c_str(), "Failed to parse PurchaseItemLog.");
        return;
    }

    uint32_t TotalCount = 0;
    for (int i = 0; i < Log.purchase_item_info_list_size(); i++)
    {
        const DS3_FpdLogMessage::PurchaseItemLog_Purchase_item_info_list& Item = Log.purchase_item_info_list(i);

        std::string StatisticKey = StringFormat("Item/TotalPurchased/Id=%u", Item.item_id());
        Database.AddGlobalStatistic(StatisticKey, Item.count());
        Database.AddPlayerStatistic(StatisticKey, Player.GetPlayerId(), Item.count());

        TotalCount += Item.count();
    }

    std::string TotalStatisticKey = StringFormat("Item/TotalPurchased");
    Database.AddGlobalStatistic(TotalStatisticKey, TotalCount);
    Database.AddPlayerStatistic(TotalStatisticKey, Player.GetPlayerId(), TotalCount);
}

void DS3_LoggingManager::Handle_GetItemLog(GameClient* Client, DS3_Frpg2RequestMessage::RequestNotifyProtoBufLog* Request)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_FpdLogMessage::GetItemLog Log;
    if (!Log.ParseFromArray(Request->data().data(), (int)Request->data().size()))
    {
        WarningS(Client->GetName().c_str(), "Failed to parse GetItemLog.");
        return;
    }

    uint32_t TotalCount = 0;
    for (int i = 0; i < Log.get_item_info_list_size(); i++)
    {
        const DS3_FpdLogMessage::GetItemLog_Get_item_info_list& Item = Log.get_item_info_list(i);

        std::string StatisticKey = StringFormat("Item/TotalRecieved/Id=%u", Item.item_id());
        Database.AddGlobalStatistic(StatisticKey, Item.count());
        Database.AddPlayerStatistic(StatisticKey, Player.GetPlayerId(), Item.count());

        TotalCount += Item.count();
    }

    std::string TotalStatisticKey = StringFormat("Item/TotalRecieved");
    Database.AddGlobalStatistic(TotalStatisticKey, TotalCount);
    Database.AddPlayerStatistic(TotalStatisticKey, Player.GetPlayerId(), TotalCount);
}

void DS3_LoggingManager::Handle_DropItemLog(GameClient* Client, DS3_Frpg2RequestMessage::RequestNotifyProtoBufLog* Request)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_FpdLogMessage::DropItemLog Log;
    if (!Log.ParseFromArray(Request->data().data(), (int)Request->data().size()))
    {
        WarningS(Client->GetName().c_str(), "Failed to parse DropItemLog.");
        return;
    }

    uint32_t TotalCount = 0;
    for (int i = 0; i < Log.throw_away_item_list_size(); i++)
    {
        const DS3_FpdLogMessage::DropItemLog_Throw_away_item_list& Item = Log.throw_away_item_list(i);

        std::string StatisticKey = StringFormat("Item/TotalDropped/Id=%u", Item.item_id());
        Database.AddGlobalStatistic(StatisticKey, Item.count());
        Database.AddPlayerStatistic(StatisticKey, Player.GetPlayerId(), Item.count());

        TotalCount += Item.count();
    }

    std::string TotalStatisticKey = StringFormat("Item/TotalDropped");
    Database.AddGlobalStatistic(TotalStatisticKey, TotalCount);
    Database.AddPlayerStatistic(TotalStatisticKey, Player.GetPlayerId(), TotalCount);
}

void DS3_LoggingManager::Handle_LeaveItemLog(GameClient* Client, DS3_Frpg2RequestMessage::RequestNotifyProtoBufLog* Request)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_FpdLogMessage::LeaveItemLog Log;
    if (!Log.ParseFromArray(Request->data().data(), (int)Request->data().size()))
    {
        WarningS(Client->GetName().c_str(), "Failed to parse LeaveItemLog.");
        return;
    }

    uint32_t TotalCount = 0;
    for (int i = 0; i < Log.set_item_info_list_size(); i++)
    {
        const DS3_FpdLogMessage::LeaveItemLog_Set_item_info_list& Item = Log.set_item_info_list(i);

        std::string StatisticKey = StringFormat("Item/TotalLeft/Id=%u", Item.item_id());
        Database.AddGlobalStatistic(StatisticKey, Item.count());
        Database.AddPlayerStatistic(StatisticKey, Player.GetPlayerId(), Item.count());

        TotalCount += Item.count();
    }

    std::string TotalStatisticKey = StringFormat("Item/TotalLeft");
    Database.AddGlobalStatistic(TotalStatisticKey, TotalCount);
    Database.AddPlayerStatistic(TotalStatisticKey, Player.GetPlayerId(), TotalCount);
}

void DS3_LoggingManager::Handle_SaleItemLog(GameClient* Client, DS3_Frpg2RequestMessage::RequestNotifyProtoBufLog* Request)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_FpdLogMessage::SaleItemLog Log;
    if (!Log.ParseFromArray(Request->data().data(), (int)Request->data().size()))
    {
        WarningS(Client->GetName().c_str(), "Failed to parse SaleItemLog.");
        return;
    }

    uint32_t TotalCount = 0;
    for (int i = 0; i < Log.sale_item_info_list_size(); i++)
    {
        const DS3_FpdLogMessage::SaleItemLog_Sale_item_info_list& Item = Log.sale_item_info_list(i);

        std::string StatisticKey = StringFormat("Item/TotalSold/Id=%u", Item.item_id());
        Database.AddGlobalStatistic(StatisticKey, Item.count());
        Database.AddPlayerStatistic(StatisticKey, Player.GetPlayerId(), Item.count());

        TotalCount += Item.count();
    }

    std::string TotalStatisticKey = StringFormat("Item/TotalSold");
    Database.AddGlobalStatistic(TotalStatisticKey, TotalCount);
    Database.AddPlayerStatistic(TotalStatisticKey, Player.GetPlayerId(), TotalCount);
}

void DS3_LoggingManager::Handle_StrengthenWeaponLog(GameClient* Client, DS3_Frpg2RequestMessage::RequestNotifyProtoBufLog* Request)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_FpdLogMessage::StrengthenWeaponLog Log;
    if (!Log.ParseFromArray(Request->data().data(), (int)Request->data().size()))
    {
        WarningS(Client->GetName().c_str(), "Failed to parse StrengthenWeaponLog.");
        return;
    }

    uint32_t TotalCount = 0;
    for (int i = 0; i < Log.strengthen_weapon_info_list_size(); i++)
    {
        const DS3_FpdLogMessage::StrengthenWeaponLog_Strengthen_weapon_info_list& Item = Log.strengthen_weapon_info_list(i);

        std::string StatisticKey = StringFormat("Item/TotalUpgraded/Id=%u", Item.from_item_id());
        Database.AddGlobalStatistic(StatisticKey, 1);
        Database.AddPlayerStatistic(StatisticKey, Player.GetPlayerId(), 1);

        TotalCount += 1;
    }

    std::string TotalStatisticKey = StringFormat("Item/TotalUpgraded");
    Database.AddGlobalStatistic(TotalStatisticKey, TotalCount);
    Database.AddPlayerStatistic(TotalStatisticKey, Player.GetPlayerId(), TotalCount);
}

void DS3_LoggingManager::Handle_VisitResultLog(GameClient* Client, DS3_Frpg2RequestMessage::RequestNotifyProtoBufLog* Request)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_FpdLogMessage::VisitResultLog Log;
    if (!Log.ParseFromArray(Request->data().data(), (int)Request->data().size()))
    {
        WarningS(Client->GetName().c_str(), "Failed to parse VisitResultLog.");
        return;
    }

#ifdef _DEBUG
    LogS(Client->GetName().c_str(), "Recieved VisitResultLog from: %s", Client->GetName().c_str());
    Log("map_id: %i", Log.map_id());
    Log("location: %.2f, %.2f, %.2f", Log.location().x(), Log.location().y(), Log.location().z());
    Log("online_area_id_source: %i", Log.online_area_id_source());
    Log("online_area_id_destination: %i", Log.online_area_id_destination());
    Log("unknown_2: %i", Log.unknown_2());
#endif
}

MessageHandleResult DS3_LoggingManager::Handle_RequestNotifyKillEnemy(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestNotifyKillEnemy* Request = (DS3_Frpg2RequestMessage::RequestNotifyKillEnemy*)Message.Protobuf.get();

    int EnemyCount = 0;
    for (int i = 0; i < Request->enemys_size(); i++)
    {
        const DS3_Frpg2RequestMessage::KillEnemyInfo& EnemyInfo = Request->enemys(i);

        std::string StatisticKey = StringFormat("Enemies/TotalKilled/Id=%u", EnemyInfo.enemy_type_id());
        Database.AddGlobalStatistic(StatisticKey, EnemyInfo.count());
        Database.AddPlayerStatistic(StatisticKey, Player.GetPlayerId(), EnemyInfo.count());

        EnemyCount += EnemyInfo.count();
    }

    std::string TotalStatisticKey = StringFormat("Enemies/TotalKilled");
    Database.AddGlobalStatistic(TotalStatisticKey, EnemyCount);
    Database.AddPlayerStatistic(TotalStatisticKey, Player.GetPlayerId(), EnemyCount);
    
    DS3_Frpg2RequestMessage::EmptyResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send EmptyResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_LoggingManager::Handle_RequestNotifyDisconnectSession(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestNotifyDisconnectSession* Request = (DS3_Frpg2RequestMessage::RequestNotifyDisconnectSession*)Message.Protobuf.get();

    // Note: I don't think we really care about this log.

    DS3_Frpg2RequestMessage::EmptyResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send EmptyResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_LoggingManager::Handle_RequestNotifyRegisterCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestNotifyRegisterCharacter* Request = (DS3_Frpg2RequestMessage::RequestNotifyRegisterCharacter*)Message.Protobuf.get();

    std::string TotalStatisticKey = StringFormat("Player/TotalRegisteredCharacters");
    Database.AddGlobalStatistic(TotalStatisticKey, 1);
    Database.AddPlayerStatistic(TotalStatisticKey, Player.GetPlayerId(), 1);

    DS3_Frpg2RequestMessage::EmptyResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send EmptyResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_LoggingManager::Handle_RequestNotifyDie(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();
    GameService& Game = *ServerInstance->GetService<GameService>();

    DS3_Frpg2RequestMessage::RequestNotifyDie* Request = (DS3_Frpg2RequestMessage::RequestNotifyDie*)Message.Protobuf.get();

    std::string TypeStatisticKey = StringFormat("Player/TotalDeaths/Cause=%u", (uint32_t)Request->cause_of_death());
    Database.AddGlobalStatistic(TypeStatisticKey, 1);
    Database.AddPlayerStatistic(TypeStatisticKey, Player.GetPlayerId(), 1);

    std::string TotalStatisticKey = StringFormat("Player/TotalDeaths");
    Database.AddGlobalStatistic(TotalStatisticKey, 1);
    Database.AddPlayerStatistic(TotalStatisticKey, Player.GetPlayerId(), 1);

    DS3_Frpg2RequestMessage::EmptyResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send EmptyResponse response.");
        return MessageHandleResult::Error;
    }
    
    if (ServerInstance->GetConfig().SendDiscordNotice_PvP)
    {
        auto& KillerInfo = Request->killer_info();

        if (KillerInfo.killer_player_id() > 0)
        {
            if (auto KillerClient = Game.FindClientByPlayerId(KillerInfo.killer_player_id()); KillerClient != nullptr)
            {
                ServerInstance->SendDiscordNotice(Client->shared_from_this(), DiscordNoticeType::PvPKill,
                    "Was killed by another player.",
                    0,
                    {
                        { "Killer", KillerClient->GetPlayerState().GetCharacterName(), false },
                        
                        { "Killer Soul Level", std::to_string(KillerClient->GetPlayerState().GetSoulLevel()), false },
                        { "Killer Weapon Level", std::to_string(KillerClient->GetPlayerState().GetMaxWeaponLevel()), false },

                        { "Soul Level", std::to_string(Client->GetPlayerState().GetSoulLevel()), false },
                        { "Weapon Level", std::to_string(Client->GetPlayerState().GetMaxWeaponLevel()), false }
                    }
                );
            }
        }
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_LoggingManager::Handle_RequestNotifyKillBoss(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestNotifyKillBoss* Request = (DS3_Frpg2RequestMessage::RequestNotifyKillBoss*)Message.Protobuf.get();

    // Note: I don't think we really care about this log. We get the death information from KillEnemy notification.

    DS3_Frpg2RequestMessage::EmptyResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send EmptyResponse response.");
        return MessageHandleResult::Error;
    }
    
    if (ServerInstance->GetConfig().SendDiscordNotice_Boss)
    {
        DS3_BossId Id = (DS3_BossId)Request->boss_id();

        if (Request->boss_died())
        {
            if (Request->in_coop())
            {
                ServerInstance->SendDiscordNotice(Client->shared_from_this(), DiscordNoticeType::KilledBoss,
                    StringFormat("Killed '%s' with help.", GetEnumString<DS3_BossId>(Id).c_str()),
                    static_cast<uint32_t>(Id),
                    {
                        { "Fight Duration", std::to_string(Request->fight_duration()), false },
                        { "Soul Level", std::to_string(Client->GetPlayerState().GetSoulLevel()), false },
                        { "Weapon Level", std::to_string(Client->GetPlayerState().GetMaxWeaponLevel()), false }
                    }
                );
            }
            else
            {
                ServerInstance->SendDiscordNotice(Client->shared_from_this(), DiscordNoticeType::KilledBoss,
                    StringFormat("Solo killed '%s'.", GetEnumString<DS3_BossId>(Id).c_str()),
                    static_cast<uint32_t>(Id),
                    { 
                        { "Fight Duration", std::to_string(Request->fight_duration()), false },
                        { "Soul Level", std::to_string(Client->GetPlayerState().GetSoulLevel()), false },
                        { "Weapon Level", std::to_string(Client->GetPlayerState().GetMaxWeaponLevel()), false }
                    }
                );
            }
        }
        else
        {
            ServerInstance->SendDiscordNotice(Client->shared_from_this(), DiscordNoticeType::DiedToBoss,
                StringFormat("Died to '%s'.", GetEnumString<DS3_BossId>(Id).c_str()),
                static_cast<uint32_t>(Id),
                {
                    { "Fight Duration", std::to_string(Request->fight_duration()), false },
                    { "Soul Level", std::to_string(Client->GetPlayerState().GetSoulLevel()), false },
                    { "Weapon Level", std::to_string(Client->GetPlayerState().GetMaxWeaponLevel()), false }
                }
            );
        }
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_LoggingManager::Handle_RequestNotifyJoinMultiplay(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestNotifyJoinMultiplay* Request = (DS3_Frpg2RequestMessage::RequestNotifyJoinMultiplay*)Message.Protobuf.get();

    // Note: I don't think we really care about this log. We get most of this info from LeaveMultiplay anyway.

    DS3_Frpg2RequestMessage::EmptyResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send EmptyResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_LoggingManager::Handle_RequestNotifyLeaveMultiplay(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestNotifyLeaveMultiplay* Request = (DS3_Frpg2RequestMessage::RequestNotifyLeaveMultiplay*)Message.Protobuf.get();

    std::string TypeStatisticKey = StringFormat("Player/TotalMultiplaySessions");
    Database.AddGlobalStatistic(TypeStatisticKey, 1);
    Database.AddPlayerStatistic(TypeStatisticKey, Player.GetPlayerId(), 1);

    for (int i = 0; i < Request->party_member_info_size(); i++)
    {        
        const DS3_Frpg2RequestMessage::PartyMemberInfo& Info = Request->party_member_info(i);
        std::string TypeStatisticKey = StringFormat("Player/TotalMultiplaySessions/PartyPlayerId=%u", Info.player_id());
        Database.AddPlayerStatistic(TypeStatisticKey, Player.GetPlayerId(), 1);
    }

    DS3_Frpg2RequestMessage::EmptyResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send EmptyResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_LoggingManager::Handle_RequestNotifySummonSignResult(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestNotifySummonSignResult* Request = (DS3_Frpg2RequestMessage::RequestNotifySummonSignResult*)Message.Protobuf.get();

    // Note: I don't think we really care about this log. We get most of this during the summon flow.

    DS3_Frpg2RequestMessage::EmptyResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send EmptyResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_LoggingManager::Handle_RequestNotifyCreateSignResult(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestNotifyCreateSignResult* Request = (DS3_Frpg2RequestMessage::RequestNotifyCreateSignResult*)Message.Protobuf.get();

    // Note: I don't think we really care about this log. We get most of this during the summon flow.

    DS3_Frpg2RequestMessage::EmptyResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send EmptyResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_LoggingManager::Handle_RequestNotifyBreakInResult(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestNotifyBreakInResult* Request = (DS3_Frpg2RequestMessage::RequestNotifyBreakInResult*)Message.Protobuf.get();

    // Note: I don't think we really care about this log. We get most of this during the summon flow.

    DS3_Frpg2RequestMessage::EmptyResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send EmptyResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string DS3_LoggingManager::GetName()
{
    return "Logging";
}
