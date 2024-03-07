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
DEFINE_PUSH_MESSAGE         (0x038C, PlayerInfoUploadConfigPushMessage,  PlayerInfoUploadConfigPushMessage)                                                 
DEFINE_REQUEST_RESPONSE     (0x0386, RequestWaitForUserLogin,            RequestWaitForUserLogin,            RequestWaitForUserLoginResponse)

DEFINE_REQUEST_RESPONSE     (0x03EC, RequestGetAnnounceMessageList,      RequestGetAnnounceMessageList,      RequestGetAnnounceMessageListResponse)

// -----------------------------------------------------------------------------------------------
// Player Data
// -----------------------------------------------------------------------------------------------
DEFINE_REQUEST_RESPONSE     (0x03B3, RequestGetLoginPlayerCharacter,     RequestGetLoginPlayerCharacter,     RequestGetLoginPlayerCharacterResponse)
DEFINE_REQUEST_RESPONSE     (0x03B6, RequestUpdateLoginPlayerCharacter,  RequestUpdateLoginPlayerCharacter,  RequestUpdateLoginPlayerCharacterResponse)
DEFINE_REQUEST_RESPONSE     (0x03B8, RequestUpdatePlayerStatus,          RequestUpdatePlayerStatus,          RequestUpdatePlayerStatusResponse)
DEFINE_REQUEST_RESPONSE     (0x03A8, RequestUpdatePlayerCharacter,       RequestUpdatePlayerCharacter,       RequestUpdatePlayerCharacterResponse)
DEFINE_REQUEST_RESPONSE     (0x03A9, RequestGetPlayerCharacter,          RequestGetPlayerCharacter,          RequestGetPlayerCharacterList)

// -----------------------------------------------------------------------------------------------
// Blood Messages (user left messages)
// -----------------------------------------------------------------------------------------------

// Can't find usage in code.
DEFINE_PUSH_MESSAGE         (0x03AA, PushRequestEvaluateBloodMessage,    PushRequestEvaluateBloodMessage)

DEFINE_REQUEST_RESPONSE     (0x03AD, RequestReentryBloodMessage,         RequestReentryBloodMessage,        RequestReentryBloodMessageResponse)
DEFINE_REQUEST_RESPONSE     (0x03AB, RequestCreateBloodMessage,          RequestCreateBloodMessage,         RequestCreateBloodMessageResponse)
DEFINE_REQUEST_RESPONSE     (0x03AC, RequestRemoveBloodMessage,          RequestRemoveBloodMessage,         RequestRemoveBloodMessageResponse)
DEFINE_REQUEST_RESPONSE     (0x03AE, RequestGetBloodMessageList,         RequestGetBloodMessageList,        RequestGetBloodMessageListResponse)
DEFINE_REQUEST_RESPONSE     (0x03AF, RequestEvaluateBloodMessage,        RequestEvaluateBloodMessage,       RequestEvaluateBloodMessageResponse)
DEFINE_REQUEST_RESPONSE     (0x03B0, RequestGetBloodMessageEvaluation,   RequestGetBloodMessageEvaluation,  RequestGetBloodMessageEvaluationResponse)
DEFINE_REQUEST_RESPONSE     (0x03FF, RequestGetAreaBloodMessageList,     RequestGetAreaBloodMessageList,    RequestGetBloodMessageListResponse)

// -----------------------------------------------------------------------------------------------
// Blood stains (blood puddles with ghosts)
// -----------------------------------------------------------------------------------------------
DEFINE_MESSAGE              (0x0391, RequestCreateBloodstain,           RequestCreateBloodstain)
DEFINE_REQUEST_RESPONSE     (0x0392, RequestGetBloodstainList,          RequestGetBloodstainList,           RequestGetBloodstainListResponse)
DEFINE_REQUEST_RESPONSE     (0x0400, RequestGetAreaBloodstainList,      RequestGetAreaBloodstainList,       RequestGetBloodstainListResponse)

