﻿/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Client/Client.h"
#include "Core/Utils/Logging.h"
#include "Core/Utils/File.h"
#include "Core/Utils/Strings.h"
#include "Core/Utils/Random.h"
#include "Core/Crypto/CWCCipher.h"
#include "Core/Network/NetUtils.h"
#include "Core/Network/NetHttpRequest.h"
#include "Core/Network/NetConnection.h"
#include "Core/Network/NetConnectionTCP.h"
#include "Core/Network/NetConnectionUDP.h"

#include "Server/Streams/Frpg2Message.h"
#include "Server/Streams/Frpg2MessageStream.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Server/AuthService/AuthClient.h"

#include <thread>
#include <chrono>
#include <fstream>

#include "ThirdParty/nlohmann/json.hpp"
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

using namespace std::chrono_literals;

Client::Client()
{
    // TODO: Move this stuff into a RuntimeConfig type class.
    SavedPath = std::filesystem::current_path() / std::filesystem::path("ClientSaved");
    DatabasePath = SavedPath / std::filesystem::path("database.sqlite");

    // Register for Ctrl+C notifications, its the only way the server shuts down right now.
    CtrlSignalHandle = PlatformEvents::OnCtrlSignal.Register([=]() {
        Warning("Quit signal recieved, starting shutdown.");        
        QuitRecieved = true;
    });
}

Client::~Client()
{
    CtrlSignalHandle.reset();
}

bool Client::Init()
{
    ClientStreamId = StringFormat("%016llx", SteamUser()->GetSteamID().ConvertToUint64());

    // We use template files from a given steam-id, you will probably get banned if you run
    // this on any arbitrary account without replacing the templates.
    Ensure(ClientStreamId == "011000014a0ce047");

    Log("Initializing client '%s' ...", ClientStreamId.c_str());

    // Generate folder we are going to save everything into.
    if (!std::filesystem::is_directory(SavedPath))
    {
        if (!std::filesystem::create_directories(SavedPath))
        {
            Error("Failed to create save path: %s", SavedPath.string().c_str());
            return false;
        }
    }

    // Open connection to our database.
    if (!Database.Open(DatabasePath))
    {
        Error("Failed to open database at '%s'.", DatabasePath.string().c_str());
        return false;
    }

    if (!PrimaryKeyPair.LoadPublicKeyFromString(ServerPublicKey))
    {
        Error("Failed to load rsa keypair.");
        return false;
    }

    Log("Requesting auth session ticket ...");
    AppTicket.resize(2048);
    uint32 TicketLength = 0;
    AppTicketHandle = SteamUser()->GetAuthSessionTicket(AppTicket.data(), (int)AppTicket.size(), &TicketLength);

    if (AppTicketHandle != k_HAuthTicketInvalid)
    {
        AppTicket.resize(TicketLength);
        Log("Recieved auth session ticket of length %i", TicketLength);
    }
    else
    {
        Error("Failed to retrieve auth session ticket.");
        return false;
    }

    ChangeState(ClientState::LoginServer_Connect);

    return true;
}

bool Client::Term()
{
    Log("Terminating server ...");

    if (AppTicketHandle != k_HAuthTicketInvalid)
    {
        SteamUser()->CancelAuthTicket(AppTicketHandle);
        AppTicketHandle = k_HAuthTicketInvalid;
    }

    if (!Database.Close())
    {
        Error("Failed to close database.");
        return false;
    }

    return true;
}

