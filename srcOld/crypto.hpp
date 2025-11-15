#pragma once

#include <openssl/sha.h>
#include <string>
#include <cstdint>

namespace Crypto 
{
    ///////////////////////////////////////////////
    // Family of functions to compute the 32-bit //
    // truncation of `input`'s SHA256 hash.      //
    ///////////////////////////////////////////////

    uint32_t sha256_32(uint32_t input);
    uint32_t sha256_32(const std::string& input);
}