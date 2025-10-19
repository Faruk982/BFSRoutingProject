//
// A* Routing Packet definition
//

packet BFSRoutingPacket
{
    string type;
    int sourceAddress;
    int destinationAddress;
    int hopCount = 0;
    int requestId;
    
    // A* algorithm fields
    double gCost = 0.0;        // Actual cost from source (g(n))
    double hCost = 0.0;        // Heuristic cost to destination (h(n))
    double fCost = 0.0;        // Total cost f(n) = g(n) + h(n)
    
    // Path tracking
    int pathLength = 0;        // Number of nodes in path
    int path[10];              // Stores the path taken (node addresses)
    
    // Quality metrics
    double cumulativeDelay = 0.0;  // Total delay accumulated
    simtime_t timestamp;           // When packet was created
};
