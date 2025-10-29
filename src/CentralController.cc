#include "CentralController.h"

Define_Module(CentralController);

void CentralController::initialize() {
    totalNodes = getSystemModule()->par("numNodes");
    topologyComplete = false;
    uniqueEdges.clear();
    registeredNodes.clear();
    
    // Generate RSA keys
    generateRSAKeys();
    
    EV << "Central Controller initialized, waiting for " << totalNodes << " nodes\n";
    EV << "RSA Keys - Public: (e=" << publicKey << ", n=" << modulus << ")\n";
}

void CentralController::handleMessage(cMessage *msg) {
    BFSRoutingPacket *pkt = check_and_cast<BFSRoutingPacket *>(msg);
    
    // Handle link-state registration from routers
    if (strcmp(pkt->getType(), "LINK_STATE") == 0) {
        receiveLinkStateInfo(pkt);
        delete pkt;
        
        // Check if all nodes have registered AND we haven't broadcasted yet
        if (!topologyComplete && registeredNodes.size() == (size_t)totalNodes) {
            EV << "\n*** All nodes registered! Starting centralized path computation ***\n";
            
            // Step 1: Build adjacency list
            buildAdjacencyList();
            
            // Step 2: Compute all paths and build forwarding tables
            computeAllPaths();
            
            // Step 3: Send forwarding tables to routers
            sendForwardingTables();
            
            topologyComplete = true;  // Set flag to prevent re-computation
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
       << ", Unique edges: " << uniqueEdges.size() 
       << ", Total nodes discovered: " << allNodes.size() << "\n";
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
                EV << "  Path " << source << " → " << dest << ": NO PATH FOUND\n";
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
    
    // Heuristic: Simple estimate (can be improved with actual positions)
    // For now, use 0 (makes it Dijkstra)
    double h = 0;
    double f = gCost[source] + h;
    
    pq.push({f, source});
    parent[source] = -1;
    
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
                    
                    double heuristic = 0;  // Simple heuristic for now
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
    EV << "  Sending Forwarding Tables to Routers\n";
    EV << "========================================================\n";
    
    // For each router
    for (int routerId : allNodes) {
        EV << "\nForwarding Table for Node " << routerId << ":\n";
        EV << "  Destination → NextHop\n";
        
        if (forwardingTable.find(routerId) != forwardingTable.end()) {
            for (const auto& entry : forwardingTable[routerId]) {
                int dest = entry.first;
                int nextHop = entry.second;
                
                EV << "  " << dest << " → " << nextHop << "\n";
                
                // Create packet with forwarding entry
                BFSRoutingPacket *pkt = new BFSRoutingPacket();
                pkt->setType("FORWARDING_ENTRY");
                
                // Use existing fields to encode forwarding info
                // hopCount = destination, requestId = nextHop
                pkt->setHopCount(dest);
                pkt->setRequestId(nextHop);
                
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
               << " forwarding entries to Node " << routerId << "\n";
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

// Generate RSA Keys (simple implementation with small primes)
void CentralController::generateRSAKeys() {
    // Use small primes > totalNodes (6 nodes)
    long long p = 11;  // Prime 1
    long long q = 13;  // Prime 2
    
    modulus = p * q;  // n = 143
    long long phi = (p - 1) * (q - 1);  // φ(n) = 120
    
    // Choose e such that 1 < e < φ(n) and gcd(e, φ(n)) = 1
    publicKey = 7;  // Common choice, coprime with 120
    
    // Find d such that (d * e) mod φ(n) = 1
    // Using extended Euclidean algorithm (simplified)
    for (long long d = 1; d < phi; d++) {
        if ((d * publicKey) % phi == 1) {
            privateKey = d;
            break;
        }
    }
    
    EV << "RSA Key Generation Complete:\n";
    EV << "  p = " << p << ", q = " << q << "\n";
    EV << "  n = " << modulus << "\n";
    EV << "  φ(n) = " << phi << "\n";
    EV << "  Public Key (e) = " << publicKey << "\n";
    EV << "  Private Key (d) = " << privateKey << "\n";
}

// Encrypt a message using public key
long long CentralController::rsaEncrypt(long long message) {
    return modPow(message, publicKey, modulus);
}

// Decrypt a ciphertext using private key
long long CentralController::rsaDecrypt(long long ciphertext) {
    return modPow(ciphertext, privateKey, modulus);
}
