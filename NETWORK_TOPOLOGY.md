# Network Topology for A* Visualization

## 13-Node Mesh Network

```
                Node 1 -------- Node 10
                /   |              |
               /    |              |
           Node 0   |           Node 11 ------ Node 12
               \    |              /  \          /
                \   |             /    \        /
              Node 2  \          /      \      /
                 |     \        /        \    /
                 |    Node 4 --          Node 9
              Node 3      |               /  \
                 |        |              /    \
                 |        |             /      \
              Node 5 ---- Node 6 -- Node 7 -- Node 8
```

## Connection Details

### Primary Paths (Blue - Low Delay 50-60ms):
- Node 0 ↔ Node 1 (50ms)
- Node 1 ↔ Node 10 (50ms)
- Node 10 ↔ Node 11 (60ms)

### Secondary Paths (Green - Medium Delay 80-100ms):
- Node 0 ↔ Node 2 (80ms)
- Node 1 ↔ Node 4 (100ms)

### Tertiary Paths (Red - Medium Delay 70-90ms):
- Node 2 ↔ Node 3 (70ms)
- Node 3 ↔ Node 9 (90ms)
- Node 4 ↔ Node 11 (50ms)
- Node 11 ↔ Node 12 (100ms)

### Alternative Paths (Yellow - Higher Delay 70-120ms):
- Node 3 ↔ Node 5 (120ms)
- Node 5 ↔ Node 9 (80ms)
- Node 9 ↔ Node 12 (70ms)

### Mesh Connections (Orange - Medium Delay 50-90ms):
- Node 5 ↔ Node 6 (90ms)
- Node 6 ↔ Node 7 (60ms)
- Node 7 ↔ Node 8 (50ms)
- Node 8 ↔ Node 9 (80ms)

### Shortcuts (Gray - Long Delay 150-200ms):
- Node 2 ↔ Node 9 (150ms) - Dashed line
- Node 0 ↔ Node 3 (200ms) - Dashed line

## Test Scenarios

### Scenario 1: Node 0 → Node 12
**A* will evaluate multiple paths:**

**Path 1:** 0 → 1 → 10 → 11 → 12
- Hops: 4
- Delay: 50 + 50 + 60 + 100 = 260ms
- g(n) = 4.0 + 0.026 = 4.026
- **Likely winner** (shortest hops + low delay)

**Path 2:** 0 → 2 → 3 → 9 → 12
- Hops: 4
- Delay: 80 + 70 + 90 + 70 = 310ms
- g(n) = 4.0 + 0.031 = 4.031

**Path 3:** 0 → 2 → 9 → 12 (using shortcut)
- Hops: 3
- Delay: 80 + 150 + 70 = 300ms
- g(n) = 3.0 + 0.030 = 3.030
- **May win** (fewer hops but longer delay)

### Scenario 2: Node 0 → Node 9
**Multiple alternative routes:**

**Path 1:** 0 → 2 → 3 → 9
- g(n) ≈ 3.024

**Path 2:** 0 → 2 → 9 (shortcut)
- g(n) ≈ 2.023

**Path 3:** 0 → 1 → 10 → 11 → 12 → 9
- g(n) ≈ 5.030 (longer)

### Scenario 3: Node 2 → Node 11
**Testing vertical paths:**

**Path 1:** 2 → 3 → 9 → 12 → 11
- Multiple hops through mesh

**Path 2:** 2 → 0 → 1 → 4 → 11
- Going up then across

### Scenario 4: Node 5 → Node 1
**Reverse path testing:**

**Path 1:** 5 → 3 → 2 → 0 → 1
**Path 2:** 5 → 9 → 3 → 2 → 0 → 1
**Path 3:** 5 → 9 → 12 → 11 → 10 → 1

## A* Algorithm Behavior

### Cost Function: f(n) = g(n) + h(n)

**g(n) = (hopCount × 1.0) + (delay × 0.1)**
- Each hop costs 1.0
- Delay adds 0.1 per second
- Example: 3 hops + 200ms = 3.0 + 0.02 = 3.02

**h(n) = |destination - current| × 1.0**
- Manhattan distance estimate
- Example: From node 3 to node 12: |12-3| = 9.0

### What to Observe:

1. **Route Discovery**: Watch packets broadcast to explore network
2. **Cost Comparison**: See nodes compare f(n) values
3. **Route Updates**: Better paths replace worse ones
4. **Optimal Selection**: Final path chosen based on lowest f(n)

## Visualization Tips

### In Qtenv:
1. **Slow down animation**: Adjust speed slider to see packet movement
2. **Watch message bubbles**: Shows packet forwarding decisions
3. **Check routing tables**: Right-click nodes → Inspect → routingTable
4. **Color coding**: 
   - Blue paths = Fast
   - Red paths = Medium
   - Gray paths = Slow shortcuts

### Expected Output:
```
Node 0 sending test packet to 12
A* costs at node 1: g(n)=1.005, h(n)=11, f(n)=12.005
A* costs at node 10: g(n)=2.010, h(n)=2, f(n)=4.010
Found better route to 12 via gate 2 with f(n)=4.036
Path taken: 0 → 1 → 10 → 11 → 12
Total hops: 4
g(n) cost: 4.026
```

## Benefits of Complex Topology

✅ **Multiple paths** to each destination
✅ **Trade-offs** between hop count and delay
✅ **A* optimization** clearly visible
✅ **Route learning** and caching demonstrated
✅ **RSA encryption** tested across many nodes
✅ **Broadcasting** vs **direct forwarding** comparison

---

**This topology allows A* to shine by showing intelligent path selection!**
