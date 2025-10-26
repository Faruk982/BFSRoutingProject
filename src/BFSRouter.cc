#include "BFSRouter.h"

Define_Module(BFSRouter);

void BFSRouter::initialize() {
    myAddress = par("address");
    numNodes = getSystemModule()->par("numNodes");
    useEncryption = false;
    rsaCrypto = nullptr;
    
    // Initialize routing structures
    routingTable.clear();
    adjacencyList.clear();
    knownNodes.clear();
    receivedTopology.clear();
    topologyComplete = false;
    
    // Initialize statistics
    packetsForwarded = 0;
    packetsDelivered = 0;
    routeDiscoveriesSent = 0;
    astarRoutesLearned = 0;
    
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
    
    knownNodes.insert(myAddress);
    
    EV << "Node " << myAddress << " initialized with " << gateSize("port") << " ports\n";
    
    // PHASE 1: Discover local topology and send to central controller
    scheduleAt(simTime() + 0.1, new cMessage("discoverTopology"));
}

void BFSRouter::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        if (strcmp(msg->getName(), "discoverTopology") == 0) {
            // PHASE 1: Discover local links
            discoverLocalTopology();
            delete msg;
            
            // PHASE 2: Send to central controller (not broadcast!)
            scheduleAt(simTime() + 0.2, new cMessage("sendToController"));
            return;
        }
        
        if (strcmp(msg->getName(), "sendToController") == 0) {
            // Send link state to central controller
            sendLinkStateToController();
            delete msg;
            return;
        }
        
        if (strcmp(msg->getName(), "calculatePaths") == 0) {
            // PHASE 3: Run A* for all destinations (after receiving topology from controller)
            EV << "\n========================================================\n";
            EV << "  Node " << myAddress << " calculating optimal paths using A*\n";
            EV << "========================================================\n";
            calculateAllPaths();
            delete msg;
            
            // PHASE 4: Node 0 sends test packet after all paths calculated
            if (myAddress == 0) {
                EV << "\n*** Node 0 will send test packet in 0.5s ***\n";
                scheduleAt(simTime() + 0.5, new cMessage("sendTest"));
            }
            return;
        }
        
        if (strcmp(msg->getName(), "sendTest") == 0) {
            // Send test packet from 0 to 5
            int destAddr = 5;
            EV << "\n#########################################################\n";
            EV << "# SENDING TEST PACKET: Node " << myAddress << " → Node " << destAddr << "\n";
            EV << "#########################################################\n";
            
            if (routingTable.find(destAddr) != routingTable.end()) {
                RouteInfo &route = routingTable[destAddr];
                EV << "Optimal path found! f(n)=" << route.fCost 
                   << ", hops=" << route.hopCount << "\n";
                EV << "Path: ";
                for (int node : route.path) {
                    EV << node << " → ";
                }
                EV << destAddr << "\n\n";
                
                BFSRoutingPacket *pkt = new BFSRoutingPacket("DATA");
                pkt->setType("DATA");
                pkt->setSourceAddress(myAddress);
                pkt->setDestinationAddress(destAddr);
                pkt->setHopCount(0);
                pkt->setTimestamp(simTime());
                
                route = routingTable[destAddr];
                EV << "Using pre-calculated path via gate " << route.nextHopGate << "\n";
                send(pkt, "port$o", route.nextHopGate);
            }
            delete msg;
            return;
        }
    }
    
    // Handle received packets
    BFSRoutingPacket *pkt = check_and_cast<BFSRoutingPacket *>(msg);
    int srcAddr = pkt->getSourceAddress();
    int destAddr = pkt->getDestinationAddress();
    
    // Handle topology from central controller
    if (strcmp(pkt->getType(), "TOPOLOGY") == 0 && srcAddr == -1) {
        receiveTopologyFromController(pkt);
        delete pkt;
        return;
    }
    
    // Handle DATA packets
    if (strcmp(pkt->getType(), "DATA") == 0) {
        if (destAddr == myAddress) {
            // Packet delivered!
            packetsDelivered++;
            EV << "\n#########################################################\n";
            EV << "# ✓✓✓ PACKET SUCCESSFULLY DELIVERED! ✓✓✓\n";
            EV << "# From: Node " << srcAddr << "\n";
            EV << "# To:   Node " << myAddress << "\n";
            EV << "# Hops: " << pkt->getHopCount() << "\n";
            EV << "#########################################################\n\n";
            delete pkt;
            return;
        } else {
            // Forward using routing table
            if (routingTable.find(destAddr) != routingTable.end()) {
                RouteInfo &route = routingTable[destAddr];
                pkt->setHopCount(pkt->getHopCount() + 1);
                EV << "→ Node " << myAddress << " forwarding to dest=" << destAddr 
                   << " via gate " << route.nextHopGate 
                   << " (hop " << pkt->getHopCount() << ")\n";
                packetsForwarded++;
                send(pkt, "port$o", route.nextHopGate);
            } else {
                EV << "No route to " << destAddr << ". Dropping packet.\n";
                delete pkt;
            }
            return;
        }
    }
    
    delete pkt;
}

