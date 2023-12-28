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

// Note all push messages use id 0x0320 in the message header. They are disambiguated by the first field in their protobuf.

// -----------------------------------------------------------------------------------------------
// Boot
// -----------------------------------------------------------------------------------------------
DEFINE_PUSH_MESSAGE    (0x038C, PlayerInfoUploadConfigPushMessage,  PlayerInfoUploadConfigPushMessage)                                                 
DEFINE_REQUEST_RESPONSE(0x0386, RequestWaitForUserLogin,            RequestWaitForUserLogin,            RequestWaitForUserLoginResponse)
DEFINE_REQUEST_RESPONSE(0x03C6, RequestGetAnnounceMessageList,      RequestGetAnnounceMessageList,      RequestGetAnnounceMessageListResponse)

// -----------------------------------------------------------------------------------------------
// Player Data
// -----------------------------------------------------------------------------------------------
DEFINE_REQUEST_RESPONSE(0x0394, RequestUpdatePlayerCharacter,       RequestUpdatePlayerCharacter,       RequestUpdatePlayerCharacterResponse)
DEFINE_REQUEST_RESPONSE(0x0395, RequestGetPlayerCharacter,          RequestGetPlayerCharacter,          RequestGetPlayerCharacterResponse)
DEFINE_REQUEST_RESPONSE(0x039F, RequestGetLoginPlayerCharacter,     RequestGetLoginPlayerCharacter,     RequestGetLoginPlayerCharacterResponse)
DEFINE_REQUEST_RESPONSE(0x03A1, RequestGetPlayerCharacterList,      RequestGetPlayerCharacterList,      RequestGetPlayerCharacterListResponse)
DEFINE_REQUEST_RESPONSE(0x03A2, RequestUpdateLoginPlayerCharacter,  RequestUpdateLoginPlayerCharacter,  RequestUpdateLoginPlayerCharacterResponse)
DEFINE_REQUEST_RESPONSE(0x03A4, RequestUpdatePlayerStatus,          RequestUpdatePlayerStatus,          RequestUpdatePlayerStatusResponse)

// -----------------------------------------------------------------------------------------------
// Blood Messages (user left messages)
// -----------------------------------------------------------------------------------------------
DEFINE_REQUEST_RESPONSE(0x0397, RequestCreateBloodMessage,          RequestCreateBloodMessage,          RequestCreateBloodMessageResponse)
DEFINE_REQUEST_RESPONSE(0x0398, RequestRemoveBloodMessage,          RequestRemoveBloodMessage,          RequestRemoveBloodMessageResponse)
DEFINE_REQUEST_RESPONSE(0x0399, RequestReentryBloodMessage,         RequestReentryBloodMessage,         RequestReentryBloodMessageResponse)
DEFINE_REQUEST_RESPONSE(0x039A, RequestGetBloodMessageList,         RequestGetBloodMessageList,         RequestGetBloodMessageListResponse)
DEFINE_REQUEST_RESPONSE(0x039B, RequestEvaluateBloodMessage,        RequestEvaluateBloodMessage,        RequestEvaluateBloodMessageResponse)
DEFINE_REQUEST_RESPONSE(0x039C, RequestGetBloodMessageEvaluation,   RequestGetBloodMessageEvaluation,   RequestGetBloodMessageEvaluationResponse)
DEFINE_REQUEST_RESPONSE(0x03DA, RequestReCreateBloodMessageList,    RequestReCreateBloodMessageList,    RequestReCreateBloodMessageListResponse)

// Can't find registration in code.
DEFINE_PUSH_MESSAGE    (0x0396, PushRequestEvaluateBloodMessage, PushRequestEvaluateBloodMessage)

// -----------------------------------------------------------------------------------------------
// Blood stains (blood puddles with ghosts)
// -----------------------------------------------------------------------------------------------
DEFINE_MESSAGE         (0x0391, RequestCreateBloodstain,            RequestCreateBloodstain)
DEFINE_REQUEST_RESPONSE(0x0392, RequestGetBloodstainList,           RequestGetBloodstainList,           RequestGetBloodstainListResponse)

