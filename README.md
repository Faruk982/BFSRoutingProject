# Centralized SDN Routing with A* Path Computation and RSA Encryption

## Project Overview

This project implements a Software-Defined Network (SDN) architecture where a centralized controller computes optimal paths for all nodes using the A* search algorithm and distributes forwarding tables to simple routers. The forwarding tables are encrypted using RSA public-key cryptography to ensure secure transmission.

### Key Features

- Centralized Controller: All routing intelligence resides in a central controller.
- A* Path Computation: Controller computes optimal paths for all node pairs using the A* algorithm.
- Simple Forwarding Routers: Routers only store and use forwarding tables (no routing computation).
- RSA Encryption: Each router's forwarding table is encrypted with that router's public key.
- Secure Key Exchange: Routers generate their own RSA key pairs and send public keys to the controller.
- Demonstration-Ready: Extensive logging shows the encryption/decryption process for educational purposes.

## Architecture

### Network Topology

The simulation uses an expanded 9-node network (nodes 0 through 8) along with 1 centralized controller.

The network structure consists of an original 6-node mesh network (nodes 0-5) that has been expanded by bridging to a new set of nodes (6, 7, and 8). 
Connections breakdown:
- Original Mesh: (Node 0 to Node 1), (Node 0 to Node 2), (Node 1 to Node 3), (Node 2 to Node 3), (Node 2 to Node 4), (Node 3 to Node 5), (Node 4 to Node 5), (Node 1 to Node 4).
- New Expansion Links: (Node 6 to Node 7), (Node 7 to Node 8).
- Bridging Connections: (Node 3 to Node 7), (Node 4 to Node 8).
 
Total: 9 router nodes (0-8) + 1 centralized controller connected via an SDN-style star topology (all router nodes connect directly to the controller).

### Components

#### 1. Centralized Controller (CentralController.cc, CentralController.h)
- Receives link-state information from all routers.
- Receives RSA public keys from all routers.
- Builds the complete network topology (adjacency list).
- Computes optimal paths for all node pairs using the A* algorithm.
- Encrypts forwarding tables using each router's public key.
- Distributes encrypted forwarding tables to routers.

#### 2. Simple Routers (BFSRouter.cc, BFSRouter.h)
- Discover only directly connected neighbors.
- Generate their own RSA key pair (public + private).
- Send their public key and link-state to the controller.
- Receive the encrypted forwarding table from the controller.
- Decrypt the forwarding table using their own private key.
- Forward packets based on the forwarding table (no path computation).

## RSA Encryption Implementation

### How It Works

1. Key Generation (Router Side)
   - Each router generates its own RSA key pair on initialization.
   - A public exponent (e) and a prime-derived modulus (n) form the public key.
   - A private exponent (d) is calculated uniquely for each router to act as the private key.

2. Public Key Distribution (Router to Controller)
   - Each router sends its public key (e, n) to the controller along with link-state information.
   - The controller stores these in a public keys mapping.

3. Encryption (Controller to Router)
   - The controller encrypts each router's forwarding table.
   - Uses that specific router's public key for encryption.
   - Formula: ciphertext = plaintext^e mod n
   - Encrypts node IDs (destination and next hop).

4. Decryption (Router Side)
   - The router receives encrypted forwarding entries.
   - Decrypts using its own private key.
   - Formula: plaintext = ciphertext^d mod n
   - Only the intended router can decrypt it since only it holds the private key.

### Security Benefits

- Confidentiality: Only the intended router can decrypt its forwarding table.
- Authentication: The router verifies data came from the controller.
- Integrity: Tampering is detected (decryption fails or produces invalid data).

## A* Algorithm Implementation

### Controller-Side Path Computation

The controller uses the A* search algorithm to compute optimal paths:

Path: source to destination
Cost: g(n) = actual cost from source to n
Heuristic: h(n) = estimated cost from n to destination (using delays)
f(n) = g(n) + h(n) // Total estimated cost

### Process

1. Build Adjacency List
   - The controller receives link-state from all routers.
   - Constructs the complete network graph using link delays as edge weights.

2. Compute All Paths
   - For each source-destination pair among all 9 nodes.
   - Runs A* to find optimal paths.
   - Extracts the next hop for each source.

3. Build Forwarding Tables
   - For each router: forwardingTable[destination] = nextHop
   - Example for Node 0: To reach Node 5, nextHop = Node 2.

## Message Flow

### Phase 1: Topology Discovery
1. Each router discovers its direct neighbors.
2. The router generates an RSA key pair.
3. The router sends its RSA_PUBLIC_KEY to the controller (e, n).
4. The router sends LINK_STATE to the controller (neighbor info).

### Phase 2: Centralized Computation
1. The controller builds an adjacency list from all link-states.
2. The controller stores all router public keys.
3. The controller runs A* for all node pairs.
4. The controller builds the forwarding table for each router.

### Phase 3: Secure Distribution
1. The controller encrypts each router's forwarding table using that router's specific public key.
2. The controller sends FORWARDING_ENTRY packets (encrypted).
3. The controller sends a FORWARDING_COMPLETE signal.

### Phase 4: Router Operation
1. The router receives encrypted forwarding entries.
2. The router decrypts them using its private key.
3. The router stores the forwarding table.
4. The router forwards packets based on the forwarding table.

### Phase 5: Data Transmission
1. Tests can be run by having a node send a packet to another node.
2. Each router looks up the destination in its forwarding table.
3. The packet is forwarded to the next hop.
4. The packet is delivered successfully.

## File Structure

BFSRoutingProject_New/
|-- src/
|   |-- BFSRouter.h/cc              # Router implementation
|   |-- CentralController.h/cc      # Controller implementation
|   |-- BFSRouting.ned              # Network definition
|   |-- BFSRoutingPacket.msg        # Message definition
|   |-- BFSRoutingPacket_m.h/cc     # Generated message code
|   |-- Makefile                    # Build configuration
|-- simulations/
|   |-- omnetpp.ini                 # Simulation parameters
|   |-- package.ned                 # Package definition
|   |-- run                         # Run script
|-- README.md                       # This documentation

## How to Build and Run

### Prerequisites
- OMNeT++ 6.2.0 or later
- Windows (MinGW) or Linux environment

### Build Instructions

1. Open OMNeT++ Shell (Windows):
   cd d:\downloads\omnetpp-6.2.0-windows-x86_64\omnetpp-6.2.0
   mingwenv.cmd

2. Navigate to Project:
   cd samples/BFSRoutingProject_New
