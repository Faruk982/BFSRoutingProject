#ifndef RSA_H
#define RSA_H

#include <omnetpp.h>
#include <string>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <numeric>

using namespace omnetpp;

/**
 * RSA Encryption for Secure Routing Information Exchange
 * Used to encrypt routing metrics (hop count, costs, path info) between routers
 */
class RSA {
private:
    long long p, q;  // Prime numbers
    long long n;     // Modulus
    long long phi;   // Euler's totient function
    long long e;     // Public exponent
    long long d;     // Private exponent
    
    // Check if a number is prime
    bool isPrime(long long num);
    
    // Generate a random prime number
    long long generatePrime(long long min, long long max);
    
    // Extended Euclidean Algorithm
    long long extendedGCD(long long a, long long b, long long &x, long long &y);
    
    // Modular exponentiation: (base^exp) % mod
    long long modExp(long long base, long long exp, long long mod);
    
    // Find modular multiplicative inverse
    long long modInverse(long long a, long long m);

public:
    // Default constructor - generates random keys
    RSA();
    
    // Constructor with custom primes (for testing/deterministic keys)
    RSA(long long prime1, long long prime2);
    
    // Get public key (e, n)
    std::pair<long long, long long> getPublicKey();
    
    // Get private key (d, n)
    std::pair<long long, long long> getPrivateKey();
    
    // Encrypt a single number
    long long encrypt(long long message);
    
    // Decrypt a single number
    long long decrypt(long long ciphertext);
    
    // Encrypt routing information (hop count, costs)
    long long encryptRoutingInfo(long long value);
    
    // Decrypt routing information
    long long decryptRoutingInfo(long long encrypted);
    
    // Encrypt double value (multiply by 1000, encrypt as long)
    long long encryptDouble(double value);
    
    // Decrypt to double value
    double decryptDouble(long long encrypted);
    
    // Static method: Encrypt with someone else's public key
    static long long encryptWithKey(long long message, long long publicKey, long long modulus);
    
    // Get modulus (for sharing with other nodes)
    long long getModulus() { return n; }
    
    // Get public exponent (for sharing with other nodes)
    long long getPublicExponent() { return e; }
};

#endif
