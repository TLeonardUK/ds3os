/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Ranking/DS3_RankingManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/DS3_Frpg2ReliableUdpMessage.h"
#include "Server.DarkSouls3/Protobuf/DS3_Protobufs.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"

DS3_RankingManager::DS3_RankingManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

MessageHandleResult DS3_RankingManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestRegisterRankingData))
    {
        return Handle_RequestRegisterRankingData(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestGetRankingData))
    {
        return Handle_RequestGetRankingData(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestGetCharacterRankingData))
    {
        return Handle_RequestGetCharacterRankingData(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestCountRankingData))
    {
        return Handle_RequestCountRankingData(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult DS3_RankingManager::Handle_RequestRegisterRankingData(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestRegisterRankingData* Request = (DS3_Frpg2RequestMessage::RequestRegisterRankingData*)Message.Protobuf.get();

    std::vector<uint8_t> Data;
    Data.assign(Request->data().data(), Request->data().data() + Request->data().size());

    if (!Database.RegisterScore(Request->board_id(), Player.GetPlayerId(), Request->character_id(), Request->score(), Data))
    {
        WarningS(Client->GetName().c_str(), "Failed to register score in database.");
    }

    std::string TypeStatisticKey = StringFormat("Ranking/TotalRegistrations");
    Database.AddGlobalStatistic(TypeStatisticKey, 1);
    Database.AddPlayerStatistic(TypeStatisticKey, Player.GetPlayerId(), 1);

    // Empty response.
    DS3_Frpg2RequestMessage::RequestRegisterRankingDataResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestRegisterRankingDataResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_RankingManager::Handle_RequestGetRankingData(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();

    DS3_Frpg2RequestMessage::RequestGetRankingData* Request = (DS3_Frpg2RequestMessage::RequestGetRankingData*)Message.Protobuf.get();
    
    std::vector<std::shared_ptr<Ranking>> Rankings = Database.GetRankings(Request->board_id(), Request->offset(), Request->count());
    
    DS3_Frpg2RequestMessage::RequestGetRankingDataResponse Response;
    for (std::shared_ptr<Ranking>& Ranking : Rankings)
    {
        DS3_Frpg2RequestMessage::RankingData& Data = *Response.add_data();
        Data.set_player_id(Ranking->PlayerId);
        Data.set_character_id(Ranking->CharacterId);
        Data.set_serial_rank(Ranking->SerialRank);
        Data.set_rank(Ranking->Rank);
        Data.set_score(Ranking->Score);
        Data.set_data(Ranking->Data.data(), Ranking->Data.size());
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetRankingDataResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_RankingManager::Handle_RequestGetCharacterRankingData(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestGetCharacterRankingData* Request = (DS3_Frpg2RequestMessage::RequestGetCharacterRankingData*)Message.Protobuf.get();
    DS3_Frpg2RequestMessage::RequestGetCharacterRankingDataResponse Response;
    DS3_Frpg2RequestMessage::RankingData& ResponseData = *Response.mutable_data();

    std::shared_ptr<Ranking> CharRanking = Database.GetCharacterRanking(Request->board_id(), Player.GetPlayerId(), Request->character_id());
    if (CharRanking)
    {
        ResponseData.set_player_id(Player.GetPlayerId());
        ResponseData.set_character_id(Request->character_id());
        ResponseData.set_serial_rank(CharRanking->SerialRank);
        ResponseData.set_rank(CharRanking->Rank);
        ResponseData.set_score(CharRanking->Score);
        ResponseData.set_data(CharRanking->Data.data(), CharRanking->Data.size());
    }
    else
    {
        ResponseData.set_player_id(Player.GetPlayerId());
        ResponseData.set_character_id(Request->character_id());
        ResponseData.set_serial_rank(0);
        ResponseData.set_rank(0);
        ResponseData.set_score(0);
        ResponseData.set_data("");
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetCharacterRankingDataResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_RankingManager::Handle_RequestCountRankingData(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();

    DS3_Frpg2RequestMessage::RequestCountRankingData* Request = (DS3_Frpg2RequestMessage::RequestCountRankingData*)Message.Protobuf.get();

    uint32_t RankingCount = Database.GetRankingCount(Request->board_id());

    DS3_Frpg2RequestMessage::RequestCountRankingDataResponse Response;
    Response.set_count(RankingCount);

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestCountRankingDataResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string DS3_RankingManager::GetName()
{
    return "Ranking";
}