// -----------------------------------------------------------------------------------------------
// Summon Signs
// -----------------------------------------------------------------------------------------------
DEFINE_PUSH_MESSAGE         (0x039B, PushRequestSummonSign,             PushRequestSummonSign)
DEFINE_PUSH_MESSAGE         (0x039C, PushRequestRejectSign,             PushRequestRejectSign)
DEFINE_PUSH_MESSAGE         (0x039D, PushRequestRemoveSign,             PushRequestRemoveSign)

DEFINE_REQUEST_RESPONSE     (0x0398, RequestSummonSign,                 RequestSummonSign,                  RequestSummonSignResponse)
DEFINE_REQUEST_RESPONSE     (0x0394, RequestCreateSign,                 RequestCreateSign,                  RequestCreateSignResponse)
DEFINE_REQUEST_RESPONSE     (0x0397, RequestGetSignList,                RequestGetSignList,                 RequestGetSignListResponse)
DEFINE_REQUEST_RESPONSE     (0x039A, RequestRejectSign,                 RequestRejectSign,                  RequestRejectSignResponse)
DEFINE_REQUEST_RESPONSE     (0x0396, RequestRemoveSign,                 RequestRemoveSign,                  RequestRemoveSignResponse)
DEFINE_REQUEST_RESPONSE     (0x0395, RequestUpdateSign,                 RequestUpdateSign,                  RequestUpdateSignResponse)

DEFINE_REQUEST_RESPONSE     (0x03fa, RequestGetRightMatchingArea,       RequestGetRightMatchingArea,        RequestGetRightMatchingAreaResponse)

// -----------------------------------------------------------------------------------------------
// Ghosts
// -----------------------------------------------------------------------------------------------
DEFINE_REQUEST_RESPONSE     (0x0393, RequestGetDeadingGhost,            RequestGetDeadingGhost,             RequestGetDeadingGhostResponse)
DEFINE_REQUEST_RESPONSE     (0x03B1, RequestCreateGhostData,            RequestCreateGhostData,             RequestCreateGhostDataResponse)
DEFINE_REQUEST_RESPONSE     (0x03B2, RequestGetGhostDataList,           RequestGetGhostDataList,            RequestGetGhostDataListResponse)

// -----------------------------------------------------------------------------------------------
// Logging
// -----------------------------------------------------------------------------------------------
DEFINE_REQUEST_RESPONSE     (0x03F7, RequestNotifyBuyItem,              RequestNotifyBuyItem,               RequestNotifyBuyItemResponse)
DEFINE_REQUEST_RESPONSE     (0x03F1, RequestNotifyDeath,                RequestNotifyDeath,                 RequestNotifyDeathResponse)
DEFINE_REQUEST_RESPONSE     (0x03F9, RequestNotifyDisconnectSession,    RequestNotifyDisconnectSession,     RequestNotifyDisconnectSessionResponse)
DEFINE_REQUEST_RESPONSE     (0x03E8, RequestNotifyJoinGuestPlayer,      RequestNotifyJoinGuestPlayer,       RequestNotifyJoinGuestPlayerResponse)
DEFINE_REQUEST_RESPONSE     (0x03EA, RequestNotifyJoinSession,          RequestNotifyJoinSession,           RequestNotifyJoinSessionResponse)
DEFINE_REQUEST_RESPONSE     (0x03F6, RequestNotifyKillEnemy,            RequestNotifyKillEnemy,             RequestNotifyKillEnemyResponse)
DEFINE_REQUEST_RESPONSE     (0x03ED, RequestNotifyKillPlayer,           RequestNotifyKillPlayer,            RequestNotifyKillPlayerResponse)
DEFINE_REQUEST_RESPONSE     (0x03E9, RequestNotifyLeaveGuestPlayer,     RequestNotifyLeaveGuestPlayer,      RequestNotifyLeaveGuestPlayerResponse)
DEFINE_REQUEST_RESPONSE     (0x03EB, RequestNotifyLeaveSession,         RequestNotifyLeaveSession,          RequestNotifyLeaveSessionResponse)
DEFINE_REQUEST_RESPONSE     (0x03D8, RequestNotifyMirrorKnight,         RequestNotifyMirrorKnight,          RequestNotifyMirrorKnightResponse)
DEFINE_REQUEST_RESPONSE     (0x03F2, RequestNotifyOfflineDeathCount,    RequestNotifyOfflineDeathCount,     RequestNotifyOfflineDeathCountResponse)

