# Complete Workflow - BFS Routing with A* Algorithm and RSA Encryption

## 📋 Project Overview

**Project Name:** Intelligent BFS Routing with Path Memory & RSA Encryption  
**Framework:** OMNeT++ 6.2.0  
**Language:** C++ / NED  
**Topology:** 4-node chain network (Node 0 ↔ Node 1 ↔ Node 2 ↔ Node 3)  
**Algorithms:** A* Search Algorithm + RSA Encryption

---

## 🔄 Complete Workflow Step-by-Step

### **PHASE 1: Initialization (When Simulation Starts)**

#### Step 1.1: Network Creation
```
Network: BFSRoutingNetwork is created
├── Node[0] (address = 0)
├── Node[1] (address = 1)
├── Node[2] (address = 2)
└── Node[3] (address = 3)

Connections:
Node[0] <--100ms, 1Mbps--> Node[1]
Node[1] <--100ms, 1Mbps--> Node[2]
Node[2] <--100ms, 1Mbps--> Node[3]
```

#### Step 1.2: Router Initialization (Each Node)
```cpp
void BFSRouter::initialize() {
    1. Read parameters:
       - myAddress = par("address")           // e.g., 0, 1, 2, 3
       - numNodes = 4
       - useEncryption = par("useEncryption") // true/false
    
    2. If encryption enabled:
       - Generate RSA key pair (p, q primes)
       - Calculate n = p × q (modulus)
       - Calculate e = 65537 (public exponent)
       - Calculate d = e⁻¹ mod φ(n) (private exponent)
       - Log: "Node X RSA Keys - Public: (e=..., n=...)"
    
    3. Initialize data structures:
       - routingTable.clear()
       - processedRequests.clear()
       - neighborPublicKeys.clear()
    
    4. Add self to routing table:
       - routingTable[myAddress] = {
           nextHopGate: -1,
           gCost: 0.0,
           hCost: 0.0,
           fCost: 0.0,
           hopCount: 0,
           path: [myAddress]
         }
    
    5. If myAddress == 0:
       - Schedule test packet at time = 1-2 seconds
}
```

**Console Output:**
```
Node 0 initialized with 1 ports
Node 0 RSA Keys - Public: (e=65537, n=147583)
Node 1 initialized with 2 ports
Node 1 RSA Keys - Public: (e=65537, n=182391)
Node 2 initialized with 2 ports
Node 2 RSA Keys - Public: (e=65537, n=159847)
Node 3 initialized with 1 ports
Node 3 RSA Keys - Public: (e=65537, n=173269)
```

---

### **PHASE 2: Packet Creation (Node 0)**

#### Step 2.1: Self-Message Triggered
```cpp
handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        // Node 0 creates test packet
        
        1. Create packet:
           - sourceAddress = 0
           - destinationAddress = 3
           - hopCount = 0
           - path[0] = 0
           - pathLength = 1
           - timestamp = simTime()
        
        2. Calculate initial A* costs:
           - gCost = 0.0 (no hops yet)
           - hCost = |3 - 0| × 1.0 = 3.0
           - fCost = gCost + hCost = 0.0 + 3.0 = 3.0
        
        3. If encryption enabled:
           - encryptedHopCount = encrypt(0)
           - encryptedGCost = encrypt(0.0 × 1000)
           - encryptedHCost = encrypt(3.0 × 1000)
           - encryptedFCost = encrypt(3.0 × 1000)
           - senderPublicKey = 65537
           - senderModulus = 147583
           - isEncrypted = true
        
        4. Broadcast to all neighbors:
           - Send to port[0] (Node 1)
    }
}
```

**Console Output:**
```
Node 0 encrypted routing info before sending
Encrypted routing info: hopCount=0 -> 98234
Node 0 sending test packet to 3
```

---

### **PHASE 3: Packet Reception & Processing (Node 1)**

#### Step 3.1: Packet Arrival at Node 1
```cpp
handleMessage(cMessage *msg) {
    1. Receive packet from Node 0
    2. Extract information:
       - srcAddr = 0
       - destAddr = 3
       - arrivalGate = 0 (port[0])
    
    3. Check if encrypted:
       if (pkt->isEncrypted() && useEncryption) {
           - Log: "Node 1 received encrypted packet from 0"
           - Store Node 0's public key: (e=65537, n=147583)
           - Decrypt routing info:
             * hopCount = decrypt(98234) = 0
             * gCost = decrypt(...) / 1000.0 = 0.0
             * hCost = decrypt(...) / 1000.0 = 3.0
             * fCost = decrypt(...) / 1000.0 = 3.0
       }
    
    4. Check if destination:
       - destAddr (3) ≠ myAddress (1) → NOT DESTINATION
    
    5. Check for loops:
       - path = [0]
       - myAddress (1) not in path → NO LOOP
    
    6. Update path:
       - path[1] = 1
       - pathLength = 2
       - path = [0, 1]
    
    7. Increment hop count:
       - hopCount = 0 + 1 = 1
}
```

