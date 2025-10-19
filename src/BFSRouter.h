#include <omnetpp.h>
#include "BFSRoutingPacket_m.h"
#include "RSA.h"
#include <map>
#include <set>
#include <queue>
#include <vector>

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

class BFSRouter : public cSimpleModule {
  private:
    int myAddress;
    
    // A* Path memory: stores learned routes with cost information
    std::map<int, RouteInfo> routingTable;  // destination -> route info
    
    // For A* discovery - track best paths found so far
    std::map<int, std::map<int, double>> bestGCosts;  // [destination][node] -> best g cost
    
    // BFS/A* discovery tracking
    std::set<int> processedRequests;  // to avoid processing same request multiple times
    
    // RSA Encryption for secure routing
    RSA* rsaCrypto;  // RSA instance for this router
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
