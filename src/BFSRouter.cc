#include "BFSRouter.h"

Define_Module(BFSRouter);

void BFSRouter::initialize() {
    myAddress = par("address");
    packetsForwarded = 0;
    packetsDelivered = 0;
    routeDiscoveriesSent = 0;
    
    EV << "Router " << myAddress << " initialized!" << endl;
    
    // Node 0 will initiate a route discovery to node 3 after 1 second
    if (myAddress == 0) {
        BFSRoutingPacket *pkt = new BFSRoutingPacket("RouteDiscovery");
        pkt->setType("ROUTE_REQUEST");
        pkt->setSourceAddress(myAddress);
        pkt->setDestinationAddress(3);
        pkt->setHopCount(0);
        pkt->setRequestId(1);
        
        scheduleAt(simTime() + 1.0, pkt);
        EV << "Node " << myAddress << " will send route discovery to node 3 at t=1s" << endl;
    }
    
    // Node 1 will send a data packet to node 3 after 2 seconds (will use discovered route)
    if (myAddress == 1) {
        BFSRoutingPacket *pkt = new BFSRoutingPacket("DataPacket");
        pkt->setType("DATA");
        pkt->setSourceAddress(myAddress);
        pkt->setDestinationAddress(3);
        pkt->setHopCount(0);
        pkt->setRequestId(0);
        
        scheduleAt(simTime() + 2.5, pkt);
        EV << "Node " << myAddress << " will send data packet to node 3 at t=2.5s" << endl;
    }
}

void BFSRouter::handleMessage(cMessage *msg) {
    BFSRoutingPacket *pkt = check_and_cast<BFSRoutingPacket *>(msg);
    
    // Handle self-messages (scheduled packets)
    if (msg->isSelfMessage()) {
        EV << "Node " << myAddress << " sending " << pkt->getType() << " to destination " << pkt->getDestinationAddress() << endl;
        
        if (strcmp(pkt->getType(), "ROUTE_REQUEST") == 0) {
            routeDiscoveriesSent++;
            broadcastPacket(pkt, -1);
        } else if (strcmp(pkt->getType(), "DATA") == 0) {
            // Check if we have a route in memory
            if (routingTable.find(pkt->getDestinationAddress()) != routingTable.end()) {
                EV << "Node " << myAddress << " found route in path memory! Using gate " 
                   << routingTable[pkt->getDestinationAddress()] << endl;
                forwardPacket(pkt, routingTable[pkt->getDestinationAddress()]);
            } else {
                EV << "Node " << myAddress << " has no route to " << pkt->getDestinationAddress() 
                   << ", initiating route discovery..." << endl;
                sendRouteDiscovery(pkt->getDestinationAddress());
                delete pkt;
            }
        }
        return;
    }
    
    // Handle received packets
    EV << "Router " << myAddress << " received " << pkt->getType() << " from " 
       << pkt->getSourceAddress() << " to " << pkt->getDestinationAddress() 
       << " (hops: " << pkt->getHopCount() << ")" << endl;
    
    // Check if this packet is for us
    if (pkt->getDestinationAddress() == myAddress) {
        EV << "*** Packet delivered to destination node " << myAddress << " ***" << endl;
        packetsDelivered++;
        
        if (strcmp(pkt->getType(), "ROUTE_REQUEST") == 0) {
            // Send ROUTE_REPLY back
            BFSRoutingPacket *reply = new BFSRoutingPacket("RouteReply");
            reply->setType("ROUTE_REPLY");
            reply->setSourceAddress(myAddress);
            reply->setDestinationAddress(pkt->getSourceAddress());
            reply->setHopCount(0);
            reply->setRequestId(pkt->getRequestId());
            
            // Learn the route back to the original sender
            int inGate = pkt->getArrivalGate()->getIndex();
            routingTable[pkt->getSourceAddress()] = inGate;
            routeTimestamp[pkt->getSourceAddress()] = simTime();
            
            EV << "Sending ROUTE_REPLY back to " << pkt->getSourceAddress() << endl;
            forwardPacket(reply, inGate);
        }
        
        delete pkt;
        return;
    }
    
    // Handle ROUTE_REQUEST - Broadcast
    if (strcmp(pkt->getType(), "ROUTE_REQUEST") == 0) {
        int reqId = getRequestId(pkt->getSourceAddress(), pkt->getRequestId());
        
        if (processedRequests.find(reqId) != processedRequests.end()) {
            EV << "Already processed this request, dropping..." << endl;
            delete pkt;
            return;
        }
        
        processedRequests.insert(reqId);
        
        // Learn route back to source
        int inGate = pkt->getArrivalGate()->getIndex();
        routingTable[pkt->getSourceAddress()] = inGate;
        routeTimestamp[pkt->getSourceAddress()] = simTime();
        
        EV << "Learning route: to reach node " << pkt->getSourceAddress() 
           << " use gate " << inGate << endl;
        
        pkt->setHopCount(pkt->getHopCount() + 1);
        broadcastPacket(pkt, inGate);
        packetsForwarded++;
        return;
    }
    
    // Handle ROUTE_REPLY - Forward back to source
    if (strcmp(pkt->getType(), "ROUTE_REPLY") == 0) {
        // Learn route to the replying node
        int inGate = pkt->getArrivalGate()->getIndex();
        routingTable[pkt->getSourceAddress()] = inGate;
        routeTimestamp[pkt->getSourceAddress()] = simTime();
        
        EV << "Learning route from REPLY: to reach node " << pkt->getSourceAddress() 
           << " use gate " << inGate << endl;
        
        if (routingTable.find(pkt->getDestinationAddress()) != routingTable.end()) {
            pkt->setHopCount(pkt->getHopCount() + 1);
            forwardPacket(pkt, routingTable[pkt->getDestinationAddress()]);
            packetsForwarded++;
        } else {
            EV << "ERROR: No route back to " << pkt->getDestinationAddress() << endl;
            delete pkt;
        }
        return;
    }
    
    // Handle DATA packets - Use routing table
    if (strcmp(pkt->getType(), "DATA") == 0) {
        if (routingTable.find(pkt->getDestinationAddress()) != routingTable.end()) {
            EV << "Forwarding DATA using cached route via gate " 
               << routingTable[pkt->getDestinationAddress()] << endl;
            pkt->setHopCount(pkt->getHopCount() + 1);
            forwardPacket(pkt, routingTable[pkt->getDestinationAddress()]);
            packetsForwarded++;
        } else {
            EV << "No route to " << pkt->getDestinationAddress() << ", dropping packet" << endl;
            delete pkt;
        }
        return;
    }
    
    delete pkt;
}

