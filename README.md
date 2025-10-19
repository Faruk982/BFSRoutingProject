# Intelligent BFS Routing with Path Memory & RSA

## Project Description

This project implements a Best-First Search (BFS) routing protocol that learns from experience. Its unique aspect is a "Path Memory" system where routers cache successfully used paths. When sending data, a node first checks its memory for a recent, reliable route instead of performing a full BFS discovery every time, making routing faster and reducing network control traffic.

## Network Topology

The simulation creates a simple chain network:
```
Node 0 <---> Node 1 <---> Node 2 <---> Node 3
```

## Simulation Scenario

1. **t = 1.0s**: Node 0 initiates a route discovery to Node 3
   - Route request floods through the network
   - Each intermediate node learns the path back to Node 0
   - Node 3 sends a route reply back
   - All nodes along the path learn the route

2. **t = 2.5s**: Node 1 sends a data packet to Node 3
   - Node 1 already has the route in memory (learned from Node 0's discovery)
   - Packet is forwarded directly without new route discovery
   - **This demonstrates the Path Memory advantage!**

## How to Build

1. Open OMNeT++ MinGW shell:
   ```bash
   cd d:\downloads\omnetpp-6.2.0-windows-x86_64\omnetpp-6.2.0
   mingwenv.cmd
   ```

2. Navigate to the project and build:
   ```bash
   cd samples/BFSRoutingProject_New
   make MODE=debug all
   ```

## How to Run

### Option 1: Graphical Mode (Qtenv)
```bash
cd simulations
../src/BFSRoutingProject_New
```
Or simply double-click the `run` file in the `simulations` folder.

### Option 2: Command Line Mode
```bash
cd simulations
../src/BFSRoutingProject_New -u Cmdenv
```

## What to Watch For

- **Route Discovery Phase**: Watch the blue route request packets flood through the network
- **Route Reply Phase**: See the reply packets trace back to the source
- **Path Memory in Action**: When Node 1 sends data at t=2.5s, it uses the already-learned route
- **Console Output**: Check the log messages showing when routes are learned and stored

## Key Features Implemented

✅ **BFS Route Discovery**: Broadcast-based route finding
✅ **Path Memory**: Routers cache learned routes
✅ **Route Learning**: Automatic learning from passing packets
✅ **Packet Types**: ROUTE_REQUEST, ROUTE_REPLY, DATA
✅ **Statistics**: Track forwarded and delivered packets

## Future Enhancements

- RSA encryption for route discovery packets (security layer)
- Route aging and expiration
- Dynamic route recalculation on failures
- More complex network topologies
- Performance metrics visualization

## Files

- `src/BFSRouter.h` - Router module header with path memory
- `src/BFSRouter.cc` - Router implementation with BFS logic
- `src/BFSRoutingPacket.msg` - Packet definition
- `src/BFSRouting.ned` - Network topology definition
- `simulations/omnetpp.ini` - Simulation configuration
