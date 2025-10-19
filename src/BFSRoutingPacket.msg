//
// BFS Routing Packet definition
//

packet BFSRoutingPacket
{
    string type;
    int sourceAddress;
    int destinationAddress;
    int hopCount = 0;
    int requestId;
};
