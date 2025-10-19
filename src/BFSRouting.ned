package bfsroutingproject_new;

simple BFSRouter {
    parameters:
        int address;
        bool useEncryption = default(true);  // Enable RSA encryption for routing info
        @display("i=block/routing");
    gates:
        inout port[];
}

network BFSRoutingNetwork {
    parameters:
        int numNodes = 4;
        @display("bgb=800,400");
    submodules:
        node[numNodes]: BFSRouter {
            address = index;
            @display("p=100+index*200,200");
        }
    connections:
        // Chain topology: 0 <-> 1 <-> 2 <-> 3
        node[0].port++ <--> { delay = 100ms; datarate = 1Mbps; @display("ls=blue,3"); } <--> node[1].port++;
        node[1].port++ <--> { delay = 100ms; datarate = 1Mbps; @display("ls=blue,3"); } <--> node[2].port++;
        node[2].port++ <--> { delay = 100ms; datarate = 1Mbps; @display("ls=blue,3"); } <--> node[3].port++;
}