#include "BFSRouter.h"

Define_Module(BFSRouter);

void BFSRouter::initialize() {
    myAddress = par("address");
    
    // Initialize forwarding table
    forwardingTable.clear();
    forwardingTableReady = false;
    directNeighbors.clear();
    linkStateSent = false;
    
    // Generate this router's RSA key pair
    generateRSAKeys();
    EV << "Node " << myAddress << " generated RSA keys: e=" << publicKey 
       << ", n=" << modulus << ", d=" << privateKey << " [SECRET]\n";
    
    // Initialize statistics
    packetsForwarded = 0;
    packetsDelivered = 0;
    
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
            return;
        }
        
        if (strcmp(msg->getName(), "sendToController") == 0) {
            // Send link state to central controller (ONLY ONCE)
            if (!linkStateSent) {
                sendLinkStateToController();
                linkStateSent = true;
            } else {
                EV << "Node " << myAddress << " already sent link state, ignoring duplicate trigger\n";
            }
            delete msg;
            return;
        }
        
        if (strcmp(msg->getName(), "sendTestWithForwarding") == 0) {
            // Send test packet from 0 to 8 (UNREACHABLE - different partition!)
            int destAddr = 8;
            EV << "\n#########################################################\n";
            EV << "# SENDING TEST PACKET (FORWARDING TABLE): Node " << myAddress << " → Node " << destAddr << "\n";
            EV << "# (Testing unreachable destination in different partition)\n";
            EV << "#########################################################\n";
            
            if (forwardingTable.find(destAddr) != forwardingTable.end()) {
                int nextHop = forwardingTable[destAddr];
                
                if (nextHop == -1) {
                    // Destination is unreachable
                    EV << "\n╔════════════════════════════════════════════════════════╗\n";
                    EV << "║  ❌ DESTINATION UNREACHABLE ❌                        ║\n";
                    EV << "╠════════════════════════════════════════════════════════╣\n";
                    EV << "║  Source:      Node " << myAddress << "                                     ║\n";
                    EV << "║  Destination: Node " << destAddr << "                                     ║\n";
                    EV << "║  Reason: No router-level path exists                 ║\n";
                    EV << "║          (Network partitions are disconnected)       ║\n";
                    EV << "╚════════════════════════════════════════════════════════╝\n\n";
                } else {
                    int nextGate = findGateToNeighbor(nextHop);
                    
                    EV << "Forwarding table says: To reach " << destAddr << ", send to next hop " << nextHop << "\n";
                    EV << "Using gate " << nextGate << " to reach neighbor " << nextHop << "\n\n";
                    
                    BFSRoutingPacket *pkt = new BFSRoutingPacket("DATA");
                    pkt->setType("DATA");
                    pkt->setSourceAddress(myAddress);
                    pkt->setDestinationAddress(destAddr);
                    pkt->setHopCount(0);
                    pkt->setTimestamp(simTime());
                    
                    send(pkt, "port$o", nextGate);
                }
            } else {
                EV << "ERROR: No forwarding entry for destination " << destAddr << "\n";
            }
            delete msg;
            return;
        }
    }
    
    // Handle received packets
    BFSRoutingPacket *pkt = check_and_cast<BFSRoutingPacket *>(msg);
    int srcAddr = pkt->getSourceAddress();
    int destAddr = pkt->getDestinationAddress();
    
    // Handle forwarding table entries from central controller
    if (strcmp(pkt->getType(), "FORWARDING_ENTRY") == 0 && srcAddr == -1) {
        receiveForwardingTable(pkt);
        delete pkt;
        return;
    }
    
    // Handle forwarding table completion signal
    if (strcmp(pkt->getType(), "FORWARDING_COMPLETE") == 0 && srcAddr == -1) {
        forwardingTableReady = true;
        EV << "\n*** Node " << myAddress << " received complete forwarding table! ***\n";
        displayForwardingTable();
        
        // PHASE 4: Node 0 sends test packet after receiving forwarding table
        if (myAddress == 0) {
            EV << "\n*** Node 0 will send test packet in 0.5s ***\n";
            scheduleAt(simTime() + 0.5, new cMessage("sendTestWithForwarding"));
        }
        
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
            // Forward using forwarding table from controller
            if (forwardingTableReady && forwardingTable.find(destAddr) != forwardingTable.end()) {
                int nextHop = forwardingTable[destAddr];
                
                if (nextHop == -1) {
                    // Destination is unreachable
                    EV << "\n╔════════════════════════════════════════════════════════╗\n";
                    EV << "║  ❌ PACKET DROPPED - UNREACHABLE DESTINATION ❌      ║\n";
                    EV << "╠════════════════════════════════════════════════════════╣\n";
                    EV << "║  Current Node: " << myAddress << "                                     ║\n";
                    EV << "║  Source:       " << srcAddr << "                                     ║\n";
                    EV << "║  Destination:  " << destAddr << "                                     ║\n";
                    EV << "║  Reason: No router-level path exists                 ║\n";
                    EV << "║          (Network partitions are disconnected)       ║\n";
                    EV << "╚════════════════════════════════════════════════════════╝\n\n";
                    delete pkt;
                } else {
                    int nextGate = findGateToNeighbor(nextHop);
                    
                    pkt->setHopCount(pkt->getHopCount() + 1);
                    EV << "→ Node " << myAddress << " forwarding to dest=" << destAddr 
                       << " via nextHop=" << nextHop << " gate=" << nextGate
                       << " (hop " << pkt->getHopCount() << ") [FORWARDING TABLE]\n";
                    packetsForwarded++;
                    send(pkt, "port$o", nextGate);
                }
            } else {
                EV << "No route to " << destAddr << ". Dropping packet.\n";
                delete pkt;
            }
            return;
        }
    }
    
    delete pkt;
}

