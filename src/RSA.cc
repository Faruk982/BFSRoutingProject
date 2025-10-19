#include "RSA.h"

RSA::RSA() {
    // Generate two distinct primes (smaller range for network simulation performance)
    p = generatePrime(100, 500);
    do {
        q = generatePrime(100, 500);
    } while (q == p);
    
    // Calculate modulus and Euler's totient
    n = p * q;
    phi = (p - 1) * (q - 1);
    
    // Choose public exponent (commonly 65537, but adjust if needed)
    e = 65537;
    while (std::gcd(e, phi) != 1) {
        e += 2;
    }
    
    // Calculate private exponent
    d = modInverse(e, phi);
    
    EV_DEBUG << "RSA Key Pair Generated: n=" << n << ", e=" << e << ", d=" << d << "\n";
}

RSA::RSA(long long prime1, long long prime2) {
    if (!isPrime(prime1) || !isPrime(prime2)) {
        throw cRuntimeError("Both numbers must be prime");
    }
    
    p = prime1;
    q = prime2;
    n = p * q;
    phi = (p - 1) * (q - 1);
    
    e = 65537;
    while (std::gcd(e, phi) != 1) {
        e += 2;
    }
    
    d = modInverse(e, phi);
}

bool RSA::isPrime(long long num) {
    if (num <= 1) return false;
    if (num <= 3) return true;
    if (num % 2 == 0 || num % 3 == 0) return false;
    
    for (long long i = 5; i * i <= num; i += 6) {
        if (num % i == 0 || num % (i + 2) == 0)
            return false;
    }
    return true;
}

long long RSA::generatePrime(long long min, long long max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long long> dis(min, max);
    
    long long num;
    int attempts = 0;
    do {
        num = dis(gen);
        attempts++;
        if (attempts > 1000) {
            throw cRuntimeError("Could not generate prime after 1000 attempts");
        }
    } while (!isPrime(num));
    
    return num;
}

long long RSA::extendedGCD(long long a, long long b, long long &x, long long &y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    
    long long x1, y1;
    long long gcd = extendedGCD(b, a % b, x1, y1);
    
    x = y1;
    y = x1 - (a / b) * y1;
    
    return gcd;
}

long long RSA::modExp(long long base, long long exp, long long mod) {
    long long result = 1;
    base = base % mod;
    
    while (exp > 0) {
        if (exp % 2 == 1)
            result = (result * base) % mod;
        
        exp = exp >> 1;
        base = (base * base) % mod;
    }
    
    return result;
}

long long RSA::modInverse(long long a, long long m) {
    long long x, y;
    long long gcd = extendedGCD(a, m, x, y);
    
    if (gcd != 1) {
        throw cRuntimeError("Modular inverse doesn't exist");
    }
    
    return (x % m + m) % m;
}

std::pair<long long, long long> RSA::getPublicKey() {
    return {e, n};
}

std::pair<long long, long long> RSA::getPrivateKey() {
    return {d, n};
}

long long RSA::encrypt(long long message) {
    if (message >= n) {
        throw cRuntimeError("Message too large for modulus: %lld >= %lld", message, n);
    }
    if (message < 0) {
        throw cRuntimeError("Message cannot be negative");
    }
    return modExp(message, e, n);
}

long long RSA::decrypt(long long ciphertext) {
    return modExp(ciphertext, d, n);
}

long long RSA::encryptRoutingInfo(long long value) {
    // Ensure value is within valid range
    if (value < 0 || value >= n) {
        EV_WARN << "Routing value " << value << " out of range, clamping\n";
        value = std::max(0LL, std::min(value, n - 1));
    }
    return encrypt(value);
}

long long RSA::decryptRoutingInfo(long long encrypted) {
    return decrypt(encrypted);
}

long long RSA::encryptDouble(double value) {
    // Convert double to long by multiplying by 1000 (3 decimal places precision)
    long long intValue = static_cast<long long>(value * 1000);
    
    // Handle negative values by using offset
    long long offset = 1000000;
    intValue += offset;
    
    if (intValue >= n) {
        EV_WARN << "Double value too large, clamping\n";
        intValue = n - 1;
    }
    
    return encrypt(intValue);
}

double RSA::decryptDouble(long long encrypted) {
    long long decrypted = decrypt(encrypted);
    
    // Remove offset
    long long offset = 1000000;
    decrypted -= offset;
    
    // Convert back to double
    return static_cast<double>(decrypted) / 1000.0;
}

long long RSA::encryptWithKey(long long message, long long publicKey, long long modulus) {
    if (message >= modulus) {
        throw cRuntimeError("Message too large for modulus");
    }
    
    // Modular exponentiation
    long long result = 1;
    long long base = message % modulus;
    long long exp = publicKey;
    
    while (exp > 0) {
        if (exp % 2 == 1)
            result = (result * base) % modulus;
        exp = exp >> 1;
        base = (base * base) % modulus;
    }
    
    return result;
}
