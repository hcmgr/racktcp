#pragma once

#include <cstdint>
#include <string>

/**
 * IP header
 */
struct __attribute__((packed)) IpHeader
{
    uint8_t ihl:4;          // header length, in 32-bit words
    uint8_t version:4;
    uint8_t tos;
    uint16_t totLen;
    uint16_t id;
    uint16_t fragOff;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint32_t saddr;
    uint32_t daddr;

    uint16_t calculateChecksum();
    std::string toString();
    void networkToHostOrder();
};