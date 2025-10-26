package bfsroutingproject_new;

simple BFSRouter {
    parameters:
        int address;
        bool useEncryption = default(true);  // Enable RSA encryption for routing info
        @display("i=block/routing");
    gates:
        inout port[];
        inout toController;  // Connection to central controller
}

simple CentralController {
    parameters:
        int totalNodes = default(6);
        @display("i=block/cogwheel");
    gates:
        inout toRouter[6];  // Connections to all routers
}

network BFSRoutingNetwork {
    parameters:
        int numNodes = 6;  // Simplified to 6 nodes
        @display("bgb=800,600");
    submodules:
        // Central Controller (SDN-style)
        controller: CentralController {
            @display("p=400,300");
        }
        
        // Simple 6-node topology
        node0: BFSRouter {
            address = 0;
            @display("p=200,100");
        }
        node1: BFSRouter {
            address = 1;
            @display("p=600,100");
        }
        node2: BFSRouter {
            address = 2;
            @display("p=200,300");
        }
        node3: BFSRouter {
            address = 3;
            @display("p=600,300");
        }
        node4: BFSRouter {
            address = 4;
            @display("p=200,500");
        }
        node5: BFSRouter {
            address = 5;
            @display("p=600,500");
        }
    connections:
        // Simple mesh topology
        node0.port++ <--> { delay = 50ms; datarate = 1Mbps; @display("ls=blue,3"); } <--> node1.port++;
        node0.port++ <--> { delay = 80ms; datarate = 1Mbps; @display("ls=green,2"); } <--> node2.port++;
        node1.port++ <--> { delay = 100ms; datarate = 1Mbps; @display("ls=green,2"); } <--> node3.port++;
        node2.port++ <--> { delay = 70ms; datarate = 1Mbps; @display("ls=red,2"); } <--> node3.port++;
        node2.port++ <--> { delay = 90ms; datarate = 1Mbps; @display("ls=red,2"); } <--> node4.port++;
        node3.port++ <--> { delay = 60ms; datarate = 1Mbps; @display("ls=red,2"); } <--> node5.port++;
        node4.port++ <--> { delay = 50ms; datarate = 1Mbps; @display("ls=orange,1"); } <--> node5.port++;
        node1.port++ <--> { delay = 120ms; datarate = 1Mbps; @display("ls=gray,1,da"); } <--> node4.port++;
        
        // Controller connections (SDN-style star topology)
        node0.toController <--> controller.toRouter[0];
        node1.toController <--> controller.toRouter[1];
        node2.toController <--> controller.toRouter[2];
        node3.toController <--> controller.toRouter[3];
        node4.toController <--> controller.toRouter[4];
        node5.toController <--> controller.toRouter[5];
}
