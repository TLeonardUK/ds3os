/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Server/Database/ServerDatabase.h"

#include "Platform/Platform.h"

#include "Core/Crypto/RSAKeyPair.h"
#include "Core/Network/NetIPAddress.h"

#include <memory>
#include <vector>
#include <filesystem>

#include <steam/steam_api.h>
#include <steam/isteamuser.h>

// This is a very-very-very simple client emulator. Its used to 
// as a super simple way to server behaviour.
// 
// TODO: Split this out into a seperate application.

class NetConnection;
class Frpg2MessageStream;
class Frpg2ReliableUdpMessageStream;
struct Frpg2ReliableUdpMessage;
struct Frpg2Message;

class Client
{
public:
    Client();
    ~Client();

    bool Init(bool DisablePersistentData = false, size_t InstanceId = 0);
    bool Term();
    void RunUntilQuit();

private:

    enum class ClientState
    {
        LoginServer_Connect,
        LoginServer_RequestServerInfo,

        AuthServer_Connect,
        AuthServer_RequestHandshake,
        AuthServer_RequestServiceStatus,
        AuthServer_ExchangeKeyData,
        AuthServer_GetServerInfo,

        GameServer_Connect,
        GameServer_RequestWaitForUserLogin,
        GameServer_RequestGetAnnounceMessageList,
        GameServer_RequestUpdateLoginPlayerCharacter,
        GameServer_RequestUpdatePlayerStatus,
        GameServer_RequestUpdatePlayerCharacter,
        GameServer_RequestGetRightMatchingArea,
        GameServer_Idle,
        GameServer_GatherStatistics,

        Complete
    };

    void Handle_LoginServer_Connect();
    void Handle_LoginServer_RequestServerInfo();

    void Handle_AuthServer_Connect();
    void Handle_AuthServer_RequestHandshake();
    void Handle_AuthServer_RequestServiceStatus();
    void Handle_AuthServer_ExchangeKeyData();
    void Handle_AuthServer_GetServerInfo();

    void Handle_GameServer_Connect();
    void Handle_GameServer_RequestWaitForUserLogin();
    void Handle_GameServer_RequestGetAnnounceMessageList();
    void Handle_GameServer_RequestUpdateLoginPlayerCharacter();
    void Handle_GameServer_RequestUpdatePlayerStatus();
    void Handle_GameServer_RequestUpdatePlayerCharacter();
    void Handle_GameServer_RequestGetRightMatchingArea();
    void Handle_GameServer_Idle();
    void Handle_GameServer_GatherStatistics();

    void ChangeState(ClientState State);

    void WaitForNextMessage(std::shared_ptr<NetConnection> Connection, std::shared_ptr<Frpg2MessageStream> Stream, Frpg2Message& Output);
    void SendAndAwaitWaitForReply(google::protobuf::MessageLite* Request, Frpg2ReliableUdpMessage& Response);
    void SendAndAwaitWaitForReply(google::protobuf::MessageLite* Request, google::protobuf::MessageLite* Response);

    std::string GetName();

    template <typename ...Args>
    void Abort(const char* Format, Args... args)
    {
        ErrorS(GetName().c_str(), Format, args...);
        throw std::exception();
    }


private:

    ClientState State = ClientState::LoginServer_Connect;

    bool QuitRecieved = false;

    PlatformEvents::CtrlSignalEvent::DelegatePtr CtrlSignalHandle = nullptr;

    RSAKeyPair PrimaryKeyPair;

    std::shared_ptr<NetConnection> LoginServerConnection;
    std::shared_ptr<Frpg2MessageStream> LoginServerMessageStream;

    std::shared_ptr<NetConnection> AuthServerConnection;
    std::shared_ptr<Frpg2MessageStream> AuthServerMessageStream;

    std::shared_ptr<NetConnection> GameServerConnection;
    std::shared_ptr<Frpg2ReliableUdpMessageStream> GameServerMessageStream;

    ServerDatabase Database;

    std::filesystem::path SavedPath;
    std::filesystem::path DatabasePath;

    bool DisablePersistentData = false;
    size_t InstanceId = 0;

    std::string ClientStreamId = "";
    int ClientAppVersion = 115;
    int LocalCharacterId = 10;
    int ServerCharacterId;

    int ClientSoulLevel = 0;
    int ClientSoulMemory = 0;
    int ClientWeaponLevel = 0;

    std::string AuthServerIP = "";
    int AuthServerPort = 0;

    bool GotAppTicketResponse = false;
    bool HasAppTicket = false;
    std::vector<uint8_t> AppTicket;
    HAuthTicket AppTicketHandle = k_HAuthTicketInvalid;

    std::vector<uint8_t> GameServerCwcKey;
    uint64_t GameServerAuthToken;
    std::string GameServerIP = "";
    int GameServerPort = 0;
    uint32_t GamePlayerId = 0;

    int ServerPort = 50050;
    //std::string ServerIP = "fdp-steam-ope-login.fromsoftware-game.net";
    //std::string ServerPublicKey = "-----BEGIN RSA PUBLIC KEY-----\nMIIBCgKCAQEA1Nuliw8Rvkt40+0OKoW0JpuSIU/ErQwjzRicZV9JDrCikiTIqoAh\nvBj3DcHwGX1d6T5PY27E4SHa24eRxDetMPEYKeclUeJ0jB07lCtH9Y0zMWl1PMfo\nlIgcm5VKfz+Ua+Ny6klgx1y3ODxMS9g0k11t1WsFtccr464lfP4i1Fgz1/C2Jmgu\n7EV+YdIYkOqT+NJtJG5Z75guq/rTQ85/tVuBKa9dvGIaAqG+nTVlJ2+vzKhjPVXJ\n6AwzWdAbG802uzNC9pk+LEQ+YZXCZSHPMNKz6IwXjlagqDxl2w0rg6dEEFxRY0lm\nS0nqh01eO9pYZA2k0TmpeWHhrKJrnvrKFwIDAQAB\n-----END RSA PUBLIC KEY-----\n";

    std::string ServerIP = "127.0.0.1";
    std::string ServerPublicKey = "-----BEGIN RSA PUBLIC KEY-----\nMIIBCgKCAQEAwUq7HWSq4/nYAsZPxp6QT/3noTvB6YnnVo3EgAI6eRhuztMvikZ6\nzMvjirSEeqENlDHWH5jDrTjK/KYvIcqLU+MaSzMTs/oNxjstlAfqDY9fuQqjagHM\nRHubAh4ubKfcsoO8tI1GB9rD9b++aA2AYIKhP1w/N9qZQmBqB2xaAL7//Ok6aoup\nUd2IRo2m6U3QsjKkYqg5PkiDWJnwlUkTKRsRfwcgEtGHI3ee7mRDZIvEdHjFfwGh\nl279Eq9LrTXG5aN0aXmd4DGmPFch4eI51/L6Vki4DXpmpyNfI7EQiunW2ezA9Qe3\nuueV4NlBwSJBx6IcT35m1yjW+OwvjbHq2wIDAQAB\n-----END RSA PUBLIC KEY-----\n";

};