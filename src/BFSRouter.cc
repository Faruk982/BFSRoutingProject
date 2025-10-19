#include "BFSRouter.h"

Define_Module(BFSRouter);

void BFSRouter::initialize() {
    myAddress = par("address");
    numNodes = getSystemModule()->par("numNodes");
    
    // Initialize routing table
    routingTable.clear();
    processedRequests.clear();
    
    // Initialize self route
    RouteInfo selfRoute;
    selfRoute.nextHopGate = -1;
    selfRoute.gCost = 0.0;
    selfRoute.hCost = 0.0;
    selfRoute.fCost = 0.0;
    selfRoute.hopCount = 0;
    selfRoute.useCount = 0;
    selfRoute.successRate = 1.0;
    selfRoute.timestamp = simTime();
    selfRoute.path.push_back(myAddress);
    routingTable[myAddress] = selfRoute;
    
    EV << "Node " << myAddress << " initialized with " << gateSize("port") << " ports\n";
    
    // Schedule test packets
    if (myAddress == 0) {
        scheduleAt(simTime() + uniform(1, 2), new cMessage("sendTest"));
    }
}

void BFSRouter::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        // Send test packet
        BFSRoutingPacket *pkt = new BFSRoutingPacket("testPacket");
        pkt->setSourceAddress(myAddress);
        pkt->setDestinationAddress(3);
        pkt->setHopCount(0);
        pkt->setGCost(0.0);
        pkt->setHCost(calculateHeuristic(myAddress, 3));
        pkt->setFCost(pkt->getGCost() + pkt->getHCost());
        pkt->setTimestamp(simTime());
        pkt->setPathLength(1);
        pkt->setPath(0, myAddress);
        
        EV << "Node " << myAddress << " sending test packet to " << pkt->getDestinationAddress() << "\n";
        
        // Broadcast to all neighbors
        for (int i = 0; i < gateSize("port"); i++) {
            if (gate("port$o", i)->isConnected()) {
                send(pkt->dup(), "port$o", i);
            }
        }
        delete pkt;
        delete msg;
        return;
    }
    
    BFSRoutingPacket *pkt = check_and_cast<BFSRoutingPacket *>(msg);
    int srcAddr = pkt->getSourceAddress();
    int destAddr = pkt->getDestinationAddress();
    int arrivalGate = pkt->getArrivalGate()->getIndex();
    
    EV << "Node " << myAddress << " received packet from " << srcAddr 
       << " to " << destAddr << " via gate " << arrivalGate << "\n";
    
    // Check if this is the destination
    if (destAddr == myAddress) {
        EV << "Node " << myAddress << " is destination. Packet delivered!\n";
        EV << "Path taken: ";
        for (int i = 0; i < pkt->getPathLength(); i++) {
            EV << pkt->getPath(i);
            if (i < pkt->getPathLength() - 1) EV << " -> ";
        }
        EV << "\n";
        EV << "Total hops: " << pkt->getHopCount() << "\n";
        EV << "g(n) cost: " << pkt->getGCost() << "\n";
        EV << "h(n) cost: " << pkt->getHCost() << "\n";
        EV << "f(n) cost: " << pkt->getFCost() << "\n";
        delete pkt;
        return;
    }
    
    // Check for loops
    bool loopDetected = false;
    for (int i = 0; i < pkt->getPathLength(); i++) {
        if (pkt->getPath(i) == myAddress) {
            loopDetected = true;
            break;
        }
    }
    
    if (loopDetected) {
        EV << "Loop detected at node " << myAddress << ". Dropping packet.\n";
        delete pkt;
        return;
    }
    
    // Update path
    if (pkt->getPathLength() < 10) {
        pkt->setPath(pkt->getPathLength(), myAddress);
        pkt->setPathLength(pkt->getPathLength() + 1);
    }
    
    // Update hop count
    pkt->setHopCount(pkt->getHopCount() + 1);
    
    // Calculate costs using A* algorithm
    double gCost = calculateGCost(pkt);
    double hCost = calculateHeuristic(myAddress, destAddr);
    double fCost = gCost + hCost;
    
    pkt->setGCost(gCost);
    pkt->setHCost(hCost);
    pkt->setFCost(fCost);
    
    EV << "A* costs at node " << myAddress << ": g(n)=" << gCost 
       << ", h(n)=" << hCost << ", f(n)=" << fCost << "\n";
    
    // Update routing table with A* information
    updateRouteInfo(srcAddr, pkt, arrivalGate);
    
    // Check if we have a route to destination
    if (routingTable.find(destAddr) != routingTable.end()) {
        RouteInfo &route = routingTable[destAddr];
        
        // Only forward if this path is better (lower f-cost)
        if (fCost < route.fCost || route.nextHopGate == -1) {
            EV << "Found better route to " << destAddr << " via gate " << route.nextHopGate 
               << " with f(n)=" << route.fCost << "\n";
            
            route.useCount++;
            route.timestamp = simTime();
            
            send(pkt, "port$o", route.nextHopGate);
        } else {
            EV << "Current route not optimal. Dropping packet.\n";
            delete pkt;
        }
    } else {
        // No route known - broadcast to all except arrival gate
        EV << "No route to " << destAddr << ". Broadcasting...\n";
        
        bool forwarded = false;
        for (int i = 0; i < gateSize("port"); i++) {
            if (i != arrivalGate && gate("port$o", i)->isConnected()) {
                send(pkt->dup(), "port$o", i);
                forwarded = true;
            }
        }
        
        if (forwarded) {
            delete pkt;
        } else {
            EV << "No available gates to forward. Dropping packet.\n";
            delete pkt;
        }
    }
}

