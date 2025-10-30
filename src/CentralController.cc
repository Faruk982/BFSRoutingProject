#include "CentralController.h"

Define_Module(CentralController);

void CentralController::initialize() {
    totalNodes = getSystemModule()->par("numNodes");
    topologyComplete = false;
    uniqueEdges.clear();
    registeredNodes.clear();
    routerPublicKeys.clear();
    
    // Initialize node positions from NED file topology
    initializeNodePositions();
    
    EV << "Central Controller initialized, waiting for " << totalNodes << " nodes\n";
    EV << "Using Manhattan distance heuristic for A* algorithm\n";
}

void CentralController::handleMessage(cMessage *msg) {
    BFSRoutingPacket *pkt = check_and_cast<BFSRoutingPacket *>(msg);
    
    // Handle RSA public key from routers
    if (strcmp(pkt->getType(), "RSA_PUBLIC_KEY") == 0) {
        receiveRouterPublicKey(pkt);
        delete pkt;
        return;
    }
    
    // Handle link-state registration from routers
    if (strcmp(pkt->getType(), "LINK_STATE") == 0) {
        // IGNORE link state messages if we've already computed topology
        if (topologyComplete) {
            EV << "Controller: Ignoring duplicate LINK_STATE from Node " 
               << pkt->getSourceAddress() << " (topology already complete)\n";
            delete pkt;
            return;
        }
        
        receiveLinkStateInfo(pkt);
        delete pkt;
        
        // Check if all nodes have registered AND we have all public keys AND we haven't processed yet
        if (!topologyComplete && registeredNodes.size() == (size_t)totalNodes && 
            routerPublicKeys.size() == (size_t)totalNodes) {
            EV << "\n*** All nodes registered with public keys! Starting centralized path computation ***\n";
            
            // Step 1: Build adjacency list
            buildAdjacencyList();
            
            // Step 2: Compute all paths and build forwarding tables
            computeAllPaths();
            
            // Step 3: Send forwarding tables to routers (encrypted with each router's public key)
            sendForwardingTables();
            
            topologyComplete = true;  // Set flag to prevent re-computation
            
            EV << "\n*** Topology processing complete! Ignoring future LINK_STATE messages ***\n";
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
        
        // Add both nodes to allNodes set
        allNodes.insert(from);
        allNodes.insert(to);
        
        EV << "Controller received from Node " << nodeId << ": " 
           << from << " ↔ " << to << " (delay=" << delay << "ms)\n";
    }
    
    EV << "Registered nodes: " << registeredNodes.size() << "/" << totalNodes 
       << ", Public keys: " << routerPublicKeys.size() << "/" << totalNodes
       << ", Unique edges: " << uniqueEdges.size() 
       << ", Total nodes discovered: " << allNodes.size() << "\n";
}

void CentralController::receiveRouterPublicKey(BFSRoutingPacket *pkt) {
    int routerId = pkt->getSourceAddress();
    long long e = pkt->getHopCount();   // Public exponent
    long long n = pkt->getRequestId();  // Modulus
    
    routerPublicKeys[routerId] = {e, n};
    
    EV << "\n========================================================\n";
    EV << "  Controller received RSA Public Key from Router " << routerId << "\n";
    EV << "========================================================\n";
    EV << "  Public Key (e): " << e << "\n";
    EV << "  Modulus (n): " << n << "\n";
    EV << "  Will use this key to encrypt forwarding table for Router " << routerId << "\n";
    EV << "========================================================\n\n";
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

// Initialize node positions based on NED file topology
void CentralController::initializeNodePositions() {
    // Node positions from BFSRouting.ned file
    // These correspond to the @display("p=x,y") parameters
    // Spread out more for better visualization
    nodePositions[0] = {150, 100};   // node0 - top left
    nodePositions[1] = {700, 100};   // node1 - top center
    nodePositions[2] = {150, 400};   // node2 - middle left
    nodePositions[3] = {700, 400};   // node3 - middle center
    nodePositions[4] = {150, 700};   // node4 - bottom left
    nodePositions[5] = {700, 700};   // node5 - bottom center
    nodePositions[6] = {1200, 200};  // node6 - top right
    nodePositions[7] = {1200, 500};  // node7 - middle right
    nodePositions[8] = {1200, 800};  // node8 - bottom right
    
    EV << "Node positions initialized for Manhattan distance heuristic:\n";
    for (const auto& entry : nodePositions) {
        EV << "  Node " << entry.first << ": (" 
           << entry.second.first << ", " << entry.second.second << ")\n";
    }
}

// Calculate Manhattan distance between two nodes (heuristic for A*)
double CentralController::calculateManhattanDistance(int node1, int node2) {
    if (nodePositions.find(node1) == nodePositions.end() || 
        nodePositions.find(node2) == nodePositions.end()) {
        return 0;  // Return 0 if positions not found (fallback to Dijkstra)
    }
    
    int x1 = nodePositions[node1].first;
    int y1 = nodePositions[node1].second;
    int x2 = nodePositions[node2].first;
    int y2 = nodePositions[node2].second;
    
    // Manhattan distance = |x1 - x2| + |y1 - y2|
    int dx = (x1 > x2) ? (x1 - x2) : (x2 - x1);  // abs(x1 - x2)
    int dy = (y1 > y2) ? (y1 - y2) : (y2 - y1);  // abs(y1 - y2)
    
    // Normalize to match delay scale (divide by 100 to get reasonable values)
    return (dx + dy) / 10.0;  // Scale factor to match delay magnitudes
}

// Build adjacency list from collected edges
void CentralController::buildAdjacencyList() {
    EV << "\n========================================================\n";
    EV << "  Building Adjacency List from Edges\n";
    EV << "========================================================\n";
    
    adjacencyList.clear();
    
    // Traverse all edges and build adjacency list
    for (const auto& edge : uniqueEdges) {
        int u = edge.node1;
        int v = edge.node2;
        double delay = edge.delay;
        
        // Add edge u -> v
        adjacencyList[u].push_back({v, delay});
        // Add edge v -> u (undirected graph)
        adjacencyList[v].push_back({u, delay});
        
        EV << "  Added: " << u << " ↔ " << v << " (delay=" << delay << "ms)\n";
    }
    
    // Display adjacency list
    EV << "\nAdjacency List:\n";
    for (const auto& node : allNodes) {
        EV << "  Node " << node << ": ";
        if (adjacencyList.find(node) != adjacencyList.end()) {
            for (size_t i = 0; i < adjacencyList[node].size(); i++) {
                if (i > 0) EV << ", ";
                EV << adjacencyList[node][i].first 
                   << "(delay=" << adjacencyList[node][i].second << "ms)";
            }
        }
        EV << "\n";
    }
    EV << "========================================================\n\n";
}

// Compute all shortest paths using A* and build forwarding tables
void CentralController::computeAllPaths() {
    EV << "\n========================================================\n";
    EV << "  Computing All-Pairs Shortest Paths (A* Algorithm)\n";
    EV << "========================================================\n";
    
    forwardingTable.clear();
    
    int pathsComputed = 0;
    
    // For each source node
    for (int source : allNodes) {
        // For each destination node
        for (int dest : allNodes) {
            if (source == dest) continue;  // Skip self
            
            // Run A* to find path from source to dest
            std::vector<int> path = runAstarFromController(source, dest);
            
            if (path.size() >= 2) {
                // First hop is the next node after source
                int nextHop = path[1];
                forwardingTable[source][dest] = nextHop;
                
                EV << "  Path " << source << " → " << dest << ": ";
                for (size_t i = 0; i < path.size(); i++) {
                    if (i > 0) EV << " → ";
                    EV << path[i];
                }
                EV << " (NextHop: " << nextHop << ")\n";
                
                pathsComputed++;
            } else {
                // No path exists - mark as unreachable
                forwardingTable[source][dest] = -1;  // -1 means unreachable
                EV << "  Path " << source << " → " << dest << ": ❌ UNREACHABLE (no router-level path)\n";
            }
        }
    }
    
    EV << "\nTotal paths computed: " << pathsComputed << "\n";
    EV << "========================================================\n\n";
}

// A* algorithm implementation in controller
std::vector<int> CentralController::runAstarFromController(int source, int destination) {
    // Priority queue: pair<f_cost, nodeId>
    typedef std::pair<double, int> PII;
    std::priority_queue<PII, std::vector<PII>, std::greater<PII>> pq;
    
    std::map<int, double> gCost;  // Actual cost from source
    std::map<int, int> parent;    // Parent node in path
    std::set<int> visited;
    
    // Initialize
    for (int node : allNodes) {
        gCost[node] = 1e9;  // Infinity
    }
    gCost[source] = 0;
    
    // Heuristic: Manhattan distance from source to destination
    double h = calculateManhattanDistance(source, destination);
    double f = gCost[source] + h;
    
    pq.push({f, source});
    parent[source] = -1;
    
    EV << "  A* from " << source << " to " << destination 
       << " (initial heuristic: " << h << ")\n";
    
    while (!pq.empty()) {
        int current = pq.top().second;
        pq.pop();
        
        if (visited.find(current) != visited.end()) continue;
        visited.insert(current);
        
        // Found destination
        if (current == destination) {
            // Reconstruct path
            std::vector<int> path;
            int node = destination;
            while (node != -1) {
                path.push_back(node);
                node = parent[node];
            }
            
            // Reverse path (manual reversal)
            int left = 0, right = path.size() - 1;
            while (left < right) {
                int temp = path[left];
                path[left] = path[right];
                path[right] = temp;
                left++;
                right--;
            }
            
            return path;
        }
        
        // Explore neighbors
        if (adjacencyList.find(current) != adjacencyList.end()) {
            for (const auto& neighbor : adjacencyList[current]) {
                int nextNode = neighbor.first;
                double edgeDelay = neighbor.second;
                
                double tentativeG = gCost[current] + edgeDelay;
                
                if (tentativeG < gCost[nextNode]) {
                    gCost[nextNode] = tentativeG;
                    parent[nextNode] = current;
                    
                    // Use Manhattan distance as heuristic
                    double heuristic = calculateManhattanDistance(nextNode, destination);
                    double fCost = tentativeG + heuristic;
                    
                    pq.push({fCost, nextNode});
                }
            }
        }
    }
    
    // No path found
    return std::vector<int>();
}

// Send forwarding tables to routers
void CentralController::sendForwardingTables() {
    EV << "\n========================================================\n";
    EV << "  Sending Forwarding Tables to Routers (WITH RSA ENCRYPTION)\n";
    EV << "  Each router's table encrypted with THAT router's public key\n";
    EV << "========================================================\n";
    
    // For each router
    for (int routerId : allNodes) {
        EV << "\nForwarding Table for Node " << routerId << ":\n";
        
        // Get this router's public key
        if (routerPublicKeys.find(routerId) == routerPublicKeys.end()) {
            EV << "  ERROR: No public key for Router " << routerId << "!\n";
            continue;
        }
        
        long long routerE = routerPublicKeys[routerId].first;
        long long routerN = routerPublicKeys[routerId].second;
        EV << "  Using Router " << routerId << "'s public key: e=" << routerE << ", n=" << routerN << "\n";
        EV << "  Destination → NextHop (Plain → Encrypted)\n";
        
        if (forwardingTable.find(routerId) != forwardingTable.end()) {
            for (const auto& entry : forwardingTable[routerId]) {
                int dest = entry.first;
                int nextHop = entry.second;
                
                // ENCRYPT using THIS ROUTER'S public key (e, n)
                long long encryptedDest = rsaEncrypt(dest, routerE, routerN);
                long long encryptedNextHop = rsaEncrypt(nextHop, routerE, routerN);
                
                EV << "  " << dest << " → " << nextHop 
                   << " (Encrypted: " << encryptedDest << " → " << encryptedNextHop << ")\n";
                
                // Create packet with forwarding entry
                BFSRoutingPacket *pkt = new BFSRoutingPacket();
                pkt->setType("FORWARDING_ENTRY");
                
                // Send ENCRYPTED node IDs (encrypted with router's public key)
                // hopCount = encrypted destination, requestId = encrypted nextHop
                pkt->setHopCount(encryptedDest);
                pkt->setRequestId(encryptedNextHop);
                
                pkt->setSourceAddress(-1);  // From controller
                pkt->setDestinationAddress(routerId);
                pkt->setName("ForwardingEntry");
                
                send(pkt, "toRouter$o", routerId);
            }
            
            // Send completion signal
            BFSRoutingPacket *completePkt = new BFSRoutingPacket();
            completePkt->setType("FORWARDING_COMPLETE");
            completePkt->setSourceAddress(-1);
            completePkt->setDestinationAddress(routerId);
            completePkt->setName("ForwardingComplete");
            send(completePkt, "toRouter$o", routerId);
            
            EV << "  → Sent " << forwardingTable[routerId].size() 
               << " encrypted forwarding entries to Node " << routerId << "\n";
        }
    }
    
    EV << "========================================================\n";
    EV << "  All forwarding tables sent!\n";
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

// RSA Helper: Compute GCD
long long CentralController::gcd(long long a, long long b) {
    while (b != 0) {
        long long temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// RSA Helper: Modular exponentiation (base^exp mod mod)
long long CentralController::modPow(long long base, long long exp, long long mod) {
    long long result = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) {
            result = (result * base) % mod;
        }
        exp = exp / 2;
        base = (base * base) % mod;
    }
    return result;
}

// Encrypt a message using a specific router's public key
long long CentralController::rsaEncrypt(long long message, long long e, long long n) {
    // Encryption: c = m^e mod n
    return modPow(message, e, n);
}

