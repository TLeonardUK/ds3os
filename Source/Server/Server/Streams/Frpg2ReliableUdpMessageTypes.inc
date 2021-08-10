// Dark Souls 3 - Open Server

// Macro fun!
// This defines the different types of messages that can be sent/recieved from the dark souls server.
//
// There are 2 macros you can use, one for messages that expect a reply, and one for one way messages.
// 
// DEFINE_REQUEST_RESPONSE(uint32_t OpCode, Frpg2ReliableUdpMessageType Type, Frpg2RequestMessage::* Class, Frpg2RequestMessage::* ResponseClass)
// DEFINE_MESSAGE(uint32_t OpCode, Frpg2ReliableUdpMessageType Type, Frpg2RequestMessage::* Class)

DEFINE_REQUEST_RESPONSE(0x0386, RequestWaitForUserLogin, RequestWaitForUserLogin, RequestWaitForUserLoginResponse)
DEFINE_REQUEST_RESPONSE(0x03C6, RequestGetAnnounceMessageList, RequestGetAnnounceMessageList, RequestGetAnnounceMessageListResponse)

DEFINE_MESSAGE(0x0320, PlayerInfoUploadConfigPushMessage, PlayerInfoUploadConfigPushMessage)


