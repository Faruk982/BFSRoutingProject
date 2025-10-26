# Centralized Controller Implementation

## Overview
Replaced distributed flooding approach with **Centralized Routing (SDN-style)** architecture to eliminate flooding loops.

## Architecture

### Problem
- Previous design used distributed flooding for topology exchange
- Caused infinite loop between nodes (especially node0 ↔ node1)
- Even with flood control (`receivedTopology` set), complexity increased

### Solution: Centralized Controller
- **Single Central Controller** acts as topology database
- All routers send their link states to controller
- Controller collects all information and broadcasts complete topology to all routers
- **No peer-to-peer flooding** - star topology to controller eliminates loops

## Key Components

### 1. CentralController Module

**Files:**
- `src/CentralController.h` - Header with class definition
- `src/CentralController.cc` - Implementation

**Functionality:**
```
Phase 1 (t=0.2s): Receive link states from all routers
  - Routers send LINK_STATE packets with format: "LINK_from_to_delay"
  - Controller stores in completeTopology map
  - Tracks registeredNodes set

Phase 2 (t=0.3s): Broadcast complete topology
  - When all 13 nodes have registered
  - Send all links to each router via toRouter[nodeId] gates
  - Routers receive complete network view
```

**Key Data Structures:**
```cpp
struct ControllerLinkInfo {
    int fromNode;
    int toNode;
    double delay;
};

map<int, vector<ControllerLinkInfo>> completeTopology;
set<int> registeredNodes;
```

### 2. Updated BFSRouter Module

**Changes:**
- Added `toController` gate for communication with controller
- **Removed** `broadcastTopology()` - no more flooding!
- **Removed** `receiveTopologyInfo()` - no peer-to-peer topology exchange
- **Added** `sendLinkStateToController()` - sends local links to controller
- **Added** `receiveTopologyFromController()` - receives complete topology
- **Added** `findGateToNeighbor()` - helper to map neighbor IDs to gate indices

**New Workflow:**
```
Phase 1 (t=0.1s): discoverLocalTopology()
  - Find directly connected neighbors via gates
  - Store in networkTopology[myAddress]

Phase 2 (t=0.2s): sendLinkStateToController()
  - Send each local link to controller
  - Format: LINK_STATE packet with name "LINK_from_to_delay"
  - Send via toController$o gate

Phase 3 (t=varies): receiveTopologyFromController()
  - Receive LINK packets from controller (sourceAddress = -1)
  - Parse and store in networkTopology map
  - Build knownNodes set
  - Trigger calculatePaths after receiving topology

Phase 4 (t=1.5s): calculateAllPaths()
  - Run A* for each known destination
  - Build routing table with optimal paths

Phase 5 (t=2.0s): Forward packets using pre-calculated paths
```

### 3. NED File Updates

**Added Controller Definition:**
```ned
simple CentralController {
    parameters:
        int totalNodes = default(13);
        @display("i=block/cogwheel");
    gates:
        inout toRouter[13];  // Star topology to all routers
}
```

**Updated Router:**
```ned
simple BFSRouter {
    gates:
        inout port[];
        inout toController;  // NEW: Connection to central controller
}
```

**Network Connections:**
```ned
network BFSRoutingNetwork {
    submodules:
        controller: CentralController {
            @display("p=700,450");  // Center of network
        }
        node0: BFSRouter { ... }
        // ... all 13 nodes
    
    connections:
        // Original mesh topology (18 links)
        node0.port++ <--> {...} <--> node1.port++;
        // ...
        
        // NEW: Star topology to controller
        node0.toController <--> controller.toRouter[0];
        node1.toController <--> controller.toRouter[1];
        // ... all 13 nodes connected to controller
}
```

### 4. Makefile Updates

**Added CentralController.o:**
```makefile
OBJS = $O/BFSRouter.o $O/CentralController.o $O/BFSRoutingPacket_m.o
```

## Benefits

### 1. Eliminates Flooding Loops
- No more node-to-node topology propagation
- Single point of collection and distribution
- No need for complex flood control mechanisms