// PHASE 1: Discover local topology
void BFSRouter::discoverLocalTopology() {
    EV << "Node " << myAddress << " discovering local neighbors...\n";
    
    // Learn about directly connected neighbors
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
            
            // Store direct neighbor with delay
            directNeighbors[neighborId] = delay;
            
            EV << "  Neighbor: " << neighborId << " via gate " << i << ", delay=" << delay << "ms\n";
        }
    }
    
    // Schedule sending to controller
    scheduleAt(simTime() + 0.1, new cMessage("sendToController"));
}

// PHASE 2: Send link state to central controller (includes public key)
void BFSRouter::sendLinkStateToController() {
    EV << "Node " << myAddress << " sending link state to controller (with RSA public key)...\n";
    
    // First, send this router's RSA public key to the controller
    BFSRoutingPacket *keyPkt = new BFSRoutingPacket("RSA_PUBLIC_KEY");
    keyPkt->setType("RSA_PUBLIC_KEY");
    keyPkt->setSourceAddress(myAddress);
    keyPkt->setDestinationAddress(-1);  // To controller
    keyPkt->setHopCount(publicKey);     // e (public exponent)
    keyPkt->setRequestId(modulus);       // n (modulus)
    send(keyPkt, "toController$o");
    
    EV << "  ✓ Sent RSA Public Key: e=" << publicKey << ", n=" << modulus << "\n";
    
    // Then send our local links to the central controller
    for (const auto& entry : directNeighbors) {
        int neighborId = entry.first;
        double delay = entry.second;
        
        BFSRoutingPacket *pkt = new BFSRoutingPacket("LINK_STATE");
        pkt->setType("LINK_STATE");
        pkt->setSourceAddress(myAddress);
        pkt->setDestinationAddress(-1);  // To controller
        
        // Encode link info: LINK_from_to_delay
        std::stringstream ss;
        ss << "LINK_" << myAddress << "_" << neighborId << "_" << delay;
        pkt->setName(ss.str().c_str());
        
        // Send to controller
        send(pkt, "toController$o");
        EV << "  Sent: " << myAddress << " → " << neighborId << " (delay=" << delay << "ms)\n";
    }
}

