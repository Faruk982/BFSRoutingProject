#ifndef CENTRALCONTROLLER_H
#define CENTRALCONTROLLER_H

#include <omnetpp.h>
#include "BFSRoutingPacket_m.h"
#include <map>
#include <vector>
#include <set>

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
    int totalNodes;
    bool topologyComplete;
    
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    
    void receiveLinkStateInfo(BFSRoutingPacket *pkt);
    void broadcastCompleteTopology();
};

#endif