**Console Output:**
```
Node 1 received encrypted packet from 0
Node 1 stored public key of node 0: (e=65537, n=147583)
Decrypted routing info: 98234 -> hopCount=0
Decrypted costs: g=0.0, h=3.0, f=3.0
Node 1 received packet from 0 to 3 via gate 0
```

---

### **PHASE 4: A* Cost Calculation (Node 1)**

#### Step 4.1: Calculate g(n) - Actual Cost
```cpp
double calculateGCost(BFSRoutingPacket *pkt) {
    1. Get current simulation time: simTime() = 1.1 seconds
    2. Get packet timestamp: 1.0 seconds
    3. Calculate delay: 1.1 - 1.0 = 0.1 seconds
    4. Calculate g(n):
       gCost = (hopCount × 1.0) + (delay × 0.1)
       gCost = (1 × 1.0) + (0.1 × 0.1)
       gCost = 1.0 + 0.01 = 1.01
    
    return 1.01
}
```

#### Step 4.2: Calculate h(n) - Heuristic
```cpp
double calculateHeuristic(int currentNode, int destNode) {
    1. Current node: 1
    2. Destination node: 3
    3. Calculate Manhattan distance:
       hCost = |3 - 1| × 1.0 = 2.0
    
    return 2.0
}
```

#### Step 4.3: Calculate f(n) - Total Cost
```cpp
fCost = gCost + hCost
fCost = 1.01 + 2.0 = 3.01
```

**Console Output:**
```
A* costs at node 1: g(n)=1.01, h(n)=2.0, f(n)=3.01
```

---

### **PHASE 5: Routing Table Update (Node 1)**

#### Step 5.1: Update Route to Source (Node 0)
```cpp
updateRouteInfo(srcAddr=0, pkt, arrivalGate=0) {
    1. Check if route to Node 0 exists:
       - routingTable[0] not found → NEW ROUTE
    
    2. Create new route entry:
       routingTable[0] = {
           nextHopGate: 0,        // Send back via port[0]
           gCost: 1.01,
           hCost: 2.0,
           fCost: 3.01,
           hopCount: 1,
           useCount: 1,
           successRate: 1.0,
           timestamp: 1.1,
           path: [0, 1]
       }
    
    3. Log: "New route to 0 via gate 0 with f(n)=3.01"
}
```

#### Step 5.2: Check Route to Destination (Node 3)
```cpp
if (routingTable.find(destAddr=3) != routingTable.end()) {
    // No route to Node 3 exists yet
    // Must broadcast/forward
}
```

---

### **PHASE 6: Packet Forwarding (Node 1 → Node 2)**

#### Step 6.1: Broadcast Decision
```cpp
else {
    // No route to destination - broadcast
    
    1. Log: "No route to 3. Broadcasting..."
    
    2. For each port (except arrival gate 0):
       - port[1] is connected to Node 2
       
       a. Duplicate packet
       b. If encryption enabled:
          - Re-encrypt routing info with Node 1's keys
          - encryptedHopCount = encrypt(1)
          - encryptedGCost = encrypt(1010)  // 1.01 × 1000
          - encryptedHCost = encrypt(2000)  // 2.0 × 1000
          - encryptedFCost = encrypt(3010)  // 3.01 × 1000
          - senderPublicKey = Node 1's e
          - senderModulus = Node 1's n
       
       c. Send via port[1] to Node 2
}
```

**Console Output:**
```
No route to 3. Broadcasting...
Node 1 encrypted routing info before sending
```

---

### **PHASE 7: Processing at Node 2**