double BFSRouter::calculateHeuristic(int currentNode, int destNode) {
    // Use 2D Manhattan distance based on node positions (6 nodes)
    std::map<int, std::pair<int, int>> nodePositions = {
        {0, {200, 100}},
        {1, {600, 100}},
        {2, {200, 300}},
        {3, {600, 300}},
        {4, {200, 500}},
        {5, {600, 500}}
    };
    
    auto currPos = nodePositions[currentNode];
    auto destPos = nodePositions[destNode];
    
    // Manhattan distance: |x1-x2| + |y1-y2|, scaled down
    double manhattanDist = (abs(currPos.first - destPos.first) + 
                           abs(currPos.second - destPos.second)) / 100.0;
    
    return manhattanDist;
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
        astarRoutesLearned++;  // Track route learning
        
        EV << "✓ New route to " << destAddr << " via gate " << gateIndex 
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
            
            EV << "✓ Updated route to " << destAddr << " via gate " << gateIndex 
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

void BFSRouter::encryptRoutingInfo(BFSRoutingPacket *pkt) {
    // Encryption disabled - no action needed
    return;
}

void BFSRouter::decryptRoutingInfo(BFSRoutingPacket *pkt) {
    // Encryption disabled - no action needed
    return;
}

void BFSRouter::storeNeighborPublicKey(int nodeAddress, long long publicKey, long long modulus) {
    // Encryption disabled - no action needed
    return;
}

// PHASE 1: Discover local topology
void BFSRouter::discoverLocalTopology() {
    EV << "Node " << myAddress << " discovering local neighbors...\n";
    
    // Learn about directly connected neighbors and build local adjacency list
    for (int i = 0; i < gateSize("port"); i++) {
        if (gate("port$o", i)->isConnected()) {
            cGate *outGate = gate("port$o", i);
            cGate *remoteGate = outGate->getNextGate();
            int neighborId = remoteGate->getOwnerModule()->par("address");
            
            // Get link delay
            cChannel *channel = outGate->getChannel();
            double delay = 0;
            if (channel) {
                delay = channel->par("delay").doubleValue() * 1000; // convert to ms
            }
            
            // Add to adjacency list
            adjacencyList[myAddress].push_back(Neighbor(neighborId, delay, i));
            knownNodes.insert(neighborId);
            
            EV << "  Neighbor: " << neighborId << " via gate " << i << ", delay=" << delay << "ms\n";
        }
    }
}

// PHASE 2: Send link state to central controller (instead of flooding!)
void BFSRouter::sendLinkStateToController() {
    EV << "Node " << myAddress << " sending link state to controller...\n";
    
    // Send our local links from adjacency list to the central controller
    for (const auto& neighbor : adjacencyList[myAddress]) {
        BFSRoutingPacket *pkt = new BFSRoutingPacket("LINK_STATE");
        pkt->setType("LINK_STATE");
        pkt->setSourceAddress(myAddress);
        pkt->setDestinationAddress(-1);  // To controller
        
        // Encode link info: LINK_from_to_delay
        std::stringstream ss;
        ss << "LINK_" << myAddress << "_" << neighbor.nodeId << "_" << neighbor.delay;
        pkt->setName(ss.str().c_str());
        
        // Send to controller
        send(pkt, "toController$o");
        EV << "  Sent: " << myAddress << " → " << neighbor.nodeId << " (delay=" << neighbor.delay << "ms)\n";
    }
}


// Receive complete topology from central controller
void BFSRouter::receiveTopologyFromController(BFSRoutingPacket *pkt) {
    // Parse: LINK_from_to_delay
    std::string name = pkt->getName();
    if (name.find("LINK_") == 0) {
        int from, to;
        double delay;
        sscanf(name.c_str(), "LINK_%d_%d_%lf", &from, &to, &delay);
        
        // Build adjacency list: adjacency[from].push(to)
        int gateIndex = (from == myAddress) ? findGateToNeighbor(to) : -1;
        adjacencyList[from].push_back(Neighbor(to, delay, gateIndex));
        
        knownNodes.insert(from);
        knownNodes.insert(to);
    }
    
    // Check if this is the last packet (controller sends "COMPLETE" as last message)
    if (!topologyComplete && strcmp(pkt->getName(), "TOPOLOGY_COMPLETE") == 0) {
        topologyComplete = true;
        
        EV << "\n========================================================\n";
        EV << "  Node " << myAddress << " received COMPLETE TOPOLOGY!\n";
        EV << "  Total nodes: " << knownNodes.size() << "\n";
        EV << "  Known nodes: ";
        for (int node : knownNodes) {
            EV << node << " ";
        }
        EV << "\n\n  ADJACENCY LIST:\n";
        for (const auto& entry : adjacencyList) {
            EV << "    Node " << entry.first << " → ";
            for (const auto& neighbor : entry.second) {
                EV << neighbor.nodeId << "(delay=" << neighbor.delay << "ms) ";
            }
            EV << "\n";
        }
        EV << "\n  Ready for A* calculation.\n";
        EV << "========================================================\n\n";
        
        // Schedule path calculation
        scheduleAt(simTime() + 0.1, new cMessage("calculatePaths"));
    }
}

int BFSRouter::findGateToNeighbor(int neighborId) {
    // Find which gate connects to a specific neighbor
    for (int i = 0; i < gateSize("port"); i++) {
        if (gate("port$o", i)->isConnected()) {
            cGate *outGate = gate("port$o", i);
            cGate *remoteGate = outGate->getNextGate();
            int connectedId = remoteGate->getOwnerModule()->par("address");
            if (connectedId == neighborId) {
                return i;
            }
        }
    }
    return -1; // Not directly connected
}

// PHASE 3: Calculate all paths using A*
void BFSRouter::calculateAllPaths() {
    EV << "Running A* for all destinations...\n";
    
    for (int dest : knownNodes) {
        if (dest != myAddress) {
            runAstar(dest);
        }
    }
    
    EV << "A* calculation complete! Routing table built.\n";
}

// A* algorithm implementation
void BFSRouter::runAstar(int destination) {
    // Proper A* algorithm with priority queue
    // priority_queue<pair<fCost, node>>
    typedef std::pair<double, int> PII;  // (fCost, nodeId)
    std::priority_queue<PII, std::vector<PII>, std::greater<PII>> pq;  // Min-heap
    
    std::map<int, double> gScore;  // Actual cost from start
    std::map<int, int> parent;     // Parent node in path
    std::set<int> visited;
    
    // Initialize
    gScore[myAddress] = 0.0;
    double h = calculateHeuristic(myAddress, destination);
    pq.push({h, myAddress});  // Using brace initialization instead of make_pair
    
    while (!pq.empty()) {
        PII top = pq.top();
        pq.pop();
        
        double fCost = top.first;
        int current = top.second;
        
        // Found destination!
        if (current == destination) {
            // Reconstruct path
            std::vector<int> path;
            int node = destination;
            while (parent.find(node) != parent.end()) {
                path.push_back(node);
                node = parent[node];
            }
            path.push_back(myAddress);
            
            // Manual reverse instead of std::reverse
            int left = 0;
            int right = path.size() - 1;
            while (left < right) {
                int temp = path[left];
                path[left] = path[right];
                path[right] = temp;
                left++;
                right--;
            }
            
            // Find next hop (first node after myAddress in path)
            int nextHop = (path.size() > 1) ? path[1] : destination;
            int nextGate = findGateToNeighbor(nextHop);
            
            // Store in routing table
            RouteInfo route;
            route.nextHopGate = nextGate;
            route.gCost = gScore[destination];
            route.hCost = 0;
            route.fCost = gScore[destination];
            route.hopCount = path.size() - 1;
            route.path = path;
            route.timestamp = simTime();
            
            routingTable[destination] = route;
            astarRoutesLearned++;
            
            EV << "    Path to " << destination << ": ";
            for (size_t i = 0; i < path.size(); i++) {
                EV << path[i];
                if (i < path.size() - 1) EV << " → ";
            }
            EV << " (cost=" << gScore[destination] << ")\n";
            return;
        }
        
        if (visited.find(current) != visited.end()) continue;
        visited.insert(current);
        
        // Explore neighbors
        for (const auto& neighbor : adjacencyList[current]) {
            int next = neighbor.nodeId;
            double newGScore = gScore[current] + neighbor.delay;
            
            if (gScore.find(next) == gScore.end() || newGScore < gScore[next]) {
                gScore[next] = newGScore;
                parent[next] = current;
                double h = calculateHeuristic(next, destination);
                double f = newGScore + h;
                pq.push({f, next});  // Using brace initialization instead of make_pair
            }
        }
    }
    
    EV << "    No path found to " << destination << "\n";
}

void BFSRouter::finish() {
    EV << "\n========================================\n";
    EV << "Node " << myAddress << " - FINAL ROUTING TABLE\n";
    EV << "========================================\n";
    EV << "Packets delivered: " << packetsDelivered << "\n";
    EV << "Packets forwarded: " << packetsForwarded << "\n";
    EV << "Total routes: " << routingTable.size() << "\n";
    EV << "Known nodes in network: " << knownNodes.size() << "\n\n";
    
    // Display OPTIMAL PATH to each destination
    EV << "--- OPTIMAL PATHS FROM NODE " << myAddress << " ---\n";
    for (auto& entry : routingTable) {
        int dest = entry.first;
        if (dest == myAddress) continue;
        
        RouteInfo& route = entry.second;
        EV << "To Node " << dest << ": ";
        
        // Show path
        EV << "Path = ";
        for (size_t i = 0; i < route.path.size(); i++) {
            EV << route.path[i];
            if (i < route.path.size() - 1) EV << " -> ";
        }
        
        EV << ", Cost(f) = " << route.fCost;
        EV << " (g=" << route.gCost << ", h=" << route.hCost << ")";
        EV << ", Hops = " << route.hopCount;
        EV << ", Next hop gate = " << route.nextHopGate;
        EV << "\n";
    }
    EV << "========================================\n\n";
}
