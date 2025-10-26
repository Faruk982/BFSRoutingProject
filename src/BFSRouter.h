#include <omnetpp.h>
#include "BFSRoutingPacket_m.h"
// #include "RSA.h"  // Disabled for now
#include <map>
#include <set>
#include <queue>
#include <vector>
#include <sstream>    // For std::stringstream

using namespace omnetpp;

// Structure to store route information with A* metrics
struct RouteInfo {
    int nextHopGate;           // Which gate to forward to
    double gCost;              // Actual cost to reach destination
    double hCost;              // Heuristic estimate
    double fCost;              // Total f = g + h
    simtime_t timestamp;       // When route was learned
    int hopCount;              // Number of hops
    int useCount;              // How many times this route was used
    double successRate;        // Success rate (0.0 to 1.0)
    std::vector<int> path;     // Full path to destination
    
    RouteInfo() : nextHopGate(-1), gCost(0), hCost(0), fCost(0), 
                  hopCount(0), useCount(0), successRate(1.0) {}
};

// Neighbor information in adjacency list
struct Neighbor {
    int nodeId;
    double delay;
    int gateIndex;  // For local routing
    
    Neighbor(int id, double d, int gate = -1) : nodeId(id), delay(d), gateIndex(gate) {}
};

class BFSRouter : public cSimpleModule {
  private:
    int myAddress;
    
    // Adjacency list representation of the graph
    std::map<int, std::vector<Neighbor>> adjacencyList;  // node -> list of neighbors
    
    // Routing table with optimal paths (calculated by A*)
    std::map<int, RouteInfo> routingTable;  // destination -> route info
    
    bool topologyComplete;  // Have we received all topology info?
    std::set<int> knownNodes;  // All nodes in the network
    
    // Flood control - track which topology messages we've already seen
    std::set<std::string> receivedTopology;  // Track "fromNode_toNode" pairs already seen
    
    // RSA Encryption disabled for now
    // RSA* rsaCrypto;  // Temporarily disabled
    void* rsaCrypto;  // Placeholder
    bool useEncryption;  // Enable/disable encryption
    
    // Store public keys of other nodes (node address -> public key pair)
    std::map<int, std::pair<long long, long long>> neighborPublicKeys;  // address -> (e, n)
    
    // Statistics
    int packetsForwarded;
    int packetsDelivered;
    int routeDiscoveriesSent;
    int astarRoutesLearned;
    int encryptedPacketsSent;
    int encryptedPacketsReceived;
    
    // Network topology for heuristic (simplified - could be learned)
    int numNodes;
    
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    
    // A* specific methods
    double calculateHeuristic(int fromNode, int toNode);
    double calculateGCost(BFSRoutingPacket *pkt);
    void updateRouteInfo(int destination, BFSRoutingPacket *pkt, int inGate);
    RouteInfo* getBestRoute(int destination);
    
    // Link-State Routing with Central Controller
    void discoverLocalTopology();  // Learn about directly connected neighbors
    void sendLinkStateToController();  // Send link state to central controller
    void receiveTopologyFromController(BFSRoutingPacket *pkt);  // Receive complete topology from controller
    void calculateAllPaths();       // Run A* for all destinations
    void runAstar(int destination); // A* algorithm for specific destination
    int findGateToNeighbor(int neighborId);  // Find gate index to specific neighbor
    
    // RSA Encryption methods
    void encryptRoutingInfo(BFSRoutingPacket *pkt);
    void decryptRoutingInfo(BFSRoutingPacket *pkt);
    void storeNeighborPublicKey(int nodeAddress, long long publicKey, long long modulus);
    
    // Helper methods
    void sendRouteDiscovery(int destination);
    void forwardPacket(BFSRoutingPacket *pkt, int outGate);
    void broadcastPacket(BFSRoutingPacket *pkt, int incomingGate);
    int getRequestId(int source, int reqId);
    void addToPath(BFSRoutingPacket *pkt);
};