#### Step 7.1: Similar Process as Node 1
```
1. Receive encrypted packet from Node 1
2. Store Node 1's public key
3. Decrypt routing info:
   - hopCount = 1
   - gCost = 1.01
   - hCost = 2.0
   - fCost = 3.01

4. Check destination: 3 ≠ 2 → NOT DESTINATION

5. Check loop: [0, 1] doesn't contain 2 → NO LOOP

6. Update path: [0, 1, 2]
7. Increment hopCount: 1 + 1 = 2

8. Calculate NEW A* costs:
   - delay = 0.2 seconds
   - gCost = (2 × 1.0) + (0.2 × 0.1) = 2.02
   - hCost = |3 - 2| × 1.0 = 1.0
   - fCost = 2.02 + 1.0 = 3.02

9. Update routing table with route to Node 0 and Node 1

10. No route to Node 3 yet → broadcast to Node 3
```

**Console Output:**
```
Node 2 received encrypted packet from 1
Node 2 stored public key of node 1: (e=65537, n=182391)
Decrypted routing info: 87654 -> hopCount=1
Decrypted costs: g=1.01, h=2.0, f=3.01
Node 2 received packet from 0 to 3 via gate 0
A* costs at node 2: g(n)=2.02, h(n)=1.0, f(n)=3.02
New route to 0 via gate 0 with f(n)=3.02
New route to 1 via gate 0 with f(n)=3.02
No route to 3. Broadcasting...
```

---

### **PHASE 8: Packet Delivery (Node 3)**

#### Step 8.1: Destination Reached
```cpp
handleMessage(cMessage *msg) {
    1. Receive encrypted packet from Node 2
    2. Decrypt routing info
    3. Check destination:
       - destAddr (3) == myAddress (3) → DESTINATION!
    
    4. Log delivery:
       - "Node 3 is destination. Packet delivered!"
       - "Path taken: 0 -> 1 -> 2 -> 3"
       - "Total hops: 3"
       - "g(n) cost: 3.03"
       - "h(n) cost: 0.0"
       - "f(n) cost: 3.03"
    
    5. Delete packet (end of journey)
}
```

**Console Output:**
```
Node 3 received encrypted packet from 2
Node 3 stored public key of node 2: (e=65537, n=159847)
Decrypted routing info: 76543 -> hopCount=2
Decrypted costs: g=2.02, h=1.0, f=3.02
Node 3 received packet from 0 to 3 via gate 0
Node 3 is destination. Packet delivered!
Path taken: 0 -> 1 -> 2 -> 3
Total hops: 3
g(n) cost: 3.03
h(n) cost: 0.0
f(n) cost: 3.03
```

---

### **PHASE 9: Return Path Learning (Optional)**

If Node 3 sends a packet back to Node 0:

```
1. Node 3 now has routes to: Node 0, Node 1, Node 2
2. When sending back, it can use optimal route:
   - Check routingTable[0]
   - Find route with lowest fCost
   - Forward via that gate (already knows the path)
3. No broadcasting needed - direct forwarding!
```

---

### **PHASE 10: Simulation Finish**

#### Step 10.1: Statistics Collection
```cpp
void BFSRouter::finish() {
    For each node:
    1. Log routes learned
    2. If encryption enabled:
       - Log encrypted packets sent
       - Log encrypted packets received
       - Log neighbor public keys stored
    3. Clean up RSA instance
}
```

**Console Output:**
```
Node 0 finishing simulation
Routes learned: 1
Encrypted packets sent: 1
Encrypted packets received: 0
Neighbor public keys stored: 0

Node 1 finishing simulation
Routes learned: 2
Encrypted packets sent: 1
Encrypted packets received: 1
Neighbor public keys stored: 1

Node 2 finishing simulation
Routes learned: 3
Encrypted packets sent: 1
Encrypted packets received: 1
Neighbor public keys stored: 1

Node 3 finishing simulation
Routes learned: 3
Encrypted packets sent: 0
Encrypted packets received: 1
Neighbor public keys stored: 1
```

---

## 📊 Visual Workflow Diagram

```
Time = 0.0s
┌────────────────────────────────────────────┐
│         INITIALIZATION PHASE               │
│  All nodes generate RSA key pairs         │
│  Routing tables initialized                │
└────────────────────────────────────────────┘

Time = 1.0s
┌─────────┐
│  Node 0 │ Creates packet (dest=3)
│         │ Encrypts: hop=0, g=0.0, h=3.0, f=3.0
└────┬────┘
     │ Encrypted packet
     ▼
┌─────────┐
│  Node 1 │ Receives & decrypts
│         │ Calculates: g=1.01, h=2.0, f=3.01
│         │ Stores Node 0's public key
│         │ Re-encrypts & forwards
└────┬────┘
     │ Encrypted packet
     ▼
┌─────────┐
│  Node 2 │ Receives & decrypts
│         │ Calculates: g=2.02, h=1.0, f=3.02
│         │ Stores Node 1's public key
│         │ Re-encrypts & forwards
└────┬────┘
     │ Encrypted packet
     ▼
┌─────────┐
│  Node 3 │ Receives & decrypts
│         │ Calculates: g=3.03, h=0.0, f=3.03
│         │ Stores Node 2's public key
│         │ DESTINATION REACHED!
└─────────┘

Time = End
┌────────────────────────────────────────────┐
│           FINISH PHASE                     │
│  All nodes report statistics               │
│  RSA instances cleaned up                  │
└────────────────────────────────────────────┘
```

