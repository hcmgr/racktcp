#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <cassert>

#include "packet.hpp"

#include "ip.hpp"
#include "tcp.hpp"

////////////////////////////////////////////
// Packet methods
////////////////////////////////////////////
uint32_t Packet::combinedHeaderSize()
{
    return sizeof(ipHeader) + sizeof(tcpHeader);
}

uint32_t Packet::payloadSize()
{
    assert(ipHeader.totLen >= combinedHeaderSize());
    return ipHeader.totLen - combinedHeaderSize();
}

Packet Packet::deserialise(std::vector<uint8_t>& buffer, uint32_t packetSize)
{
    Packet packet;

    size_t requiredSize = sizeof(packet.ipHeader) + sizeof(packet.tcpHeader);
    if (buffer.size() < requiredSize) 
        throw std::runtime_error("Packet too small to contain TCP/IP headers");

    auto it = buffer.begin();

    // ip header
    std::copy(it, it + sizeof(packet.ipHeader), reinterpret_cast<uint8_t*>(&packet.ipHeader));
    it += sizeof(packet.ipHeader);

    // tcp header
    std::copy(it, it + sizeof(packet.tcpHeader), reinterpret_cast<uint8_t*>(&packet.tcpHeader));
    it += sizeof(packet.tcpHeader);

    packet.ipHeader.networkToHostOrder();
    packet.tcpHeader.networkToHostOrder();

    // payload
    if (packet.ipHeader.totLen != packetSize)
        throw std::runtime_error("Packet sizes don't agree");

    uint16_t payloadSize = packet.ipHeader.totLen - packet.combinedHeaderSize();
    packet.payload.resize(payloadSize);
    std::copy(it, it + payloadSize, packet.payload.data());

    return packet;
}

std::vector<uint8_t> Packet::serialise(bool includeIpHeader)
{
    ssize_t size = sizeof(tcpHeader) + payload.size();
    if (includeIpHeader)
        size += sizeof(ipHeader);

    std::vector<uint8_t> buffer(size);
    auto it = buffer.begin();

    // ip header - perhaps networkToHostOrder call here?
    if (includeIpHeader)
    {
        memcpy(&(*it), &ipHeader, sizeof(ipHeader));
        it += sizeof(ipHeader);
    }

    // tcp header
    tcpHeader.hostToNetworkOrder();
    memcpy(&(*it), &tcpHeader, sizeof(tcpHeader));
    it += sizeof(tcpHeader);

    // payload
    if (payload.size())
        memcpy(&(*it), payload.data(), payload.size());

    return buffer;
}

std::string Packet::toString(bool showIpHeader, bool showPayload)
{
    std::ostringstream oss;

    oss << "####################################" << "\n";
    if (showIpHeader)
        oss << ipHeader.toString() << "\n";
    oss << tcpHeader.toString();
    if (showPayload && payload.size() > 0)
    {
        std::string payloadStr(
            reinterpret_cast<char*>(payload.data()),
            payload.size()
        );
        oss << "\nPayload" << "\n\n";
        oss << "  " << payloadStr << "\n";
    }
    oss << "####################################" << "\n";
    return oss.str();
}