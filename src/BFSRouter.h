#include <omnetpp.h>
#include "BFSRoutingPacket_m.h"
#include <map>
#include <sstream>

using namespace omnetpp;

class BFSRouter : public cSimpleModule {
  private:
    int myAddress;
    
    // Local neighbor discovery (only direct neighbors)
    std::map<int, double> directNeighbors;  // neighborId -> delay
    bool linkStateSent;  // Have we sent link state to controller?
    
    // Forwarding table from controller (destination -> nextHop node)
    std::map<int, int> forwardingTable;
    bool forwardingTableReady;  // Have we received forwarding table from controller?
    
    // RSA Keys - Each router has its own key pair
    long long publicKey;   // e (this router's public exponent)
    long long privateKey;  // d (this router's private exponent - SECRET!)
    long long modulus;     // n (this router's modulus)
    
    // Statistics
    int packetsForwarded;
    int packetsDelivered;
    int routeDiscoveriesSent;
    int astarRoutesLearned;
    int encryptedPacketsSent;
    int encryptedPacketsReceived;
    
    // Network topology for heuristic (simplified - could be learned)
    int numNodes;
    
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    
    // Central Controller Communication
    void discoverLocalTopology();  // Learn about directly connected neighbors
    void sendLinkStateToController();  // Send link state to central controller (includes public key)
    void receiveForwardingTable(BFSRoutingPacket *pkt);  // Receive forwarding table from controller
    void displayForwardingTable();  // Display received forwarding table
    int findGateToNeighbor(int neighborId);  // Find gate index to specific neighbor
    
    // RSA Key Generation and Decryption
    void generateRSAKeys();  // Generate this router's RSA key pair
    long long rsaDecrypt(long long ciphertext);  // Decrypt using this router's private key
};