// -----------------------------------------------------------------------------------------------
// Summon Signs
// -----------------------------------------------------------------------------------------------
DEFINE_REQUEST_RESPONSE(0x0456, RequestCreateSign,                  RequestCreateSign,                  RequestCreateSignResponse)
DEFINE_REQUEST_RESPONSE(0x0457, RequestUpdateSign,                  RequestUpdateSign,                  RequestUpdateSignResponse)
DEFINE_REQUEST_RESPONSE(0x0458, RequestRemoveSign,                  RequestRemoveSign,                  RequestRemoveSignResponse)
DEFINE_REQUEST_RESPONSE(0x0459, RequestGetSignList,                 RequestGetSignList,                 RequestGetSignListResponse)
DEFINE_REQUEST_RESPONSE(0x045A, RequestSummonSign,                  RequestSummonSign,                  RequestSummonSignResponse)
DEFINE_REQUEST_RESPONSE(0x045B, RequestRejectSign,                  RequestRejectSign,                  RequestRejectSignResponse)
DEFINE_REQUEST_RESPONSE(0x03D9, RequestGetRightMatchingArea,        RequestGetRightMatchingArea,        RequestGetRightMatchingAreaResponse)

DEFINE_PUSH_MESSAGE    (0x033D, PushRequestRemoveSign,              PushRequestRemoveSign)
DEFINE_PUSH_MESSAGE    (0x033E, PushRequestSummonSign,              PushRequestSummonSign)
DEFINE_PUSH_MESSAGE    (0x033F, PushRequestRejectSign,              PushRequestRejectSign)

// -----------------------------------------------------------------------------------------------
// Ghosts
// -----------------------------------------------------------------------------------------------
DEFINE_REQUEST_RESPONSE(0x0393, RequestGetDeadingGhost,             RequestGetDeadingGhost,             RequestGetDeadingGhostResponse)
DEFINE_REQUEST_RESPONSE(0x039D, RequestCreateGhostData,             RequestCreateGhostData,             RequestCreateGhostDataResponse)
DEFINE_REQUEST_RESPONSE(0x039E, RequestGetGhostDataList,            RequestGetGhostDataList,            RequestGetGhostDataListResponse)

// -----------------------------------------------------------------------------------------------
// Logging
// -----------------------------------------------------------------------------------------------
DEFINE_MESSAGE         (0x03CD, RequestNotifyKillEnemy,             RequestNotifyKillEnemy)
DEFINE_MESSAGE         (0x03CF, RequestNotifyDisconnectSession,     RequestNotifyDisconnectSession)
DEFINE_MESSAGE         (0x03D0, RequestNotifyRegisterCharacter,     RequestNotifyRegisterCharacter)
DEFINE_MESSAGE         (0x03D1, RequestNotifyDie,                   RequestNotifyDie)
DEFINE_MESSAGE         (0x03D2, RequestNotifyKillBoss,              RequestNotifyKillBoss)
DEFINE_MESSAGE         (0x03D3, RequestNotifyJoinMultiplay,         RequestNotifyJoinMultiplay)
DEFINE_MESSAGE         (0x03D4, RequestNotifyLeaveMultiplay,        RequestNotifyLeaveMultiplay)
DEFINE_MESSAGE         (0x03D6, RequestNotifySummonSignResult,      RequestNotifySummonSignResult)
DEFINE_MESSAGE         (0x03D8, RequestNotifyProtoBufLog,           RequestNotifyProtoBufLog)
DEFINE_MESSAGE         (0x03D7, RequestNotifyBreakInResult,         RequestNotifyBreakInResult)
DEFINE_MESSAGE         (0x03D5, RequestNotifyCreateSignResult,      RequestNotifyCreateSignResult)

// -----------------------------------------------------------------------------------------------
// Ranking
// -----------------------------------------------------------------------------------------------
DEFINE_REQUEST_RESPONSE(0x03E8, RequestRegisterRankingData,         RequestRegisterRankingData,         RequestRegisterRankingDataResponse)
DEFINE_REQUEST_RESPONSE(0x03E9, RequestGetRankingData,              RequestGetRankingData,              RequestGetRankingDataResponse)
DEFINE_REQUEST_RESPONSE(0x03EA, RequestGetCharacterRankingData,     RequestGetCharacterRankingData,     RequestGetCharacterRankingDataResponse)
DEFINE_REQUEST_RESPONSE(0x03EB, RequestCountRankingData,            RequestCountRankingData,            RequestCountRankingDataResponse)

// Can't find registration in code.
//DEFINE_REQUEST_RESPONSE(0x,   RequestGetCurrentRank,              RequestGetCurrentRank,              RequestGetCurrentRankResponse)