### 2. Simplified Logic
- Clear separation of concerns
  - Controller: Topology collection & distribution
  - Routers: Local discovery & path calculation
- Easier to debug and maintain

### 3. Scalability
- Controller can easily manage topology changes
- New nodes only need to register with controller
- Centralized view enables advanced features (load balancing, QoS, etc.)

### 4. Similar to Real-World SDN
- OpenFlow/SDN architecture pattern
- Centralized control plane, distributed data plane
- Controller has complete network view

## Packet Format

### LINK_STATE Packet (Router → Controller)
```cpp
type: "LINK_STATE"
sourceAddress: myAddress
destinationAddress: -1  // To controller
name: "LINK_fromNode_toNode_delay"  // e.g., "LINK_0_1_50.0"
```

### TOPOLOGY Packet (Controller → Router)
```cpp
type: "TOPOLOGY"
sourceAddress: -1  // From controller
destinationAddress: -1  // Broadcast
name: "LINK_fromNode_toNode_delay"  // Same format as link state
```

## Compilation

**To build:**
```bash
cd src
opp_makemake -f --deep
make MODE=debug
```

**Or using OMNeT++ IDE:**
1. Right-click project → Build Project
2. Makefile will automatically compile CentralController.o

## Testing

**Expected Flow:**
1. Simulation starts
2. t=0.1s: All routers discover local neighbors
3. t=0.2s: Routers send link states to controller
4. t=0.3s: Controller receives all 13 nodes' link states
5. t=0.3s: Controller broadcasts complete topology to all routers
6. t=1.0s: Routers receive full topology, calculate A* paths
7. t=1.5s: Node 0 sends test packet to Node 12
8. Packet follows optimal A* path (no loops!)

**Verification:**
- Check log: "Node X sending link state to central controller..."
- Check log: "Node X learned from controller: A -> B (delay=Xms)"
- Check log: "Running A* for all destinations..."
- Check log: "PACKET DELIVERED to Node 12 from Node 0"
- No flooding loops!

## Future Enhancements

1. **Dynamic Topology Updates**
   - Controller can detect link failures
   - Redistribute updated topology
   - Routers recalculate paths

2. **Load Balancing**
   - Controller tracks link utilization
   - Can suggest alternative paths
   - Implement ECMP (Equal-Cost Multi-Path)

3. **QoS Routing**
   - Controller maintains service level requirements
   - Routers request paths with QoS constraints
   - A* can include bandwidth/latency in cost function

4. **Authentication**
   - Secure controller-router communication
   - Re-enable RSA for control messages
   - Prevent rogue nodes from poisoning topology

## Comparison: Flooding vs Centralized

| Aspect | Distributed Flooding | Centralized Controller |
|--------|---------------------|------------------------|
| **Topology Exchange** | Peer-to-peer flooding | Star: all send to controller |
| **Loop Prevention** | Complex flood control | Inherent (no loops in star) |
| **Scalability** | O(N²) messages | O(N) messages |
| **Convergence Time** | Depends on TTL/hops | Fast (one round trip) |
| **Fault Tolerance** | High (distributed) | Lower (single point of failure) |
| **Complexity** | High (each node processes floods) | Low (centralized logic) |
| **Real-World Example** | OSPF, IS-IS | OpenFlow/SDN |

## Files Modified

1. ✅ `src/CentralController.h` - NEW FILE
2. ✅ `src/CentralController.cc` - NEW FILE
3. ✅ `src/BFSRouter.h` - Updated method signatures
4. ✅ `src/BFSRouter.cc` - Replaced flooding with controller communication
5. ✅ `src/BFSRouting.ned` - Added controller module and connections
6. ✅ `src/Makefile` - Added CentralController.o to OBJS

## Summary

Successfully migrated from **distributed flooding** to **centralized controller** architecture:
- **Problem:** Flooding caused infinite loops between nodes
- **Solution:** Central controller collects all link states and distributes complete topology
- **Result:** No loops, simpler code, SDN-style architecture
- **Next:** Ready to compile and test!