void Client::RunUntilQuit()
{
    Success("Client is now running.");

    // We should really do this event driven ...
    // This suffices for now.
    while (!QuitRecieved)
    {
        switch (State)
        {
            case ClientState::LoginServer_Connect:                                  Handle_LoginServer_Connect();                                   break;
            case ClientState::LoginServer_RequestServerInfo:                        Handle_LoginServer_RequestServerInfo();                         break;

            case ClientState::AuthServer_Connect:                                   Handle_AuthServer_Connect();                                    break;
            case ClientState::AuthServer_RequestHandshake:                          Handle_AuthServer_RequestHandshake();                           break;
            case ClientState::AuthServer_RequestServiceStatus:                      Handle_AuthServer_RequestServiceStatus();                       break;
            case ClientState::AuthServer_ExchangeKeyData:                           Handle_AuthServer_ExchangeKeyData();                            break;
            case ClientState::AuthServer_GetServerInfo:                             Handle_AuthServer_GetServerInfo();                              break;

            case ClientState::GameServer_Connect:                                   Handle_GameServer_Connect();                                    break;
            case ClientState::GameServer_RequestWaitForUserLogin:                   Handle_GameServer_RequestWaitForUserLogin();                    break;
            case ClientState::GameServer_RequestGetAnnounceMessageList:             Handle_GameServer_RequestGetAnnounceMessageList();              break;
            case ClientState::GameServer_RequestUpdateLoginPlayerCharacter:         Handle_GameServer_RequestUpdateLoginPlayerCharacter();          break;
            case ClientState::GameServer_RequestUpdatePlayerStatus:                 Handle_GameServer_RequestUpdatePlayerStatus();                  break;
            case ClientState::GameServer_RequestUpdatePlayerCharacter:              Handle_GameServer_RequestUpdatePlayerCharacter();               break;
            case ClientState::GameServer_RequestGetRightMatchingArea:               Handle_GameServer_RequestGetRightMatchingArea();                break;
            case ClientState::GameServer_Experiment:                                Handle_GameServer_Experiment();                                 break;
            case ClientState::GameServer_GatherStatistics:                          Handle_GameServer_GatherStatistics();                           break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Client::ChangeState(ClientState NewState)
{
    State = NewState;
}

void Client::WaitForNextMessage(std::shared_ptr<NetConnection> Connection, std::shared_ptr<Frpg2MessageStream> Stream, Frpg2Message& Output)
{
    while (!Stream->Recieve(&Output))
    {
        if (Connection->Pump())
        {
            Fatal("Connection entered error state while waiting for message.");
        }
        if (Stream->Pump())
        {
            Fatal("Message stream entered error state while waiting for message.");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Client::SendAndAwaitWaitForReply(google::protobuf::MessageLite* Request, Frpg2ReliableUdpMessage& Response)
{
    Ensure(GameServerMessageStream->Send(Request, nullptr));

    uint32_t RequestId = GameServerMessageStream->GetLastSentMessageIndex();

    while (true)
    {
        if (GameServerConnection->Pump())
        {
            Fatal("Connection entered error state while waiting for message.");
        }
        if (GameServerMessageStream->Pump())
        {
            Fatal("Message stream entered error state while waiting for message.");
        }

        Frpg2ReliableUdpMessage Message;

        if (GameServerMessageStream->Recieve(&Message))
        {
            GameServerMessageStream->HandledPacket(Message.AckSequenceIndex);

            if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::Reply &&
                Message.Header.msg_index == Message.Header.msg_index)
            {
                Response = Message;
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Client::SendAndAwaitWaitForReply(google::protobuf::MessageLite* Request, google::protobuf::MessageLite* Response)
{
    Frpg2ReliableUdpMessage UntypedResponse;
    SendAndAwaitWaitForReply(Request, UntypedResponse);

    bool Result = Response->ParseFromArray(UntypedResponse.Payload.data(), (int)UntypedResponse.Payload.size());
    Ensure(Result);
}

void Client::Handle_LoginServer_Connect()
{
    Log("Connecting to login server.");

    LoginServerConnection = std::make_shared<NetConnectionTCP>("Client Emulator - Login Server");
    if (!LoginServerConnection->Connect(ServerIP, ServerPort, true))
    {
        Fatal("Failed to connect to server at %s:%i", ServerIP, ServerPort);
    }

    LoginServerMessageStream = std::make_shared<Frpg2MessageStream>(LoginServerConnection, &PrimaryKeyPair, true);

    ChangeState(ClientState::LoginServer_RequestServerInfo);

    Log("Connected to login server.");
}

void Client::Handle_LoginServer_RequestServerInfo()
{
    Log("Requesting server info.");

    Frpg2RequestMessage::RequestQueryLoginServerInfo Request;
    Request.set_steam_id(ClientStreamId.c_str());
    Request.set_app_version(ClientAppVersion);
    Ensure(LoginServerMessageStream->Send(&Request, Frpg2MessageType::RequestQueryLoginServerInfo));

    Frpg2Message Response;
    WaitForNextMessage(LoginServerConnection, LoginServerMessageStream, Response);

    Frpg2RequestMessage::RequestQueryLoginServerInfoResponse TypedResponse;
    Ensure(TypedResponse.ParseFromArray(Response.Payload.data(), (int)Response.Payload.size()));

    AuthServerIP = TypedResponse.server_ip();
    AuthServerPort = (int)TypedResponse.port();

    Log("Recieved auth server info: %s:%i", AuthServerIP.c_str(), AuthServerPort);

    LoginServerConnection->Disconnect();
    LoginServerConnection = nullptr;

    ChangeState(ClientState::AuthServer_Connect);
}

void Client::Handle_AuthServer_Connect()
{
    Log("Connecting to auth server.");

    AuthServerConnection = std::make_shared<NetConnectionTCP>("Client Emulator - Auth Server");
    if (!AuthServerConnection->Connect(AuthServerIP, AuthServerPort))
    {
        Fatal("Failed to connect to server at %s:%i", AuthServerIP, AuthServerPort);
    }

    AuthServerMessageStream = std::make_shared<Frpg2MessageStream>(AuthServerConnection, &PrimaryKeyPair, true);

    ChangeState(ClientState::AuthServer_RequestHandshake);

    Log("Connected to auth server.");
}

void Client::Handle_AuthServer_RequestHandshake()
{
    Log("Requesting handshake.");

    std::vector<uint8_t> CwcKey;
    CwcKey.resize(16);
    FillRandomBytes(CwcKey);

    Frpg2RequestMessage::RequestHandshake Request;
    Request.set_aes_cwc_key(CwcKey.data(), CwcKey.size());
    Ensure(AuthServerMessageStream->Send(&Request, Frpg2MessageType::RequestHandshake));

    AuthServerMessageStream->SetCipher(nullptr, nullptr);

    Frpg2Message KeyExchangeResponse;
    WaitForNextMessage(AuthServerConnection, AuthServerMessageStream, KeyExchangeResponse);
    Ensure(KeyExchangeResponse.Payload.size() == 27);

    AuthServerMessageStream->SetCipher(std::make_shared<CWCCipher>(CwcKey), std::make_shared<CWCCipher>(CwcKey));

    ChangeState(ClientState::AuthServer_RequestServiceStatus);

    Log("Handshake completed with auth server.");
}

void Client::Handle_AuthServer_RequestServiceStatus()
{
    Log("Requesting service status.");

    Frpg2RequestMessage::GetServiceStatus Request;
    Request.set_id(1);
    Request.set_steam_id(ClientStreamId.c_str(), (int)ClientStreamId.size());
    Request.set_app_version(ClientAppVersion);
    Ensure(AuthServerMessageStream->Send(&Request, Frpg2MessageType::GetServiceStatus));

    Frpg2Message Response;
    WaitForNextMessage(AuthServerConnection, AuthServerMessageStream, Response);

    if (Response.Payload.size() == 0)
    {
        Fatal("New version of application available or server is down for maintenance.");
    }

    Frpg2RequestMessage::GetServiceStatusResponse TypedResponse;
    Ensure(TypedResponse.ParseFromArray(Response.Payload.data(), (int)Response.Payload.size()));

    ChangeState(ClientState::AuthServer_ExchangeKeyData);

    Log("Recieved service status from auth server.");
}

void Client::Handle_AuthServer_ExchangeKeyData()
{
    Log("Exchanging game server key material.");

    std::vector<uint8_t> HalfGameCwcKey;
    HalfGameCwcKey.resize(8);
    FillRandomBytes(HalfGameCwcKey);

    Frpg2Message KeyExchangeMessage;
    KeyExchangeMessage.Payload = HalfGameCwcKey;
    Ensure(AuthServerMessageStream->Send(KeyExchangeMessage, Frpg2MessageType::KeyMaterial));

    Frpg2Message KeyExchangeResponse;
    WaitForNextMessage(AuthServerConnection, AuthServerMessageStream, KeyExchangeResponse);
    Ensure(KeyExchangeResponse.Payload.size() == 16);

    GameServerCwcKey = KeyExchangeResponse.Payload;

    ChangeState(ClientState::AuthServer_GetServerInfo);

    Log("Completed game server key exchange.");
}

void Client::Handle_AuthServer_GetServerInfo()
{
    Log("Sending steam ticket to auth server.");

    Frpg2Message SteamTicketMessage; 
    SteamTicketMessage.Payload.resize(AppTicket.size() + 16);
    memcpy(SteamTicketMessage.Payload.data(), GameServerCwcKey.data(), 16);
    memcpy(SteamTicketMessage.Payload.data() + 16, AppTicket.data(), AppTicket.size());
    Ensure(AuthServerMessageStream->Send(SteamTicketMessage, Frpg2MessageType::SteamTicket));
     
    Frpg2Message GameInfoResponse;
    WaitForNextMessage(AuthServerConnection, AuthServerMessageStream, GameInfoResponse);
    Ensure(GameInfoResponse.Payload.size() == sizeof(Frpg2GameServerInfo));

    Frpg2GameServerInfo GameInfo;
    memcpy(&GameInfo, GameInfoResponse.Payload.data(), sizeof(Frpg2GameServerInfo));
    GameInfo.SwapEndian(); 

    GameServerAuthToken = GameInfo.auth_token;
    GameServerIP = GameInfo.game_server_ip;
    GameServerPort = GameInfo.game_port;

    ChangeState(ClientState::GameServer_Connect);

    Log("Recieved game server info: %s:%i (auth token: 0x%016llx)", GameInfo.game_server_ip, GameInfo.game_port, GameInfo.auth_token);
}

void Client::Handle_GameServer_Connect()
{
    Log("Connecting to game server.");

    GameServerConnection = std::make_shared<NetConnectionUDP>("Client Emulator - Game Server");
    if (!GameServerConnection->Connect(GameServerIP, GameServerPort))
    {
        Fatal("Failed to connect to server at %s:%i", GameServerIP, GameServerPort);
    }

    GameServerMessageStream = std::make_shared<Frpg2ReliableUdpMessageStream>(GameServerConnection, GameServerCwcKey, GameServerAuthToken, true);
    GameServerMessageStream->Connect(ClientStreamId);

    while (GameServerMessageStream->GetState() != Frpg2ReliableUdpStreamState::Established)
    {
        if (GameServerConnection->Pump())
        {
            Fatal("Connection entered error state while waiting for message.");
        }
        if (GameServerMessageStream->Pump())
        {
            Fatal("Message stream entered error state while waiting for message.");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));        
    }

    ChangeState(ClientState::GameServer_RequestWaitForUserLogin);

    Log("Connected to game server.");
}

void Client::Handle_GameServer_RequestWaitForUserLogin()
{
    Log("Waiting for user login.");

    Frpg2RequestMessage::RequestWaitForUserLogin Request;
    Request.set_steam_id(ClientStreamId.c_str(), ClientStreamId.size());
    Request.set_unknown_1(1);
    Request.set_unknown_2(0);
    Request.set_unknown_3(1);
    Request.set_unknown_4(2);
    
    Frpg2RequestMessage::RequestWaitForUserLoginResponse Response;
    SendAndAwaitWaitForReply(&Request, &Response);

    GamePlayerId = Response.player_id();

    Log("Logged in as player id %i", GamePlayerId);

    ChangeState(ClientState::GameServer_RequestGetAnnounceMessageList);
}

void Client::Handle_GameServer_RequestGetAnnounceMessageList()
{
    Log("Requesting announcement messages.");

    Frpg2RequestMessage::RequestGetAnnounceMessageList Request;
    Request.set_max_entries(100);

    Frpg2RequestMessage::RequestGetAnnounceMessageListResponse Response;
    SendAndAwaitWaitForReply(&Request, &Response);

    Log("Recieved announcements.");
    Log("\tChanges=%i", Response.changes().items_size());
    Log("\tNotices=%i", Response.notices().items_size());

    /*for (int i = 0; i < Response.changes().items_size(); i++)
    {
        Log("=== Change %i ===", i);
        Log("%s", Response.changes().items(i).header().c_str());
        Log("%s", Response.changes().items(i).message().c_str());
    }
    for (int i = 0; i < Response.notices().items_size(); i++)
    {
        Log("=== Notice %i ===", i);
        Log("%s", Response.notices().items(i).header().c_str());
        Log("%s", Response.notices().items(i).message().c_str());
    }*/

    ChangeState(ClientState::GameServer_RequestUpdateLoginPlayerCharacter);
}

void Client::Handle_GameServer_RequestUpdateLoginPlayerCharacter()
{
    Log("Requesting update of login player character.");

    Frpg2RequestMessage::RequestUpdateLoginPlayerCharacter Request;
    Request.set_character_id(LocalCharacterId);
    Request.mutable_unknown_2()->Add(LocalCharacterId);

    Frpg2RequestMessage::RequestUpdateLoginPlayerCharacterResponse Response;
    SendAndAwaitWaitForReply(&Request, &Response);

    ServerCharacterId = Response.character_id();

    Log("Recieved update login player character response.");
    Log("\tLocal Character Id=%i", LocalCharacterId);
    Log("\tServer Character Id=%i", ServerCharacterId);
    Log("\tQuick Match Brawl Rank: Rank=%i XP=%i", Response.quickmatch_brawl_rank().rank(), Response.quickmatch_brawl_rank().xp());
    Log("\tQuick Match Dual Rank: Rank=%i XP=%i", Response.quickmatch_dual_rank().rank(), Response.quickmatch_dual_rank().xp());

    ChangeState(ClientState::GameServer_RequestUpdatePlayerStatus);
}

void Client::Handle_GameServer_RequestUpdatePlayerStatus()
{
    Log("Requesting update of player status.");

    std::vector<uint8_t> RequestTemplateBytes;
    Ensure(ReadBytesFromFile("../../Resources/TemplateProtobufs/RequestUpdatePlayerStatus.dat", RequestTemplateBytes));

    Frpg2RequestMessage::RequestUpdatePlayerStatus Request;
    Request.ParseFromArray(RequestTemplateBytes.data(), (int)RequestTemplateBytes.size());

    std::string bytes = Request.status();
    Frpg2PlayerData::AllStatus status;
    status.ParseFromArray(bytes.data(), (int)bytes.size());

    ClientSoulLevel = status.player_status().soul_level();
    ClientSoulMemory = status.player_status().soul_memory();
    ClientWeaponLevel = status.player_status().max_weapon_level();

    Frpg2RequestMessage::RequestUpdatePlayerStatusResponse Response;
    SendAndAwaitWaitForReply(&Request, &Response);

    Log("Recieved update player status response.");

    ChangeState(ClientState::GameServer_RequestUpdatePlayerCharacter);
}

void Client::Handle_GameServer_RequestUpdatePlayerCharacter()
{
    Log("Requesting update of player character.");

    std::vector<uint8_t> RequestTemplateBytes;
    Ensure(ReadBytesFromFile("../../Resources/TemplateProtobufs/RequestUpdatePlayerCharacter.dat", RequestTemplateBytes));

    Frpg2RequestMessage::RequestUpdatePlayerCharacter Request;
    Request.ParseFromArray(RequestTemplateBytes.data(), (int)RequestTemplateBytes.size());

    Frpg2RequestMessage::RequestUpdatePlayerCharacterResponse Response;
    SendAndAwaitWaitForReply(&Request, &Response);

    Log("Recieved update player character response.");

    ChangeState(ClientState::GameServer_RequestGetRightMatchingArea);
}

void Client::Handle_GameServer_RequestGetRightMatchingArea()
{
    Log("Requesting right matching area.");

    Frpg2RequestMessage::RequestGetRightMatchingArea Request;
    Request.mutable_matching_parameter()->set_unknown_id_1(1350000);
    Request.mutable_matching_parameter()->set_unknown_id_2(2);
    Request.mutable_matching_parameter()->set_unknown_id_3(0);
    Request.mutable_matching_parameter()->set_unknown_id_4(1);
    Request.mutable_matching_parameter()->set_unknown_id_5(0);
    Request.mutable_matching_parameter()->set_soul_level(ClientSoulLevel);
    Request.mutable_matching_parameter()->set_soul_memory(ClientSoulMemory);
    Request.mutable_matching_parameter()->set_unknown_id_9(0);
    Request.mutable_matching_parameter()->set_password("");
    Request.mutable_matching_parameter()->set_covenant(Frpg2RequestMessage::Covenant_Blue_Sentinels);
    Request.mutable_matching_parameter()->set_weapon_level(ClientWeaponLevel);
    Request.set_unknown(0);

    Frpg2RequestMessage::RequestGetRightMatchingAreaResponse Response;
    SendAndAwaitWaitForReply(&Request, &Response);

    Log("Recieved populated matching areas.");
    for (int i = 0; i < Response.area_info_size(); i++)
    {
        const Frpg2RequestMessage::RequestGetRightMatchingAreaResponse_Area_info& AreaInfo = Response.area_info(i);
        Log("\tArea:%i Population:%i", AreaInfo.online_area_id(), AreaInfo.population());
    }

    ChangeState(ClientState::GameServer_GatherStatistics);
}

void Client::Handle_GameServer_Experiment()
{
    /*
    Log("Requesting regulation file.");

    std::vector<uint8_t> RequestTemplateBytes;
    Ensure(ReadBytesFromFile("../../Resources/TemplateProtobufs/RequestUpdatePlayerCharacter.dat", RequestTemplateBytes));

    Frpg2RequestMessage::RequestGetRegulationFile Request;
    Request.set_unknown_1(0);

    Frpg2RequestMessage::RequestGetRegulationFileResponse Response;
    SendAndAwaitWaitForReply(&Request, &Response);

    Log("Recieved regulation file response");
    */
    ChangeState(ClientState::Complete);
}

void Client::Handle_GameServer_GatherStatistics()
{
    std::vector<OnlineAreaId> OnlineAreaIds = *GetEnumValues<OnlineAreaId>();

    // Go through each area and each level/weapon config and find out how many people 
    // are available for coop/invasion at each combination.
    for (int Level = 150; Level <= 200; Level += 10)
    {
        for (int WeaponLevel = 0; WeaponLevel <= 10; WeaponLevel += 1)
        {
            Log("===== Level:%i WeaponLevel:%i =====", Level, WeaponLevel);

            for (OnlineAreaId Area : OnlineAreaIds)
            {
                if (Area == OnlineAreaId::None)
                {
                    continue;
                }

                int32 AreaMapId = ((int32)Area / 10000) * 10000;

                // Get sign statistics.
                {
                    Frpg2RequestMessage::RequestGetSignList Request;
                    Request.set_unknown_id_1(0);
                    Request.set_max_signs(100);
                    Request.mutable_sign_get_flags()->set_unknown_id_1(1);
                    Request.mutable_sign_get_flags()->set_unknown_id_2(1);
                    Request.mutable_sign_get_flags()->set_unknown_id_3(0);

                    Frpg2RequestMessage::SignDomainGetInfo* DomainInfo = Request.mutable_search_areas()->Add();
                    DomainInfo->set_max_signs(100);
                    DomainInfo->set_online_area_id((int32)Area);

                    Request.mutable_matching_parameter()->set_unknown_id_1(1350000);
                    Request.mutable_matching_parameter()->set_unknown_id_2(2);
                    Request.mutable_matching_parameter()->set_unknown_id_3(0);
                    Request.mutable_matching_parameter()->set_unknown_id_4(1);
                    Request.mutable_matching_parameter()->set_unknown_id_5(0);
                    Request.mutable_matching_parameter()->set_soul_level(Level);
                    Request.mutable_matching_parameter()->set_soul_memory(Level * 100000); // Huuum, this might cause some issues.
                    Request.mutable_matching_parameter()->set_unknown_id_9(0);
                    Request.mutable_matching_parameter()->set_password("");
                    Request.mutable_matching_parameter()->set_covenant(Frpg2RequestMessage::Covenant_Blue_Sentinels);
                    Request.mutable_matching_parameter()->set_weapon_level(WeaponLevel);

                    Frpg2RequestMessage::RequestGetSignListResponse Response;
                    SendAndAwaitWaitForReply(&Request, &Response);

                    int SignCount = Response.has_get_sign_result() ? Response.get_sign_result().sign_data_size() : 0;
                    if (SignCount > 0)
                    {
                        Log("Area:%i Level:%i WeaponLevel:%i: Got %i signs", Area, Level, WeaponLevel, SignCount);
                        Database.AddMatchingSample("ActiveSigns", StringFormat("%i", (int32)Area), SignCount, Level, WeaponLevel);
                    }
                }

                // Get invasion statistics.
                {
                    Frpg2RequestMessage::RequestGetBreakInTargetList Request;
                    Request.set_map_id(AreaMapId);
                    Request.set_online_area_id((int32)Area);
                    Request.set_max_targets(100);
                    Request.set_unknown_5(0);

                    Request.mutable_matching_parameter()->set_unknown_id_1(1350000);
                    Request.mutable_matching_parameter()->set_unknown_id_2(2);
                    Request.mutable_matching_parameter()->set_unknown_id_3(0);
                    Request.mutable_matching_parameter()->set_unknown_id_4(1);
                    Request.mutable_matching_parameter()->set_unknown_id_5(0);
                    Request.mutable_matching_parameter()->set_soul_level(Level);
                    Request.mutable_matching_parameter()->set_soul_memory(Level * 100000); // Huuum, this might cause some issues.
                    Request.mutable_matching_parameter()->set_unknown_id_9(0);
                    Request.mutable_matching_parameter()->set_password("");
                    Request.mutable_matching_parameter()->set_covenant(Frpg2RequestMessage::Covenant_Blue_Sentinels);
                    Request.mutable_matching_parameter()->set_weapon_level(WeaponLevel);

                    Frpg2RequestMessage::RequestGetBreakInTargetListResponse Response;
                    SendAndAwaitWaitForReply(&Request, &Response);

                    int TargetCount = Response.target_data_size();
                    if (TargetCount > 0)
                    {
                        Log("Area:%i Level:%i WeaponLevel:%i: Got %i break in targets", Area, Level, WeaponLevel, TargetCount);
                        Database.AddMatchingSample("ActiveBreakInTargets", StringFormat("%i", (int32)Area), TargetCount, Level, WeaponLevel);
                    }
                }

                // Don't hammer the server.
                std::this_thread::sleep_for(200ms);
            }
        }
    }
}