// -----------------------------------------------------------------------------------------------
// Quick Match
// -----------------------------------------------------------------------------------------------

// Looking in the code all the following match this: 0x0340,  0x0343, 0x0346,  0x0349,  0x034c,  0x034f,  0x0352,  0x0355 - We will only use one though.
DEFINE_PUSH_MESSAGE    (0x0340,   PushRequestJoinQuickMatch,        PushRequestJoinQuickMatch)
// Looking in the code all the following match this: 0x0341, 0x0344, 0x0347, 0x034a, 0x034d, 0x0350, 0x0353, 0x0356 - We will only use one though.
DEFINE_PUSH_MESSAGE    (0x0341,   PushRequestAcceptQuickMatch,      PushRequestAcceptQuickMatch)
// Looking in the code all the following match this: 0x0342, 0x0345, 0x0348, 0x034b, 0x034e, 0x0351, 0x0354, 0x0357 - We will only use one though.
DEFINE_PUSH_MESSAGE    (0x0342,   PushRequestRejectQuickMatch,      PushRequestRejectQuickMatch)

DEFINE_REQUEST_RESPONSE(0x044C,   RequestSearchQuickMatch,          RequestSearchQuickMatch,            RequestSearchQuickMatchResponse)
DEFINE_REQUEST_RESPONSE(0x044D,   RequestUnregisterQuickMatch,      RequestUnregisterQuickMatch,        RequestUnregisterQuickMatchResponse)
DEFINE_REQUEST_RESPONSE(0x044E,   RequestUpdateQuickMatch,          RequestUpdateQuickMatch,            RequestUpdateQuickMatchResponse)
DEFINE_REQUEST_RESPONSE(0x044F,   RequestJoinQuickMatch,            RequestJoinQuickMatch,              RequestJoinQuickMatchResponse)
DEFINE_REQUEST_RESPONSE(0x0450,   RequestAcceptQuickMatch,          RequestAcceptQuickMatch,            RequestAcceptQuickMatchResponse)
DEFINE_REQUEST_RESPONSE(0x0451,   RequestRejectQuickMatch,          RequestRejectQuickMatch,            RequestRejectQuickMatchResponse)
DEFINE_REQUEST_RESPONSE(0x0452,   RequestRegisterQuickMatch,        RequestRegisterQuickMatch,          RequestRegisterQuickMatchResponse)
DEFINE_REQUEST_RESPONSE(0x0454,   RequestSendQuickMatchStart,       RequestSendQuickMatchStart,         RequestSendQuickMatchStartResponse)
DEFINE_REQUEST_RESPONSE(0x0455,   RequestSendQuickMatchResult,      RequestSendQuickMatchResult,        RequestSendQuickMatchResultResponse)

// Can't find registration in code.
//DEFINE_REQUEST_RESPONSE(0x,     RequestPushSendQuickMatchStart,   RequestPushSendQuickMatchStart,     RequestPushSendQuickMatchStartResponse)

// -----------------------------------------------------------------------------------------------
// Archedragon Peak Bell
// -----------------------------------------------------------------------------------------------

// Can't find registration in code.
DEFINE_PUSH_MESSAGE    (0x03C9,   PushRequestNotifyRingBell,        PushRequestNotifyRingBell)

DEFINE_REQUEST_RESPONSE(0x03C8,   RequestNotifyRingBell,            RequestNotifyRingBell,              RequestNotifyRingBellResponse)

// -----------------------------------------------------------------------------------------------
// Break In (Invaders)
// -----------------------------------------------------------------------------------------------
DEFINE_PUSH_MESSAGE    (0x03A5,   PushRequestBreakInTarget,         PushRequestBreakInTarget)             
DEFINE_PUSH_MESSAGE    (0x03A6,   PushRequestRejectBreakInTarget,   PushRequestRejectBreakInTarget)
DEFINE_PUSH_MESSAGE    (0x03A7,   PushRequestAllowBreakInTarget,    PushRequestAllowBreakInTarget)

// Can't find registration in code.
//DEFINE_PUSH_MESSAGE  (0x,   PushRequestRemoveBreakInTarget,     PushRequestRemoveBreakInTarget)

DEFINE_REQUEST_RESPONSE(0x03B1,   RequestGetBreakInTargetList,      RequestGetBreakInTargetList,        RequestGetBreakInTargetListResponse)
DEFINE_REQUEST_RESPONSE(0x03B2,   RequestBreakInTarget,             RequestBreakInTarget,               RequestBreakInTargetResponse)
DEFINE_REQUEST_RESPONSE(0x03B3,   RequestRejectBreakInTarget,       RequestRejectBreakInTarget,         RequestRejectBreakInTargetResponse)

