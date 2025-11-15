#pragma once

#include <string>
#include <vector>
#include <cstring>
#include <sstream>

#include "ip.hpp"
#include "tcp.hpp"

/**
 * Represents a raw IP packet (i.e. ip header, tcp header and tcp payload).
 */
struct Packet
{
    struct IpHeader ipHeader;
    struct TcpHeader tcpHeader;
    std::vector<uint8_t> payload;

    uint32_t combinedHeaderSize();

    uint32_t payloadSize();

    static Packet deserialise(std::vector<uint8_t>& buffer, uint32_t packetSize);

    std::vector<uint8_t> serialise(bool includeIpHeader);

    std::string toString(bool showIpHeader = true, bool showPayload = false);
};