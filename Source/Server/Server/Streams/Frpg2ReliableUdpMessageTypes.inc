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

// -----------------------------------------------------------------------------------------------
// Boot
// -----------------------------------------------------------------------------------------------
DEFINE_MESSAGE         (0x0320, PlayerInfoUploadConfigPushMessage,  PlayerInfoUploadConfigPushMessage)
DEFINE_REQUEST_RESPONSE(0x0386, RequestWaitForUserLogin,            RequestWaitForUserLogin,            RequestWaitForUserLoginResponse)
DEFINE_REQUEST_RESPONSE(0x03C6, RequestGetAnnounceMessageList,      RequestGetAnnounceMessageList,      RequestGetAnnounceMessageListResponse)

//DEFINE_REQUEST_RESPONSE(0x,   RequestCheckLogin,                  RequestCheckLogin,                  RequestCheckLoginResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestGetPlayerConnectGameServer,  RequestGetPlayerConnectGameServer,  RequestGetPlayerConnectGameServerResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestGetServerConnectTime,        RequestGetServerConnectTime,        RequestGetServerConnectTimeResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestGetConnectionCount,          RequestGetConnectionCount,          RequestGetConnectionCountResponse)
//DEFINE_MESSAGE         (0x,   RequestSendMessageToPushClient,     RequestSendMessageToPushClient)
//DEFINE_MESSAGE         (0x,   RequestGetServerSidePlayerStatus,   RequestGetServerSidePlayerStatus)
//DEFINE_MESSAGE         (0x,   RequestDisconnectUser,              RequestDisconnectUser)
//DEFINE_MESSAGE         (0x,   RequestDisconnectAllUser,           RequestDisconnectAllUser)

// -----------------------------------------------------------------------------------------------
// Player Data
// -----------------------------------------------------------------------------------------------
DEFINE_REQUEST_RESPONSE(0x0394, RequestUpdatePlayerCharacter,       RequestUpdatePlayerCharacter,       RequestUpdatePlayerCharacterResponse)
DEFINE_REQUEST_RESPONSE(0x03A2, RequestUpdateLoginPlayerCharacter,  RequestUpdateLoginPlayerCharacter,  RequestUpdateLoginPlayerCharacterResponse)
DEFINE_REQUEST_RESPONSE(0x03A4, RequestUpdatePlayerStatus,          RequestUpdatePlayerStatus,          RequestUpdatePlayerStatusResponse)

//DEFINE_REQUEST_RESPONSE(0x,   RequestGetPlayerCharacter,          RequestGetPlayerCharacter,          RequestGetPlayerCharacterResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestGetLoginPlayerCharacter,     RequestGetLoginPlayerCharacter,     RequestGetLoginPlayerCharacterResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestGetPlayerCharacterList,      RequestGetPlayerCharacterList,      RequestGetPlayerCharacterListResponse)

// -----------------------------------------------------------------------------------------------
// Blood Messages (user left messages)
// -----------------------------------------------------------------------------------------------
DEFINE_REQUEST_RESPONSE(0x0397, RequestCreateBloodMessage,          RequestCreateBloodMessage,          RequestCreateBloodMessageResponse)
DEFINE_REQUEST_RESPONSE(0x0398, RequestRemoveBloodMessage,          RequestRemoveBloodMessage,          RequestRemoveBloodMessageResponse)
DEFINE_REQUEST_RESPONSE(0x0399, RequestReentryBloodMessage,         RequestReentryBloodMessage,         RequestReentryBloodMessageResponse)
DEFINE_REQUEST_RESPONSE(0x039B, RequestEvaluateBloodMessage,        RequestEvaluateBloodMessage,        RequestEvaluateBloodMessageResponse)
DEFINE_REQUEST_RESPONSE(0x039C, RequestGetBloodMessageEvaluation,   RequestGetBloodMessageEvaluation,   RequestGetBloodMessageEvaluationResponse)
DEFINE_REQUEST_RESPONSE(0x03D8, RequestGetBloodMessageList,         RequestGetBloodMessageList,         RequestGetBloodMessageListResponse)

//DEFINE_REQUEST_RESPONSE(0x,   RequestReCreateBloodMessageList,    RequestReCreateBloodMessageList,    RequestReCreateBloodMessageListResponse)

// -----------------------------------------------------------------------------------------------
// Blood stains (blood puddles with ghosts)
// -----------------------------------------------------------------------------------------------
DEFINE_MESSAGE         (0x0391, RequestCreateBloodstain,            RequestCreateBloodstain)
DEFINE_REQUEST_RESPONSE(0x039A, RequestGetBloodstainList,           RequestGetBloodstainList,           RequestGetBloodstainListResponse)

//DEFINE_MESSAGE       (0x,     PushRequestEvaluateBloodMessage,    PushRequestEvaluateBloodMessage)