// -----------------------------------------------------------------------------------------------
// Vistors (Join via covenenant I think)
// -----------------------------------------------------------------------------------------------
DEFINE_REQUEST_RESPONSE  (0x03B4, RequestGetVisitorList,            RequestGetVisitorList,              RequestGetVisitorListResponse)
DEFINE_REQUEST_RESPONSE  (0x03B5, RequestVisit,                     RequestVisit,                       RequestVisitResponse)
DEFINE_REQUEST_RESPONSE  (0x03B6, RequestRejectVisit,               RequestRejectVisit,                 RequestRejectVisitResponse)

// Looking in the code all the following match this: 0x03b7, 0x03ba, 0x03bd, 0x03c0, 0x03c3 - We will only use one though.
DEFINE_PUSH_MESSAGE      (0x03B7, PushRequestVisit,                 PushRequestVisit)
// Looking in the code all the following match this: 0x03b8, 0x03bb, 0x03be, 0x03c1, 0x03c4 - We will only use one though.
DEFINE_PUSH_MESSAGE      (0x03B8, PushRequestRejectVisit,           PushRequestRejectVisit)
// Looking in the code all the following match this: 0x03b9, 0x03bc, 0x03bf, 0x03c2, 0x03c5 - We will only use one though.
DEFINE_PUSH_MESSAGE      (0x03B9, PushRequestRemoveVisitor,         PushRequestRemoveVisitor)

// -----------------------------------------------------------------------------------------------
// Networking
// -----------------------------------------------------------------------------------------------
DEFINE_REQUEST_RESPONSE(0x0320,   RequestSendMessageToPlayers,      RequestSendMessageToPlayers,        RequestSendMessageToPlayersResponse)                            // Conflict
DEFINE_REQUEST_RESPONSE(0x038E,   RequestMeasureUploadBandwidth,    RequestMeasureUploadBandwidth,      RequestMeasureUploadBandwidthResponse)
DEFINE_REQUEST_RESPONSE(0x038F,   RequestMeasureDownloadBandwidth,  RequestMeasureDownloadBandwidth,    RequestMeasureDownloadBandwidthResponse)

// -----------------------------------------------------------------------------------------------
// Online Shop
// -----------------------------------------------------------------------------------------------
DEFINE_REQUEST_RESPONSE(0x03DB,   RequestGetOnlineShopItemList,     RequestGetOnlineShopItemList,       RequestGetOnlineShopItemListResponse)

// -----------------------------------------------------------------------------------------------
// Regulation Files
// -----------------------------------------------------------------------------------------------

// Can't find registration in code.
//DEFINE_REQUEST_RESPONSE(0x,       RequestGetRegulationFile,           RequestGetRegulationFile,           RequestGetRegulationFileResponse)

DEFINE_PUSH_MESSAGE    (0x038B,     RegulationFileUpdatePushMessage,    RegulationFileUpdatePushMessage)

// -----------------------------------------------------------------------------------------------
// Ritual Marks (No idea what this is, probably cut content).
// -----------------------------------------------------------------------------------------------
DEFINE_REQUEST_RESPONSE(0x0460,   RequestCreateMark,                RequestCreateMark,                  RequestCreateMarkResponse)
DEFINE_REQUEST_RESPONSE(0x0461,   RequestRemoveMark,                RequestRemoveMark,                  RequestRemoveMarkResponse)
DEFINE_REQUEST_RESPONSE(0x0462,   RequestReentryMark,               RequestReentryMark,                 RequestReentryMarkResponse)
DEFINE_REQUEST_RESPONSE(0x0463,   RequestGetMarkList,               RequestGetMarkList,                 RequestGetMarkListResponse)

// -----------------------------------------------------------------------------------------------
// Misc
// -----------------------------------------------------------------------------------------------
DEFINE_REQUEST_RESPONSE(0x03A3,   RequestBenchmarkThroughput,       RequestBenchmarkThroughput,         RequestBenchmarkThroughputResponse)
DEFINE_PUSH_MESSAGE    (0x0389,   ManagementTextMessage,            ManagementTextMessage)
DEFINE_PUSH_MESSAGE    (0x038D,   ServerPing,                       ServerPing)