double BFSRouter::calculateHeuristic(int currentNode, int destNode) {
    // Manhattan distance heuristic: |dest - current| * 1.0
    return abs(destNode - currentNode) * 1.0;
}

double BFSRouter::calculateGCost(BFSRoutingPacket *pkt) {
    // Actual cost: hopCount * 1.0 + delay * 0.1
    simtime_t delay = simTime() - pkt->getTimestamp();
    return (pkt->getHopCount() * 1.0) + (delay.dbl() * 0.1);
}

void BFSRouter::updateRouteInfo(int destAddr, BFSRoutingPacket *pkt, int gateIndex) {
    double gCost = calculateGCost(pkt);
    double hCost = calculateHeuristic(myAddress, destAddr);
    double fCost = gCost + hCost;
    if (routingTable.find(destAddr) == routingTable.end()) {
        // New route
        RouteInfo newRoute;
        newRoute.nextHopGate = gateIndex;
        newRoute.gCost = gCost;
        newRoute.hCost = hCost;
        newRoute.fCost = fCost;
        newRoute.hopCount = pkt->getHopCount();
        newRoute.useCount = 1;
        newRoute.successRate = 1.0;
        newRoute.timestamp = simTime();
        
        for (int i = 0; i < pkt->getPathLength() && i < 10; i++) {
            newRoute.path.push_back(pkt->getPath(i));
        }
        
        routingTable[destAddr] = newRoute;
        
        EV << "New route to " << destAddr << " via gate " << gateIndex 
           << " with f(n)=" << fCost << "\n";
    } else {
        // Update existing route only if better (lower f-cost)
        RouteInfo &existingRoute = routingTable[destAddr];
        
        if (fCost < existingRoute.fCost) {
            existingRoute.nextHopGate = gateIndex;
            existingRoute.gCost = gCost;
            existingRoute.hCost = hCost;
            existingRoute.fCost = fCost;
            existingRoute.hopCount = pkt->getHopCount();
            existingRoute.timestamp = simTime();
            
            existingRoute.path.clear();
            for (int i = 0; i < pkt->getPathLength() && i < 10; i++) {
                existingRoute.path.push_back(pkt->getPath(i));
            }
            
            EV << "Updated route to " << destAddr << " via gate " << gateIndex 
               << " with better f(n)=" << fCost << " (was " << existingRoute.fCost << ")\n";
        }
    }
}

RouteInfo* BFSRouter::getBestRoute(int destAddr) {
    if (routingTable.find(destAddr) != routingTable.end()) {
        return &routingTable[destAddr];
    }
    return nullptr;
}

void BFSRouter::finish() {
    EV << "Node " << myAddress << " finishing simulation\n";
    EV << "Routes learned: " << routingTable.size() << "\n";
}
