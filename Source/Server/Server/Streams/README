Just as a quick explanation.

A stream is an object that sends or recieves blocks of data. Streams can be inherited by other streams and
wrap data before passing it to the base class.

We use this to implement the "multiple layers of transport" that DS3 has. The basic structure is:

Frpg2PacketStream:                      Sends and recieves raw packets across a TCP connection.
 - Frpg2MessageStream:                  Sends and recieves messages as packets.

Frpd2UdpPacketStream:                   Sends and recieves raw packets across a UDP connection.
 - Frpg2ReliableUdpPacketStream:        Sends and recieves packets using a simple reliable udp algorithm, packets that come out of here are guaranteed ordered and deduplicated.
  - Frp2ReliableUdpFragmentStream:      Sends and recieves blocks of data as multiple packets which are fragmented on send and defragmented on recieve. Packets send and recieved can be any size, the stream with handle the fragmenting.
   - Frpg2ReliableUdpMessageStream:     Sends and recieves messages, which just wrap a protobuf struct. This is the layer the server will normally act on.