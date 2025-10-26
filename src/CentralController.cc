#include "CentralController.h"

Define_Module(CentralController);

void CentralController::initialize() {
    totalNodes = getSystemModule()->par("numNodes");
    topologyComplete = false;
    uniqueEdges.clear();
    registeredNodes.clear();
    
    EV << "Central Controller initialized, waiting for " << totalNodes << " nodes\n";
}

void CentralController::handleMessage(cMessage *msg) {
    BFSRoutingPacket *pkt = check_and_cast<BFSRoutingPacket *>(msg);
    
    // Handle link-state registration from routers
    if (strcmp(pkt->getType(), "LINK_STATE") == 0) {
        receiveLinkStateInfo(pkt);
        delete pkt;
        
        // Check if all nodes have registered AND we haven't broadcasted yet
        if (!topologyComplete && registeredNodes.size() == (size_t)totalNodes) {
            EV << "\n*** All nodes registered! Broadcasting complete topology ***\n";
            broadcastCompleteTopology();
            topologyComplete = true;  // Set flag to prevent re-broadcasting
        }
    }
}

void CentralController::receiveLinkStateInfo(BFSRoutingPacket *pkt) {
    int nodeId = pkt->getSourceAddress();
    registeredNodes.insert(nodeId);
    
    // Parse link state from packet name: "LINK_from_to_delay"
    std::string name = pkt->getName();
    if (name.find("LINK_") == 0) {
        int from, to;
        double delay;
        sscanf(name.c_str(), "LINK_%d_%d_%lf", &from, &to, &delay);
        
        // Create edge with normalized order (smaller ID first)
        EdgeInfo edge;
        edge.node1 = (from < to) ? from : to;
        edge.node2 = (from < to) ? to : from;
        edge.delay = delay;
        
        // Insert into set (automatically handles duplicates)
        uniqueEdges.insert(edge);
        
        EV << "Controller received from Node " << nodeId << ": " 
           << from << " ↔ " << to << " (delay=" << delay << "ms)\n";
    }
    
    EV << "Registered nodes: " << registeredNodes.size() << "/" << totalNodes 
       << ", Unique edges: " << uniqueEdges.size() << "\n";
}

void CentralController::broadcastCompleteTopology() {
    EV << "\n========================================================\n";
    EV << "  Broadcasting Complete Topology to All Nodes\n";
    EV << "  Total unique edges: " << uniqueEdges.size() << "\n";
    EV << "========================================================\n";
    
    // Display all edges
    for (const auto& edge : uniqueEdges) {
        EV << "  Edge: " << edge.node1 << " ↔ " << edge.node2 
           << " (delay=" << edge.delay << "ms)\n";
    }
    
    // Send each edge to ALL routers
    for (int nodeId : registeredNodes) {
        int edgesSent = 0;
        
        // Send each unique edge as TWO directed links (bidirectional)
        for (const auto& edge : uniqueEdges) {
            // Send edge in both directions to form adjacency list
            
            // Direction 1: node1 -> node2
            BFSRoutingPacket *pkt1 = new BFSRoutingPacket("TOPOLOGY");
            pkt1->setType("TOPOLOGY");
            pkt1->setSourceAddress(-1);  // From controller
            pkt1->setDestinationAddress(nodeId);
            
            std::stringstream ss1;
            ss1 << "LINK_" << edge.node1 << "_" << edge.node2 << "_" << edge.delay;
            pkt1->setName(ss1.str().c_str());
            send(pkt1, "toRouter$o", nodeId);
            edgesSent++;
            
            // Direction 2: node2 -> node1
            BFSRoutingPacket *pkt2 = new BFSRoutingPacket("TOPOLOGY");
            pkt2->setType("TOPOLOGY");
            pkt2->setSourceAddress(-1);  // From controller
            pkt2->setDestinationAddress(nodeId);
            
            std::stringstream ss2;
            ss2 << "LINK_" << edge.node2 << "_" << edge.node1 << "_" << edge.delay;
            pkt2->setName(ss2.str().c_str());
            send(pkt2, "toRouter$o", nodeId);
            edgesSent++;
        }
        
        // Send completion signal
        BFSRoutingPacket *completePkt = new BFSRoutingPacket("TOPOLOGY_COMPLETE");
        completePkt->setType("TOPOLOGY");
        completePkt->setSourceAddress(-1);
        completePkt->setDestinationAddress(nodeId);
        completePkt->setName("TOPOLOGY_COMPLETE");
        send(completePkt, "toRouter$o", nodeId);
        
        EV << "  → Sent " << edgesSent << " directed links + completion signal to Node " << nodeId << "\n";
    }
    
    EV << "========================================================\n";
    EV << "  Topology broadcast complete!\n";
    EV << "========================================================\n\n";
}

void CentralController::finish() {
    EV << "\n========================================\n";
    EV << "Central Controller - Final State\n";
    EV << "========================================\n";
    EV << "Total nodes registered: " << registeredNodes.size() << "\n";
    EV << "Total unique edges: " << uniqueEdges.size() << "\n";
    EV << "Total directed links sent: " << (uniqueEdges.size() * 2) << "\n";
    EV << "========================================\n";
}
