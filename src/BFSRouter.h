#include <omnetpp.h>
#include "BFSRoutingPacket_m.h"
#include <map>
#include <set>
#include <queue>

using namespace omnetpp;

class BFSRouter : public cSimpleModule {
  private:
    int myAddress;
    
    // Path memory: stores learned routes
    std::map<int, int> routingTable;  // destination -> next hop gate
    std::map<int, simtime_t> routeTimestamp;  // when the route was learned
    
    // BFS discovery tracking
    std::set<int> processedRequests;  // to avoid processing same request multiple times
    
    // Statistics
    int packetsForwarded;
    int packetsDelivered;
    int routeDiscoveriesSent;
    
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    
    // Helper methods
    void sendRouteDiscovery(int destination);
    void forwardPacket(BFSRoutingPacket *pkt, int outGate);
    void broadcastPacket(BFSRoutingPacket *pkt, int incomingGate);
    int getRequestId(int source, int reqId);
};