// -----------------------------------------------------------------------------------------------
// Ranking
// -----------------------------------------------------------------------------------------------
DEFINE_REQUEST_RESPONSE     (0x03F5, RequestGetPowerStoneMyRanking,     RequestGetPowerStoneMyRanking,      RequestGetPowerStoneMyRankingResponse)
DEFINE_REQUEST_RESPONSE     (0x03F4, RequestGetPowerStoneRanking,       RequestGetPowerStoneRanking,        RequestGetPowerStoneRankingResponse)
DEFINE_REQUEST_RESPONSE     (0x03F8, RequestGetPowerStoneRankingRecordCount, RequestGetPowerStoneRankingRecordCount, RequestGetPowerStoneRankingRecordCountResponse)
DEFINE_REQUEST_RESPONSE     (0x03F3, RequestRegisterPowerStoneData,     RequestRegisterPowerStoneData,      RequestRegisterPowerStoneDataResponse)

// -----------------------------------------------------------------------------------------------
// Mirror Knight
// -----------------------------------------------------------------------------------------------
DEFINE_PUSH_MESSAGE         (0x03A6, PushRequestRejectMirrorKnightSign, PushRequestRejectMirrorKnightSign)
DEFINE_PUSH_MESSAGE         (0x03A7, PushRequestRemoveMirrorKnightSign, PushRequestRemoveMirrorKnightSign)
DEFINE_PUSH_MESSAGE         (0x03A5, PushRequestSummonMirrorKnightSign, PushRequestSummonMirrorKnightSign)

DEFINE_REQUEST_RESPONSE     (0x039e, RequestCreateMirrorKnightSign,     RequestCreateMirrorKnightSign,      RequestCreateMirrorKnightSignResponse)
DEFINE_REQUEST_RESPONSE     (0x03a1, RequestGetMirrorKnightSignList,    RequestGetMirrorKnightSignList,     RequestGetMirrorKnightSignListResponse)
DEFINE_REQUEST_RESPONSE     (0x03a4, RequestRejectMirrorKnightSign,     RequestRejectMirrorKnightSign,      RequestRejectMirrorKnightSignResponse)
DEFINE_REQUEST_RESPONSE     (0x03a0, RequestRemoveMirrorKnightSign,     RequestRemoveMirrorKnightSign,      RequestRemoveMirrorKnightSignResponse)
DEFINE_REQUEST_RESPONSE     (0x03a2, RequestSummonMirrorKnightSign,     RequestSummonMirrorKnightSign,      RequestSummonMirrorKnightSignResponse)
DEFINE_REQUEST_RESPONSE     (0x039f, RequestUpdateMirrorKnightSign,     RequestUpdateMirrorKnightSign,      RequestUpdateMirrorKnightSignResponse)

// -----------------------------------------------------------------------------------------------
// Break In (Invaders)
// -----------------------------------------------------------------------------------------------
DEFINE_PUSH_MESSAGE         (0x03FD, PushRequestAllowBreakInTarget,     PushRequestAllowBreakInTarget)
DEFINE_PUSH_MESSAGE         (0x03FB, PushRequestBreakInTarget,          PushRequestBreakInTarget)
DEFINE_PUSH_MESSAGE         (0x03FC, PushRequestRejectBreakInTarget,    PushRequestRejectBreakInTarget)

// Can't find registration in code.
//DEFINE_PUSH_MESSAGE         (0x0000, PushRequestRemoveBreakInTarget,    PushRequestRemoveBreakInTarget)

DEFINE_REQUEST_RESPONSE     (0x03d3, RequestBreakInTarget,              RequestBreakInTarget,               RequestBreakInTargetResponse)
DEFINE_REQUEST_RESPONSE     (0x03d2, RequestGetBreakInTargetList,       RequestGetBreakInTargetList,        RequestGetBreakInTargetListResponse)
DEFINE_REQUEST_RESPONSE     (0x03d4, RequestRejectBreakInTarget,        RequestRejectBreakInTarget,         RequestRejectBreakInTargetResponse)

