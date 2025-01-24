#include <unistd.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <mutex>
#include <memory>
#include <string.h>
#include <iterator>
#include <sstream>
#include <iomanip>

#include "tcp.hpp"
#include "ip.hpp"
#include "buffer.hpp"
#include "utils.hpp"

// ////////////////////////////////////////////
// // iphdr methods
// ////////////////////////////////////////////
// std::string ipToString(struct iphdr header) {
//     std::ostringstream oss;

//     oss << "IPv4 Header:" << "\n";
//     oss << "  Version: " << static_cast<int>(header.version) << "\n";
//     oss << "  Header Length: " << static_cast<int>(header.ihl) * 4 << " bytes\n"; // IHL is in 32-bit words
//     oss << "  Type of Service: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(header.tos) << std::dec << "\n";
//     oss << "  Total Length: " << ntohs(header.tot_len) << " bytes\n";
//     oss << "  Identification: 0x" << std::hex << std::setw(4) << std::setfill('0') << ntohs(header.id) << std::dec << "\n";
//     oss << "  Flags and Fragment Offset: 0x" << std::hex << std::setw(4) << std::setfill('0') << ntohs(header.frag_off) << std::dec << "\n";
//     oss << "  Time to Live: " << static_cast<int>(header.ttl) << "\n";
//     oss << "  Protocol: " << static_cast<int>(header.protocol) << "\n";
//     oss << "  Header Checksum: 0x" << std::hex << std::setw(4) << std::setfill('0') << ntohs(header.check) << std::dec << "\n";

//     // Convert source and destination addresses to human-readable format
//     struct in_addr src, dest;
//     src.s_addr = header.saddr;
//     dest.s_addr = header.daddr;

//     oss << "  Source Address: " << inet_ntoa(src) << "\n";
//     oss << "  Destination Address: " << inet_ntoa(dest) << "\n";

//     return oss.str();
// }

// std::string ipToStringDone(struct iphdr header) {
//     std::ostringstream oss;

//     oss << "####################################" << "\n";
//     oss << "IPv4 Header" << "\n\n";
//     oss << "  Version: " << static_cast<int>(header.version) << "\n";
//     oss << "  Header Length: " << static_cast<int>(header.ihl) * 4 << " bytes\n"; // IHL is in 32-bit words
//     oss << "  Type of Service: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(header.tos) << std::dec << "\n";
//     oss << "  Total Length: " << header.tot_len << " bytes\n";
//     oss << "  Identification: 0x" << std::hex << std::setw(4) << std::setfill('0') << header.id << std::dec << "\n";
//     oss << "  Flags and Fragment Offset: 0x" << std::hex << std::setw(4) << std::setfill('0') << header.frag_off << std::dec << "\n";
//     oss << "  Time to Live: " << static_cast<int>(header.ttl) << "\n";
//     oss << "  Protocol: " << static_cast<int>(header.protocol) << "\n";
//     oss << "  Header Checksum: 0x" << std::hex << std::setw(4) << std::setfill('0') << header.check << std::dec << "\n";
    

//     // Convert source and destination addresses to human-readable format
//     struct in_addr src, dest;
//     src.s_addr = htonl(header.saddr);
//     dest.s_addr = htonl(header.daddr);

//     oss << "  Source Address: " << inet_ntoa(src) << "\n";
//     oss << "  Destination Address: " << inet_ntoa(dest) << "\n";

//     oss << "####################################" << "\n";

//     return oss.str();
// }

// void ipNetworkToHost(struct iphdr *header)
// {
//     header->tot_len = ntohs(header->tot_len);
//     header->id = ntohs(header->id);
//     header->frag_off = ntohs(header->frag_off);
//     header->check = ntohs(header->check);
//     header->saddr = ntohl(header->saddr);
//     header->daddr = ntohl(header->daddr);
// }

////////////////////////////////////////////
// tcphdr methods
////////////////////////////////////////////
std::string TcpHeader::toString()
{
    std::ostringstream oss;

    oss << "TCP Header" << "\n\n";
    oss << "  Source Port: " << sourcePort << "\n";
    oss << "  Destination Port: " << destPort << "\n";

    oss << "  Sequence Number: " << seqNum << "\n";
    oss << "  Acknowledgment Number: " << ackNum << "\n";

    oss << "  Data Offset: " << static_cast<int>(doff) << " (words)" << "\n";

    oss << "  Flags: [";
    if (fin) oss << "FIN: " << fin << ", ";
    if (syn) oss << "SYN: " << syn << ", ";
    if (rst) oss << "RST: " << rst << ", ";
    if (psh) oss << "PSH: " << psh << ", ";
    if (ack) oss << "ACK: " << ack << ", ";
    if (urg) oss << "URG: " << urg;
    oss << "]" << "\n";

    oss << "  Window Size: " << window << "\n";
    oss << "  Checksum: 0x" << std::hex << std::setw(4) << std::setfill('0') << checksum << std::dec << "\n";
    oss << "  Urgent Pointer: " << urgPtr << "\n";

    return oss.str();
}

