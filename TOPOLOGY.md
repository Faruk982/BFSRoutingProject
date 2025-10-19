# Network Topology Visualization

## Current Setup (Chain Topology)

```
     Node 0 --------- Node 1 --------- Node 2 --------- Node 3
    (Source)      (Intermediate)   (Intermediate)   (Destination)
```

## Gate Connections

Each node has the following connections:

### Node 0 (End Node)
- out[0] --> Node 1 (in[0])
- in[0] <-- Node 1 (out[0])

### Node 1 (Middle Node)
- out[0] --> Node 0 (in[0])
- out[1] --> Node 2 (in[0])
- in[0] <-- Node 0 (out[0])
- in[1] <-- Node 2 (out[0])

### Node 2 (Middle Node)
- out[0] --> Node 1 (in[1])
- out[1] --> Node 3 (in[0])
- in[0] <-- Node 1 (out[1])
- in[1] <-- Node 3 (out[0])

### Node 3 (End Node)
- out[0] --> Node 2 (in[1])
- in[0] <-- Node 2 (out[1])

## Simulation Flow

1. **t=1.0s**: Node 0 sends ROUTE_REQUEST to Node 3
   - Node 0 broadcasts → Node 1 receives
   - Node 1 learns route to Node 0, broadcasts → Node 2 receives
   - Node 2 learns route to Node 0, broadcasts → Node 3 receives
   - Node 3 learns route to Node 0, sends ROUTE_REPLY back

2. **Route Reply travels back**:
   - Node 3 → Node 2 (learns route to Node 3)
   - Node 2 → Node 1 (learns route to Node 3)
   - Node 1 → Node 0 (learns route to Node 3)

3. **t=2.5s**: Node 1 sends DATA to Node 3
   - Node 1 already has route in memory!
   - Direct forwarding: Node 1 → Node 2 → Node 3
   - No route discovery needed!

## Path Memory Advantage

- **Without Path Memory**: Every data transmission requires a new route discovery (flooding)
- **With Path Memory**: Routes learned once are cached and reused
- **Result**: Reduced network overhead, faster packet delivery
