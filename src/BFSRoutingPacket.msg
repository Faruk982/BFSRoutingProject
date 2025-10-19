//
// A* Routing Packet definition with RSA Encryption
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
    
    // RSA Encryption fields - encrypted routing information
    bool isEncrypted = false;          // Flag indicating if routing info is encrypted
    long encryptedHopCount = 0;        // Encrypted hop count
    long encryptedGCost = 0;           // Encrypted g(n) cost
    long encryptedHCost = 0;           // Encrypted h(n) cost
    long encryptedFCost = 0;           // Encrypted f(n) cost
    
    // RSA Public Key of sender (for secure communication)
    long senderPublicKey = 0;          // Public exponent (e)
    long senderModulus = 0;            // Modulus (n)
};
