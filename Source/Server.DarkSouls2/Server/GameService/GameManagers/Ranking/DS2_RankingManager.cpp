/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Ranking/DS2_RankingManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/DS2_Frpg2ReliableUdpMessage.h"
#include "Server.DarkSouls2/Protobuf/DS2_Protobufs.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"

DS2_RankingManager::DS2_RankingManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

MessageHandleResult DS2_RankingManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestRegisterPowerStoneData))
    {
        return Handle_RequestRegisterPowerStoneData(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestGetPowerStoneRanking))
    {
        return Handle_RequestGetPowerStoneRanking(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestGetPowerStoneMyRanking))
    {
        return Handle_RequestGetPowerStoneMyRanking(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestGetPowerStoneRankingRecordCount))
    {
        return Handle_RequestGetPowerStoneRankingRecordCount(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult DS2_RankingManager::Handle_RequestRegisterPowerStoneData(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestRegisterPowerStoneData* Request = (DS2_Frpg2RequestMessage::RequestRegisterPowerStoneData*)Message.Protobuf.get();

    std::vector<uint8_t> Data;
    Data.assign(Request->data().data(), Request->data().data() + Request->data().size());

    size_t score = Request->increment();
    std::shared_ptr<Ranking> CharRanking = Database.GetCharacterRanking(0, Player.GetPlayerId(), Request->character_id());
    if (CharRanking)
    {
        score += CharRanking->Score;
    }

    if (!Database.RegisterScore(0, Player.GetPlayerId(), Request->character_id(), score, Data))
    {
        WarningS(Client->GetName().c_str(), "Failed to register score in database.");
    }

    std::string TypeStatisticKey = StringFormat("Ranking/TotalRegistrations");
    Database.AddGlobalStatistic(TypeStatisticKey, 1);
    Database.AddPlayerStatistic(TypeStatisticKey, Player.GetPlayerId(), 1);

    // Empty response.
    DS2_Frpg2RequestMessage::RequestRegisterPowerStoneDataResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestRegisterRankingDataResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_RankingManager::Handle_RequestGetPowerStoneRanking(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();

    DS2_Frpg2RequestMessage::RequestGetPowerStoneRanking* Request = (DS2_Frpg2RequestMessage::RequestGetPowerStoneRanking*)Message.Protobuf.get();
    
    std::vector<std::shared_ptr<Ranking>> Rankings = Database.GetRankings(0, Request->offset(), Request->count());
    
    DS2_Frpg2RequestMessage::RequestGetPowerStoneRankingResponse Response;
    for (std::shared_ptr<Ranking>& Ranking : Rankings)
    {
        DS2_Frpg2RequestMessage::PowerStoneRankingData& Data = *Response.add_data();
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

MessageHandleResult DS2_RankingManager::Handle_RequestGetPowerStoneMyRanking(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestGetPowerStoneMyRanking* Request = (DS2_Frpg2RequestMessage::RequestGetPowerStoneMyRanking*)Message.Protobuf.get();
    DS2_Frpg2RequestMessage::RequestGetPowerStoneMyRankingResponse Response;
    DS2_Frpg2RequestMessage::PowerStoneRankingData& ResponseData = *Response.mutable_data();

    std::shared_ptr<Ranking> CharRanking = Database.GetCharacterRanking(0, Player.GetPlayerId(), Request->character_id());
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

MessageHandleResult DS2_RankingManager::Handle_RequestGetPowerStoneRankingRecordCount(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();

    DS2_Frpg2RequestMessage::RequestGetPowerStoneRankingRecordCount* Request = (DS2_Frpg2RequestMessage::RequestGetPowerStoneRankingRecordCount*)Message.Protobuf.get();

    uint32_t RankingCount = Database.GetRankingCount(0);

    DS2_Frpg2RequestMessage::RequestGetPowerStoneRankingRecordCountResponse Response;
    Response.set_count(RankingCount);

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestCountRankingDataResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string DS2_RankingManager::GetName()
{
    return "Ranking";
}