---

## 🔐 Encryption Workflow Detail

```
Node A wants to send to Node B:
┌─────────────────────────────────────────────────────┐
│ 1. Calculate routing metrics (hopCount, g, h, f)   │
│    hopCount = 2                                     │
│    gCost = 2.1                                      │
│    hCost = 1.5                                      │
│    fCost = 3.6                                      │
└─────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────┐
│ 2. Encrypt with Node A's PRIVATE key               │
│    encryptedHop = encrypt(2) = 98234                │
│    encryptedG = encrypt(2100) = 145678              │
│    encryptedH = encrypt(1500) = 123456              │
│    encryptedF = encrypt(3600) = 187654              │
└─────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────┐
│ 3. Attach Node A's PUBLIC key to packet             │
│    senderPublicKey = 65537 (e)                      │
│    senderModulus = 147583 (n)                       │
│    isEncrypted = true                               │
└─────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────┐
│ 4. Transmit encrypted packet over network          │
│    [Encrypted data cannot be read by attackers]    │
└─────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────┐
│ 5. Node B receives packet                          │
│    - Stores Node A's public key (e, n)             │
│    - Decrypts with Node B's PRIVATE key:           │
│      hopCount = decrypt(98234) = 2                  │
│      gCost = decrypt(145678) / 1000 = 2.1           │
│      hCost = decrypt(123456) / 1000 = 1.5           │
│      fCost = decrypt(187654) / 1000 = 3.6           │
└─────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────┐
│ 6. Node B uses decrypted values for routing        │
│    - Updates routing table                         │
│    - Makes forwarding decisions                    │
│    - Re-encrypts before forwarding to Node C       │
└─────────────────────────────────────────────────────┘
```

---

## 🎯 Key Features in Action

### 1. **A* Algorithm**
```
At each node:
f(n) = g(n) + h(n)

g(n) = Actual cost from source
     = (hopCount × 1.0) + (delay × 0.1)

h(n) = Heuristic estimate to destination
     = |destination - current_node| × 1.0

Route selection: Choose path with LOWEST f(n)
```

### 2. **RSA Encryption**
```
Each node:
- Generates unique key pair at startup
- Encrypts outgoing routing info
- Decrypts incoming routing info
- Stores neighbor public keys
- Prevents eavesdropping on routing metrics
```

### 3. **Path Memory**
```
Routing table stores:
- Best path to each destination
- Cost metrics (g, h, f)
- Next hop gate
- Path history
- Success rate
- Timestamp

Benefit: No need to re-discover routes!
```

---

## 🚀 Running the Workflow

### Build:
```bash
cd d:/downloads/omnetpp-6.2.0-windows-x86_64/omnetpp-6.2.0/samples/BFSRoutingProject_New/src
make clean
make MODE=release all
```

### Run:
```bash
cd ../simulations
../src/BFSRoutingProject_New
```

### Watch:
- Qtenv GUI opens
- Click "Run" to start simulation
- Observe packet flow in animation
- Check log window for encrypted/decrypted messages
- See routing table updates in real-time

---

## ✅ Summary

**Complete workflow:**
1. **Initialize** → Generate RSA keys, setup routing tables
2. **Create** → Node 0 creates test packet
3. **Encrypt** → Routing info encrypted with RSA
4. **Transmit** → Packet sent to neighbor
5. **Receive** → Neighbor receives encrypted packet
6. **Decrypt** → Routing info decrypted
7. **Calculate** → A* costs computed (g, h, f)
8. **Update** → Routing table updated with new route
9. **Forward** → Re-encrypt and forward to next hop
10. **Deliver** → Destination reached, packet delivered
11. **Finish** → Statistics collected and displayed

**Result:** Secure, optimal routing with encrypted routing information! 🎉