// -----------------------------------------------------------------------------------------------
// Summon Signs
// -----------------------------------------------------------------------------------------------
DEFINE_REQUEST_RESPONSE(0x0456, RequestCreateSign,                  RequestCreateSign,                  RequestCreateSignResponse)
DEFINE_REQUEST_RESPONSE(0x0457, RequestUpdateSign,                  RequestUpdateSign,                  RequestUpdateSignResponse)
DEFINE_REQUEST_RESPONSE(0x0459, RequestGetSignList,                 RequestGetSignList,                 RequestGetSignListResponse)
DEFINE_REQUEST_RESPONSE(0x03D5, RequestRemoveSign,                  RequestRemoveSign,                  RequestRemoveSignResponse)
DEFINE_REQUEST_RESPONSE(0x03D9, RequestGetRightMatchingArea,        RequestGetRightMatchingArea,        RequestGetRightMatchingAreaResponse)

//DEFINE_REQUEST_RESPONSE(0x,   RequestSummonSign,                  RequestSummonSign,                  RequestSummonSignResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestRejectSign,                  RequestRejectSign,                  RequestRejectSignResponse)

//DEFINE_MESSAGE         (0x,   PushRequestRemoveSign,              PushRequestRemoveSign)
//DEFINE_MESSAGE         (0x,   PushRequestSummonSign,              PushRequestSummonSign)
//DEFINE_MESSAGE         (0x,   PushRequestRejectSign,              PushRequestRejectSign)

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
DEFINE_MESSAGE         (0x03D0, RequestNotifyRegisterCharacter,     RequestNotifyRegisterCharacter)
DEFINE_MESSAGE         (0x03D1, RequestNotifyDie,                   RequestNotifyDie)
DEFINE_MESSAGE         (0x03D2, RequestNotifyKillBoss,              RequestNotifyKillBoss)
DEFINE_MESSAGE         (0x03D3, RequestNotifyJoinMultiplay,         RequestNotifyJoinMultiplay)
DEFINE_MESSAGE         (0x03D4, RequestNotifyLeaveMultiplay,        RequestNotifyLeaveMultiplay)
DEFINE_MESSAGE         (0x03D6, RequestNotifySummonSignResult,      RequestNotifySummonSignResult)
DEFINE_MESSAGE         (0x03D8, RequestNotifyProtoBufLog,           RequestNotifyProtoBufLog)


//DEFINE_MESSAGE       (0x,     RequestNotifyCreateSignResult,      RequestNotifyCreateSignResult)
//DEFINE_MESSAGE       (0x,     RequestNotifyBreakInResult,         RequestNotifyBreakInResult)
//DEFINE_MESSAGE       (0x,     RequestNotifyDisconnectSession,     RequestNotifyDisconnectSession)

// -----------------------------------------------------------------------------------------------
// Multiplayer
// -----------------------------------------------------------------------------------------------
DEFINE_REQUEST_RESPONSE(0x0320, RequestSendMessageToPlayers,        RequestSendMessageToPlayers,        RequestSendMessageToPlayersResponse)

// -----------------------------------------------------------------------------------------------
// Ranking
// -----------------------------------------------------------------------------------------------
//DEFINE_REQUEST_RESPONSE(0x,   RequestRegisterRankingData,         RequestRegisterRankingData,         RequestRegisterRankingDataResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestGetRankingData,              RequestGetRankingData,              RequestGetRankingDataResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestGetCharacterRankingData,     RequestGetCharacterRankingData,     RequestGetCharacterRankingDataResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestCountRankingData,            RequestCountRankingData,            RequestCountRankingDataResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestGetCurrentRank,              RequestGetCurrentRank,              RequestGetCurrentRankResponse)

// -----------------------------------------------------------------------------------------------
// Quick Match
// -----------------------------------------------------------------------------------------------
//DEFINE_MESSAGE         (0x, PushRequestJoinQuickMatch,            PushRequestJoinQuickMatch)
//DEFINE_MESSAGE         (0x, PushRequestAcceptQuickMatch,          PushRequestAcceptQuickMatch)
//DEFINE_MESSAGE         (0x, PushRequestRejectQuickMatch,          PushRequestRejectQuickMatch)

//DEFINE_REQUEST_RESPONSE(0x,   RequestSearchQuickMatch,            RequestSearchQuickMatch,            RequestSearchQuickMatchResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestRegisterQuickMatch,          RequestRegisterQuickMatch,          RequestRegisterQuickMatchResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestUnregisterQuickMatch,        RequestUnregisterQuickMatch,        RequestUnregisterQuickMatchResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestUpdateQuickMatch,            RequestUpdateQuickMatch,            RequestUpdateQuickMatchResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestJoinQuickMatch,              RequestJoinQuickMatch,              RequestJoinQuickMatchResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestAcceptQuickMatch,            RequestAcceptQuickMatch,            RequestAcceptQuickMatchResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestRejectQuickMatch,            RequestRejectQuickMatch,            RequestRejectQuickMatchResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestSendQuickMatchStart,         RequestSendQuickMatchStart,         RequestSendQuickMatchStartResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestSendQuickMatchResult,        RequestSendQuickMatchResult,        RequestSendQuickMatchResultResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestPushSendQuickMatchStart,     RequestPushSendQuickMatchStart,     RequestPushSendQuickMatchStartResponse)

