/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license. 
 * You should have received a copy of the license along with this program. 
 * If not, see <https://opensource.org/licenses/MIT>.
 */

// Macro fun!
// This defines the different types of messages that can be sent/recieved from the dark souls server.
//
// There are 2 macros you can use, one for messages that expect a reply, and one for one way messages.
// 
// DEFINE_REQUEST_RESPONSE(uint32_t OpCode, Frpg2ReliableUdpMessageType Type, Frpg2RequestMessage::* Class, Frpg2RequestMessage::* ResponseClass)
// DEFINE_MESSAGE(uint32_t OpCode, Frpg2ReliableUdpMessageType Type, Frpg2RequestMessage::* Class)

// Boot
DEFINE_REQUEST_RESPONSE(0x0386, RequestWaitForUserLogin,            RequestWaitForUserLogin,            RequestWaitForUserLoginResponse)
DEFINE_REQUEST_RESPONSE(0x03C6, RequestGetAnnounceMessageList,      RequestGetAnnounceMessageList,      RequestGetAnnounceMessageListResponse)
DEFINE_MESSAGE         (0x0320, PlayerInfoUploadConfigPushMessage,  PlayerInfoUploadConfigPushMessage)

// Player Data
DEFINE_REQUEST_RESPONSE(0x03A2, RequestUpdateLoginPlayerCharacter,  RequestUpdateLoginPlayerCharacter,  RequestUpdateLoginPlayerCharacterResponse)
DEFINE_REQUEST_RESPONSE(0x03A4, RequestUpdatePlayerStatus,          RequestUpdatePlayerStatus,          RequestUpdatePlayerStatusResponse)
DEFINE_REQUEST_RESPONSE(0x0394, RequestUpdatePlayerCharacter,       RequestUpdatePlayerCharacter,       RequestUpdatePlayerCharacterResponse)

// Blood Messages
DEFINE_REQUEST_RESPONSE(0x0399, RequestReentryBloodMessage,         RequestReentryBloodMessage,         RequestReentryBloodMessageResponse)

// Logging
DEFINE_MESSAGE         (0x03D8, RequestNotifyProtoBufLog,           RequestNotifyProtoBufLog)