void TcpHeader::networkToHostOrder()
{
    sourcePort = ntohs(sourcePort);
    destPort = ntohs(destPort);
    seqNum = ntohl(seqNum);
    ackNum = ntohl(ackNum);
    window = ntohs(window);
    checksum = ntohs(checksum);
    urgPtr = ntohs(urgPtr);
}

////////////////////////////////////////////
// Tcb methods
////////////////////////////////////////////
SendStream::SendStream(uint32_t bufferCapacity)
    : sendBuffer(bufferCapacity) { }

RecvStream::RecvStream(uint32_t bufferCapacity)
    : recvBuffer(bufferCapacity) { }

Tcb::Tcb(
    uint32_t sendBufferCapacity,
    uint32_t recvBufferCapacity
)
    : sendStream(sendBufferCapacity),
      recvStream(recvBufferCapacity) { }

#define MTU (1 << 15)

/**
 * Represents a raw IP packet (i.e. ip header, tcp header and tcp payload).
 */
struct Packet
{
    struct IpHeader ipHeader;
    struct TcpHeader tcpHeader;
    std::vector<uint8_t> payload;

    uint32_t combinedHeaderSize()
    {
        return sizeof(ipHeader) + sizeof(tcpHeader);
    }

    static Packet readPacket(std::vector<uint8_t>& buffer, uint32_t packetSize)
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

        // // payload
        if (packet.ipHeader.totLen != packetSize)
            throw std::runtime_error("Packet sizes don't agree");

        uint16_t payloadSize = packet.ipHeader.totLen - packet.combinedHeaderSize();
        packet.payload.resize(payloadSize);
        std::copy(it, it + payloadSize, packet.payload.data());

        return packet;
    }

    std::string toString(bool showIpHeader = true, bool showPayload = false)
    {
        std::ostringstream oss;

        oss << "####################################" << "\n";
        if (showIpHeader)
            oss << ipHeader.toString() << "\n";
        oss << tcpHeader.toString();
        if (showPayload)
        {
            std::string payloadStr(reinterpret_cast<char*>(payload.data()));
            oss << "\nPayload" << "\n\n";
            oss << "  " << payloadStr << "\n";
        }
        oss << "####################################" << "\n";
        return oss.str();
    }
};

/**
 * Represents the TCP thread responsible for sending/receiving packets,
 * and updating the state accordingly.
 * 
 * Given we are doing this in userland, this separate thread mimics the role
 * that the kernel process plays in usual TCP stacks.
 */
class SegmentThread
{
public:
    std::shared_ptr<Tcb> tcb;

    SegmentThread(std::shared_ptr<Tcb> tcb)
    {
        this->tcb = tcb;
    }

    void startThread()
    {
        run();
    }

    int initialiseRawSocket()
    {
        /**
         * Open a raw IP socket.
         */
        int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
        if (sock < 0)
        {
            perror("Failed socket creation");
            return -1;
        }

        /**
         * Bind to destination addr
         */
        struct sockaddr_in destAddr;
        destAddr.sin_family = AF_INET;
        destAddr.sin_addr.s_addr = tcb->destAddr;
        socklen_t destAddrLen = sizeof(destAddr);

        if (bind(sock, (struct sockaddr*)&destAddr, destAddrLen))
        {
            perror("Failed bind");
            return -1;
        }

        return sock;
    }

    ssize_t retreiveNextPacket(int sock, std::vector<uint8_t>& packetBuffer)
    {
        ssize_t packetSize = recvfrom(
            sock, 
            packetBuffer.data(), 
            packetBuffer.size(),
            0, 
            NULL, 
            NULL
        );

        if (packetSize < 0 || packetSize > packetBuffer.size())
        {
            perror("Packet receive failed");
            return -1;
        }

        return packetSize;
    }

    void run()
    {
        int sock = initialiseRawSocket();
        if (sock < 0) 
            return;

        std::vector<uint8_t> packetBuffer(MTU);

        while (1)
        {
            ssize_t packetSize = retreiveNextPacket(sock, packetBuffer);
            
            if (packetSize < 0) 
                return;
            
            Packet packet = Packet::readPacket(packetBuffer, packetSize);
            if (packet.tcpHeader.sourcePort == 8100 && packet.tcpHeader.destPort == 8101)
            {
                std::cout << packet.toString(true, true) << std::endl;
            }

            // switch(tcb->state)
            // {
            //     case CLOSED:
            //     case LISTEN:
            //     case SYN_SENT:
            //     case SYN_RECEIVED:
            //         break;
            // }
        }
    }
};

int main()
{
    std::string ip = "10.126.0.2";

    auto tcb = std::make_shared<Tcb>(4096, 4096);
    tcb->state = ESTABLISHED;
    tcb->destAddr = inet_addr(ip.c_str());

    SegmentThread st(tcb);
    st.startThread();
}