// -----------------------------------------------------------------------------------------------
// Archedragon Peak Bell
// -----------------------------------------------------------------------------------------------
//DEFINE_MESSAGE         (0x,   PushRequestNotifyRingBell,          PushRequestNotifyRingBell)

//DEFINE_REQUEST_RESPONSE(0x,   RequestNotifyRingBell,              RequestNotifyRingBell,              RequestNotifyRingBellResponse)

// -----------------------------------------------------------------------------------------------
// Break In (Invaders)
// -----------------------------------------------------------------------------------------------
//DEFINE_MESSAGE         (0x,   PushRequestRemoveBreakInTarget,     PushRequestRemoveBreakInTarget)
//DEFINE_MESSAGE         (0x,   PushRequestBreakInTarget,           PushRequestBreakInTarget)
//DEFINE_MESSAGE         (0x,   PushRequestRejectBreakInTarget,     PushRequestRejectBreakInTarget)
//DEFINE_REQUEST_RESPONSE(0x,   RequestGetBreakInTargetList,        RequestGetBreakInTargetList,        RequestGetBreakInTargetListResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestBreakInTarget,               RequestBreakInTarget,               RequestBreakInTargetResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestRejectBreakInTarget,         RequestRejectBreakInTarget,         RequestRejectBreakInTargetResponse)

// -----------------------------------------------------------------------------------------------
// Vistors (Join via covenenant I think)
// -----------------------------------------------------------------------------------------------
DEFINE_REQUEST_RESPONSE  (0x03B4, RequestGetVisitorList,            RequestGetVisitorList,              RequestGetVisitorListResponse)
DEFINE_REQUEST_RESPONSE  (0x03B5, RequestVisit,                     RequestVisit,                       RequestVisitResponse)
//DEFINE_REQUEST_RESPONSE(0x,     RequestRejectVisit,               RequestRejectVisit,                 RequestRejectVisitResponse)

//DEFINE_MESSAGE         (0x, PushRequestRemoveVisitor,             PushRequestRemoveVisitor)
//DEFINE_MESSAGE         (0x, PushRequestVisit,                     PushRequestVisit)
//DEFINE_MESSAGE         (0x, PushRequestRejectVisit,               PushRequestRejectVisit)

// -----------------------------------------------------------------------------------------------
// Networking
// -----------------------------------------------------------------------------------------------
//DEFINE_REQUEST_RESPONSE(0x,   RequestMeasureUploadBandwidth,              RequestMeasureUploadBandwidth,                  RequestMeasureUploadBandwidthResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestMeasureDownloadBandwidth,            RequestMeasureDownloadBandwidth,                RequestMeasureDownloadBandwidthResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestSendMessageToPlayers,                RequestSendMessageToPlayers,                    RequestSendMessageToPlayersResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestSendMessageToSelectLanguagePlayers,  RequestSendMessageToSelectLanguagePlayers,      RequestSendMessageToSelectLanguagePlayersResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestSendMessageToAllPlayers,             RequestSendMessageToAllPlayers,                 RequestSendMessageToAllPlayersResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestSendPlayerList,                      RequestSendPlayerList,                          RequestSendPlayerListResponse)

// -----------------------------------------------------------------------------------------------
// Online Shop
// -----------------------------------------------------------------------------------------------
//DEFINE_REQUEST_RESPONSE(0x,   RequestGetOnlineShopItemList,               RequestGetOnlineShopItemList,                   RequestGetOnlineShopItemListResponse)

// -----------------------------------------------------------------------------------------------
// Regulation Files
// -----------------------------------------------------------------------------------------------
//DEFINE_REQUEST_RESPONSE(0x,   RequestGetRegulationFile,                   RequestGetRegulationFile,                       RequestGetRegulationFileResponse)

// -----------------------------------------------------------------------------------------------
// Ritual Marks (No idea what this is, probably cut content).
// -----------------------------------------------------------------------------------------------
//DEFINE_REQUEST_RESPONSE(0x,   RequestCreateMark,                          RequestCreateMark,                              RequestCreateMarkResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestRemoveMark,                          RequestRemoveMark,                              RequestRemoveMarkResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestReentryMark,                         RequestReentryMark,                             RequestReentryMarkResponse)
//DEFINE_REQUEST_RESPONSE(0x,   RequestGetMarkList,                         RequestGetMarkList,                             RequestGetMarkListResponse)

// -----------------------------------------------------------------------------------------------
// Misc
// -----------------------------------------------------------------------------------------------
//DEFINE_REQUEST_RESPONSE(0x,   RequestBenchmarkThroughput,                 RequestBenchmarkThroughput,                     RequestBenchmarkThroughputResponse)

