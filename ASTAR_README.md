# A* Routing Algorithm Implementation

## 🌟 **Overview**

This project implements **A* (A-star) routing** with Path Memory in OMNeT++. Unlike traditional routing protocols, this uses the A* search algorithm to find optimal paths through the network.

## 📐 **A* Algorithm Components**

### **Cost Functions:**

```
f(n) = g(n) + h(n)
```

- **g(n)**: Actual cost from source to node `n`
  - Hop count × 1.0
  - Cumulative delay × 0.1
  - Formula: `g(n) = hops + (time_elapsed × 0.1)`

- **h(n)**: Heuristic estimate from node `n` to destination
  - Based on topological distance
  - Formula: `h(n) = |destination - current_node| × 1.0`

- **f(n)**: Total estimated cost (used to select best path)

### **Key Features:**

1. **Path Memory**: Stores routes with their f-costs
2. **Cost-based Selection**: Always chooses path with lowest f-cost
3. **Dynamic Updates**: Updates routes when better paths are found
4. **Pruning**: Discards suboptimal paths during discovery

## 🎯 **How It Works**

### **Route Discovery Phase (A*):**

1. Source broadcasts ROUTE_REQUEST with:
   - g(n) = 0 (starting cost)
   - h(n) = estimated distance to destination
   - f(n) = g(n) + h(n)

2. Each intermediate node:
   - Calculates its g(n) = previous g + hop cost + delay
   - Calculates new h(n) to destination
   - Updates f(n) = g(n) + h(n)
   - **Prunes** if a better path was already found
   - Forwards only if this is the best path so far

3. Destination receives requests and replies with best path

### **Path Memory:**

Routes are stored with:
- `nextHopGate`: Which gate to forward to
- `gCost`: Actual cost to destination
- `hCost`: Heuristic estimate
- `fCost`: Total cost (g + h)
- `hopCount`: Number of hops
- `useCount`: How many times used
- `successRate`: Reliability metric
- `timestamp`: When learned

### **Route Selection:**

When sending data:
1. Check Path Memory for destination
2. Use route with **lowest f-cost**
3. Increment use counter
4. Forward directly (no new discovery needed!)

## 💡 **Advantages Over Standard Routing**

| Feature | Standard BFS | Our A* Implementation |
|---------|-------------|----------------------|
| Path Selection | First path found | **Optimal path (lowest cost)** |
| Discovery Overhead | Floods entire network | **Prunes suboptimal paths** |
| Cost Awareness | No cost consideration | **Considers hops + delay** |
| Path Memory | Simple caching | **Cost-based with metrics** |
| Route Updates | Replace on any new path | **Replace only if better f-cost** |

## 📊 **Simulation Scenario**

1. **t=1.0s**: Node 0 → Node 3 (A* discovery)
   - Packets carry g, h, f costs
   - Each node calculates optimal path
   - Best path is cached

2. **t=2.5s**: Node 1 → Node 3 (Uses cached route!)
   - No discovery needed
   - Direct forwarding using stored f-cost

3. **t=4.0s**: Node 2 → Node 0 (Reverse path test)
   - Uses previously learned return routes

## 🔍 **What to Watch in Simulation**

### **Console Output Shows:**
```
Router 1 received ROUTE_REQUEST from 0 to 3 | g=1.01 h=2.0 f=3.01 (hops: 1)
★ New A* route learned: to node 0 via gate 0 with f=3.01
   Forwarding with updated f=3.12
★ A* Path Memory Hit! Using cached route with f-cost=3.15, hops=2
```

### **Key Metrics:**
- **g values**: Show actual accumulated cost
- **h values**: Show heuristic estimates
- **f values**: Show total path quality
- **Pruning messages**: Show when suboptimal paths are discarded

## 🎓 **Heuristic Function Details**

### **Current Implementation:**
```cpp
h(n) = |destination_node - current_node| × 1.0
```

This works well for chain topology where node IDs reflect distance.

### **Alternative Heuristics (Future):**
1. **Geographic Distance**: If nodes have (x,y) coordinates
   ```cpp
   h(n) = sqrt((x2-x1)² + (y2-y1)²)
   ```

2. **Historical Performance**: Based on past success rates
   ```cpp
   h(n) = estimated_hops × (1.0 - success_rate)
   ```

3. **Learned Patterns**: Machine learning based predictions
   ```cpp
   h(n) = neural_network_estimate(current, destination)
   ```

## 📈 **Statistics Collected**

At the end of simulation, each node reports:
- Packets forwarded
- Packets delivered
- Route discoveries sent
- A* routes learned
- Complete routing table with f-costs

Example output:
```
=== Node 1 A* Routing Statistics ===
Route to node 0: gate=0 f=1.01 g=1.01 h=0.0 hops=1 uses=5
Route to node 3: gate=1 f=2.05 g=1.02 h=1.0 hops=2 uses=3
```

## 🚀 **Build and Run**

```bash
# Clean and build
cd d:/downloads/omnetpp-6.2.0-windows-x86_64/omnetpp-6.2.0/samples/BFSRoutingProject_New
make clean
make MODE=debug all

# Run simulation
cd simulations
../src/BFSRoutingProject_New
```

## 🎯 **Project Benefits**

1. ✅ **Optimal Paths**: Uses A* to find best routes
2. ✅ **Cost-Aware**: Considers both hops and delays
3. ✅ **Efficient**: Prunes suboptimal paths early
4. ✅ **Intelligent Memory**: Stores routes with quality metrics
5. ✅ **Self-Learning**: Improves over time with usage statistics

## 📚 **Technical Details**

- **Language**: C++ (OMNeT++)
- **Algorithm**: A* Search
- **Data Structures**: `std::map` for routing table, `std::set` for tracking
- **Packet Fields**: Extended with g, h, f costs
- **Heuristic**: Manhattan distance (adaptable)

---

**This implementation demonstrates true intelligent routing using the A* algorithm!** 🌟