void BFSRouter::sendRouteDiscovery(int destination) {
    BFSRoutingPacket *pkt = new BFSRoutingPacket("RouteDiscovery");
    pkt->setType("ROUTE_REQUEST");
    pkt->setSourceAddress(myAddress);
    pkt->setDestinationAddress(destination);
    pkt->setHopCount(0);
    pkt->setRequestId(routeDiscoveriesSent + 1);
    
    routeDiscoveriesSent++;
    broadcastPacket(pkt, -1);
}

void BFSRouter::forwardPacket(BFSRoutingPacket *pkt, int outGate) {
    // Check if the gate is connected before sending
    cGate *gate = this->gate("port$o", outGate);
    if (gate && gate->isConnected()) {
        send(pkt, "port$o", outGate);
    } else {
        EV << "Warning: Gate port$o[" << outGate << "] is not connected, dropping packet" << endl;
        delete pkt;
    }
}

void BFSRouter::broadcastPacket(BFSRoutingPacket *pkt, int incomingGate) {
    int numGates = gateSize("port");
    EV << "Broadcasting to " << numGates << " gates (except gate " << incomingGate << ")" << endl;
    
    int sentCount = 0;
    for (int i = 0; i < numGates; i++) {
        if (i != incomingGate) {
            cGate *gate = this->gate("port$o", i);
            if (gate && gate->isConnected()) {
                BFSRoutingPacket *copy = pkt->dup();
                send(copy, "port$o", i);
                sentCount++;
            }
        }
    }
    
    EV << "Broadcast sent to " << sentCount << " neighbors" << endl;
    delete pkt;
}

int BFSRouter::getRequestId(int source, int reqId) {
    return source * 10000 + reqId;
}