// Generate RSA key pair for this router
void BFSRouter::generateRSAKeys() {
    // Each router uses different primes based on its address
    // For simplicity, using small primes (in real world, would use large primes)
    long long p, q;
    
    switch(myAddress) {
        case 0: p = 11; q = 13; break;   // Node 0: p=11, q=13
        case 1: p = 7;  q = 17; break;   // Node 1: p=7,  q=17
        case 2: p = 11; q = 17; break;   // Node 2: p=11, q=17
        case 3: p = 13; q = 17; break;   // Node 3: p=13, q=17
        case 4: p = 7;  q = 19; break;   // Node 4: p=7,  q=19
        case 5: p = 11; q = 19; break;   // Node 5: p=11, q=19
        case 6: p = 13; q = 19; break;   // Node 6: p=13, q=19 (NEW)
        case 7: p = 17; q = 19; break;   // Node 7: p=17, q=19 (NEW)
        case 8: p = 11; q = 23; break;   // Node 8: p=11, q=23 (NEW)
        default: p = 11; q = 13; break;
    }
    
    modulus = p * q;  // n = p * q
    long long phi = (p - 1) * (q - 1);  // φ(n) = (p-1)(q-1)
    
    // Choose public exponent e (commonly 3, 7, or 65537)
    publicKey = 7;
    
    // Calculate private exponent d (d * e ≡ 1 (mod φ))
    // Using Extended Euclidean Algorithm (simplified for small numbers)
    for (long long d = 1; d < phi; d++) {
        if ((d * publicKey) % phi == 1) {
            privateKey = d;
            break;
        }
    }
}

// RSA Decryption using this router's PRIVATE KEY
long long BFSRouter::rsaDecrypt(long long ciphertext) {
    // Decryption: m = c^d mod n (using PRIVATE key!)
    long long result = 1;
    long long base = ciphertext % modulus;
    long long exp = privateKey;  // Use private key for decryption!
    long long mod = modulus;
    
    while (exp > 0) {
        if (exp % 2 == 1) {
            result = (result * base) % mod;
        }
        exp = exp / 2;
        base = (base * base) % mod;
    }
    
    return result;
}

// Receive ENCRYPTED forwarding table from central controller
void BFSRouter::receiveForwardingTable(BFSRoutingPacket *pkt) {
    // Extract ENCRYPTED forwarding entry (encrypted with THIS router's public key)
    long long encryptedDest = pkt->getHopCount();
    long long encryptedNextHop = pkt->getRequestId();
    
    EV << "Node " << myAddress << " received ENCRYPTED forwarding entry: "
       << encryptedDest << " → " << encryptedNextHop;
    
    // DECRYPT using THIS router's PRIVATE KEY
    int dest = (int)rsaDecrypt(encryptedDest);
    int nextHop = (int)rsaDecrypt(encryptedNextHop);
    
    EV << " → Decrypted with private key: dest=" << dest << ", nextHop=" << nextHop << "\n";
    
    forwardingTable[dest] = nextHop;
}

void BFSRouter::displayForwardingTable() {
    EV << "\n========================================================\n";
    EV << "  FORWARDING TABLE for Node " << myAddress << "\n";
    EV << "========================================================\n";
    EV << "  Destination → NextHop\n";
    
    for (const auto& entry : forwardingTable) {
        EV << "      " << entry.first << " → " << entry.second << "\n";
    }
    
    EV << "========================================================\n\n";
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
void BFSRouter::finish() {
    EV << "\n========================================\n";
    EV << "Node " << myAddress << " - FINAL STATISTICS\n";
    EV << "========================================\n";
    EV << "Packets delivered: " << packetsDelivered << "\n";
    EV << "Packets forwarded: " << packetsForwarded << "\n\n";
    
    // Display forwarding table
    displayForwardingTable();
    
    EV << "========================================\n\n";
}
