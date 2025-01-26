#include <cstdint>
#include <string>
#include <iomanip>
#include <sstream>
#include <arpa/inet.h>
#include <iostream>

#include "ip.hpp"

////////////////////////////////////////////
// IpHeader methods
////////////////////////////////////////////

/**
 * Calculate checksum of the IP header.
 * 
 * From RFC 791:
 * 
 * "The checksum field is the 16 bit one's complement of 
 * the one's complement sum of all 16 bit words in the header.  
 * For purposes of computing the checksum, the value of the 
 * checksum field is zero."
 */
uint16_t IpHeader::calculateChecksum()
{
    // checksum calculation done on network ordered header

    uint16_t *addr = (uint16_t*)this;
    uint32_t sum = 0;
    int count = ihl * 4;
    while (count > 1) {
        sum += * addr++;
        count -= 2;
    }
    //if any bytes left, pad the bytes and add
    if(count > 0) {
        sum += ((*addr)&htons(0xFF00));
    }

    //Fold sum to 16 bits: add carrier to result
    while (sum>>16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    // this->networkToHostOrder();

    //one's complement
    sum = ~sum;
    return ((unsigned short)sum);
}

std::string IpHeader::toString() 
{
    std::ostringstream oss;

    oss << "IPv4 Header" << "\n\n";
    oss << "  Version: " << static_cast<int>(version) << "\n";
    oss << "  Header Length: " << static_cast<int>(ihl) * 4 << " bytes\n"; // IHL is in 32-bit words
    oss << "  Type of Service: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(tos) << std::dec << "\n";
    oss << "  Total Length: " << totLen << " bytes\n";
    oss << "  Identification: 0x" << std::hex << std::setw(4) << std::setfill('0') << id << std::dec << "\n";
    oss << "  Flags and Fragment Offset: 0x" << std::hex << std::setw(4) << std::setfill('0') << fragOff << std::dec << "\n";
    oss << "  Time to Live: " << static_cast<int>(ttl) << "\n";
    oss << "  Protocol: " << static_cast<int>(protocol) << "\n";
    oss << "  Header Checksum: 0x" << std::hex << std::setw(4) << std::setfill('0') << checksum << std::dec << "\n";
    

    // Convert source and destination addresses to human-readable format
    struct in_addr src, dest;
    src.s_addr = saddr;
    dest.s_addr = daddr;

    oss << "  Source Address: " << inet_ntoa(src) << "\n";
    oss << "  Destination Address: " << inet_ntoa(dest) << "\n";

    return oss.str();
}

void IpHeader::networkToHostOrder()
{
    totLen = ntohs(totLen);
    id = ntohs(id);
    fragOff = ntohs(fragOff);
    checksum = ntohs(checksum);
}