// -----------------------------------------------------------------------------------------------
// Vistors (Join via covenenant I think)
// -----------------------------------------------------------------------------------------------
DEFINE_PUSH_MESSAGE         (0x03D1, PushRequestRemoveVisitor,          PushRequestRemoveVisitor)
DEFINE_PUSH_MESSAGE         (0x03D0, PushRequestRejectVisit,            PushRequestRejectVisit)
DEFINE_PUSH_MESSAGE         (0x03CF, PushRequestVisit,                  PushRequestVisit)

DEFINE_REQUEST_RESPONSE     (0x03d5, RequestGetVisitorList,             RequestGetVisitorList,              RequestGetVisitorListResponse)
DEFINE_REQUEST_RESPONSE     (0x03d6, RequestVisit,                      RequestVisit,                       RequestVisitResponse)
DEFINE_REQUEST_RESPONSE     (0x03d7, RequestRejectVisit,                RequestRejectVisit,                 RequestRejectVisitResponse)

// -----------------------------------------------------------------------------------------------
// Bell
// -----------------------------------------------------------------------------------------------

// Can't find registration in code
//DEFINE_PUSH_MESSAGE         (0x03C9, PushRequestNotifyRingBell,         PushRequestNotifyRingBell)

DEFINE_REQUEST_RESPONSE     (0x03EE, RequestNotifyRingBell,             RequestNotifyRingBell,              RequestNotifyRingBellResponse)

// -----------------------------------------------------------------------------------------------
// Regulation Files
// -----------------------------------------------------------------------------------------------

// Can't find registration in code.
//DEFINE_REQUEST_RESPONSE     (0x0000, RequestGetRegulationFile,          RequestGetRegulationFile,           RequestGetRegulationFileResponse)

// -----------------------------------------------------------------------------------------------
// Quick Match
// -----------------------------------------------------------------------------------------------
DEFINE_PUSH_MESSAGE         (0x03E5, PushRequestAllowQuickMatch,        PushRequestAllowQuickMatch)
DEFINE_PUSH_MESSAGE         (0x03E1, PushRequestJoinQuickMatch,         PushRequestJoinQuickMatch)
DEFINE_PUSH_MESSAGE         (0x03E3, PushRequestRejectQuickMatch,       PushRequestRejectQuickMatch)
DEFINE_PUSH_MESSAGE         (0x03E7, PushRequestRemoveQuickMatch,       PushRequestRemoveQuickMatch)

DEFINE_REQUEST_RESPONSE     (0x03dd, RequestJoinQuickMatch,             RequestJoinQuickMatch,              RequestJoinQuickMatchResponse)
DEFINE_REQUEST_RESPONSE     (0x03d9, RequestRegisterQuickMatch,         RequestRegisterQuickMatch,          RequestRegisterQuickMatchResponse)
DEFINE_REQUEST_RESPONSE     (0x03de, RequestRejectQuickMatch,           RequestRejectQuickMatch,            RequestRejectQuickMatchResponse)
DEFINE_REQUEST_RESPONSE     (0x03dc, RequestSearchQuickMatch,           RequestSearchQuickMatch,            RequestSearchQuickMatchResponse)
DEFINE_REQUEST_RESPONSE     (0x03da, RequestUnregisterQuickMatch,       RequestUnregisterQuickMatch,        RequestUnregisterQuickMatchResponse)
DEFINE_REQUEST_RESPONSE     (0x03db, RequestUpdateQuickMatch,           RequestUpdateQuickMatch,            RequestUpdateQuickMatchResponse)

// -----------------------------------------------------------------------------------------------
// Death Count
// -----------------------------------------------------------------------------------------------
DEFINE_REQUEST_RESPONSE     (0x03F0, RequestGetTotalDeathCount,         RequestGetTotalDeathCount,          RequestGetTotalDeathCountResponse)

// -----------------------------------------------------------------------------------------------
// Misc
// -----------------------------------------------------------------------------------------------
DEFINE_REQUEST_RESPONSE     (0x0320, RequestSendMessageToPlayers,       RequestSendMessageToPlayers,        RequestSendMessageToPlayersResponse)
DEFINE_PUSH_MESSAGE         (0x0389, ManagementTextMessage,             ManagementTextMessage)

