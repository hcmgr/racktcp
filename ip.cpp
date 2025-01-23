#include <cstdint>
#include <string>
#include <iomanip>
#include <sstream>
#include <arpa/inet.h>

#include "ip.hpp"

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
    src.s_addr = htonl(saddr);
    dest.s_addr = htonl(daddr);

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
    saddr = ntohl(saddr);
    daddr = ntohl(daddr);
}