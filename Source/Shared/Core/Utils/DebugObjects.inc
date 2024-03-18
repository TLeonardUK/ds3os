/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */
 
TIMER(AllServerUpdateTime, "All Server Update Time")
TIMER(UpdateTime, "Update Time")
TIMER(WebUIService_PollTime, "Web UI Service (Poll Time)")
TIMER(GameService_PollTime, "Game Service (Poll Time)")
TIMER(AuthService_PollTime, "Auth Service (Poll Time)")
TIMER(LoginService_PollTime, "Login Service (Poll Time)")
TIMER(DatabaseQueryTime, "Database Query Time")
TIMER(AntiCheatTime, "Anti-Cheat Time")

COUNTER(DatabaseQueries, "Database Queries")

COUNTER(AuthConnections, "Auth Connections")
COUNTER(LoginConnections, "Login Connections")
COUNTER(GameConnections, "Game Connections")

COUNTER(TcpBytesRecieved, "TCP Bytes Recieved")
COUNTER(TcpBytesSent, "TCP Bytes Sent")
COUNTER(UdpBytesRecieved, "UDP Bytes Recieved")
COUNTER(UdpBytesSent, "UDP Bytes Sent")

COUNTER(RequestsRecieved, "Requests Recieved")
COUNTER(ResponsesSent, "Responses Sent")
COUNTER(PushMessagesSent, "Push Messages Sent")
