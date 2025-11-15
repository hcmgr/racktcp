#include <openssl/sha.h>
#include <string>

#include "crypto.hpp"

namespace Crypto {

    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"

    /**
     * Computes 32-bit truncation of `data`'s SHA256 hash.
     */
    uint32_t sha256_32(const void* data, size_t length) 
    {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;

        SHA256_Init(&sha256);
        SHA256_Update(&sha256, data, length);
        SHA256_Final(hash, &sha256);

        // extract first 4 bytes (32-bit truncation)
        return (uint32_t(hash[0]) << 24) | 
            (uint32_t(hash[1]) << 16) | 
            (uint32_t(hash[2]) << 8)  | 
            (uint32_t(hash[3]));
    }

    ///////////////////////////////////////////////
    // Family of functions to compute the 32-bit //
    // truncation of `input`'s SHA256 hash.      //
    ///////////////////////////////////////////////

    uint32_t sha256_32(uint32_t input) 
    {
        return sha256_32(&input, sizeof(input));
    }

    uint32_t sha256_32(const std::string& input) 
    {
        return sha256_32(input.c_str(), input.size());
    }
}
