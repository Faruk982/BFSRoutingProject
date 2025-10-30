package bfsroutingproject_new;

simple BFSRouter
{
    parameters:
        int address;
        bool useEncryption = default(true);  // Enable RSA encryption for routing info
        @display("i=block/routing");
    gates:
        inout port[];
        inout toController;  // Connection to central controller
}

simple CentralController
{
    parameters:
        int totalNodes = default(9);
        @display("i=block/cogwheel");
    gates:
        inout toRouter[9];  // Connections to all routers (expanded to 9)
}

network BFSRoutingNetwork
{
    parameters:
        int numNodes = 9;  // Expanded to 9 nodes
        @display("bgb=1400,900");
    submodules:
        // Central Controller (SDN-style)
        controller: CentralController {
            @display("p=470,521");
        }

        // Expanded 9-node topology with more spacing
        node0: BFSRouter {
            address = 0;
            @display("p=150,100");
        }
        node1: BFSRouter {
            address = 1;
            @display("p=700,100");
        }
        node2: BFSRouter {
            address = 2;
            @display("p=150,400");
        }
        node3: BFSRouter {
            address = 3;
            @display("p=700,400");
        }
        node4: BFSRouter {
            address = 4;
            @display("p=150,700");
        }
        node5: BFSRouter {
            address = 5;
            @display("p=700,700");
        }
        node6: BFSRouter {
            address = 6;
            @display("p=1200,200");
        }
        node7: BFSRouter {
            address = 7;
            @display("p=1200,500");
        }
        node8: BFSRouter {
            address = 8;
            @display("p=1200,800");
        }
    connections:
        // Original mesh topology (nodes 0-5)
        node0.port++ <--> {  delay = 50ms; datarate = 1Mbps;@display("ls=blue,3"); } <--> node1.port++;
        node0.port++ <--> {  delay = 80ms; datarate = 1Mbps;@display("ls=green,2"); } <--> node2.port++;
        node1.port++ <--> {  delay = 100ms; datarate = 1Mbps;@display("ls=green,2"); } <--> node3.port++;
        node2.port++ <--> {  delay = 70ms; datarate = 1Mbps;@display("ls=red,2"); } <--> node3.port++;
        node2.port++ <--> {  delay = 90ms; datarate = 1Mbps;@display("ls=red,2"); } <--> node4.port++;
        node3.port++ <--> {  delay = 60ms; datarate = 1Mbps;@display("ls=red,2"); } <--> node5.port++;
        node4.port++ <--> {  delay = 50ms; datarate = 1Mbps;@display("ls=orange,1"); } <--> node5.port++;
        node1.port++ <--> {  delay = 120ms; datarate = 1Mbps;@display("ls=gray,1,da"); } <--> node4.port++;

        // New connections: ONLY 6-7 and 7-8 (as requested)
        node6.port++ <--> {  delay = 65ms; datarate = 1Mbps;@display("ls=purple,2"); } <--> node7.port++;
        node7.port++ <--> {  delay = 75ms; datarate = 1Mbps;@display("ls=purple,2"); } <--> node8.port++;

        // New connections: ONLY 6-7 and 7-8 (as requested)
        node3.port++ <--> {  delay = 110ms; datarate = 1Mbps;@display("ls=red,2"); } <--> node7.port++;
        node4.port++ <--> {  delay = 125ms; datarate = 1Mbps;@display("ls=orange,2"); } <--> node8.port++;

        // Controller connections (SDN-style star topology - all nodes connected to controller)
        node0.toController <--> controller.toRouter[0];
        node1.toController <--> controller.toRouter[1];
        node2.toController <--> controller.toRouter[2];
        node3.toController <--> controller.toRouter[3];
        node4.toController <--> controller.toRouter[4];
        node5.toController <--> controller.toRouter[5];
        node6.toController <--> controller.toRouter[6];
        node7.toController <--> controller.toRouter[7];
        node8.toController <--> controller.toRouter[8];
}
