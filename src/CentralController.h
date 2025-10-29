#ifndef CENTRALCONTROLLER_H
#define CENTRALCONTROLLER_H

#include <omnetpp.h>
#include "BFSRoutingPacket_m.h"
#include <map>
#include <vector>
#include <set>
#include <queue>

using namespace omnetpp;

// Edge information (unique edge between two nodes)
struct EdgeInfo {
    int node1;  // Lower node ID
    int node2;  // Higher node ID
    double delay;
    
    // For set comparison
    bool operator<(const EdgeInfo& other) const {
        if (node1 != other.node1) return node1 < other.node1;
        return node2 < other.node2;
    }
};

class CentralController : public cSimpleModule {
  private:
    // Unique edges in the network (undirected)
    std::set<EdgeInfo> uniqueEdges;
    
    // Track which nodes have sent their link state
    std::set<int> registeredNodes;
    
    // All nodes in the network
    std::set<int> allNodes;
    
    int totalNodes;
    bool topologyComplete;
    
    // Adjacency list: adj[nodeId] = vector<pair<neighbor, delay>>
    std::map<int, std::vector<std::pair<int, double>>> adjacencyList;
    
    // Forwarding table: forwarding[source][destination] = nextHop
    std::map<int, std::map<int, int>> forwardingTable;
    
    // Router public keys: routerPublicKeys[routerId] = (e, n)
    std::map<int, std::pair<long long, long long>> routerPublicKeys;
    
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    
    void receiveLinkStateInfo(BFSRoutingPacket *pkt);
    void receiveRouterPublicKey(BFSRoutingPacket *pkt);  // NEW: Receive router's public key
    void broadcastCompleteTopology();
    
    // Centralized path computation
    void buildAdjacencyList();
    void computeAllPaths();
    std::vector<int> runAstarFromController(int source, int destination);
    void sendForwardingTables();
    
    // RSA functions for encryption
    long long rsaEncrypt(long long message, long long e, long long n);  // Encrypt with router's public key
    long long modPow(long long base, long long exp, long long mod);
    long long gcd(long long a, long long b);
};

#endif
