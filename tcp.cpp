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

////////////////////////////////////////////
// TcpHeader methods
////////////////////////////////////////////
std::string TcpHeader::toString()
{
    std::ostringstream oss;

    oss << "TCP Header" << "\n\n";
    oss << "  Source Port: " << ntohs(sourcePort) << "\n";
    oss << "  Destination Port: " << ntohs(destPort) << "\n";

    oss << "  Sequence Number: " << seqNum << "\n";
    oss << "  Acknowledgment Number: " << ackNum << "\n";

    oss << "  Data Offset: " << static_cast<int>(doff) << " (words)" << "\n";

    oss << "  Flags: [";
    if (FIN) oss << "FIN: " << FIN << ", ";
    if (SYN) oss << "SYN: " << SYN << ", ";
    if (RST) oss << "RST: " << RST << ", ";
    if (PSH) oss << "PSH: " << PSH << ", ";
    if (ACK) oss << "ACK: " << ACK << ", ";
    if (URG) oss << "URG: " << URG;
    oss << "]" << "\n";

    oss << "  Window Size: " << window << "\n";
    oss << "  Checksum: 0x" << std::hex << std::setw(4) << std::setfill('0') << checksum << std::dec << "\n";
    oss << "  Urgent Pointer: " << urgPtr << "\n";

    return oss.str();
}

void TcpHeader::networkToHostOrder()
{
    // sourcePort = ntohs(sourcePort);
    // destPort = ntohs(destPort);
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

    static Packet deserialise(std::vector<uint8_t>& buffer, uint32_t packetSize)
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

    std::vector<uint8_t> serialise(bool includeIpHeader)
    {
        ssize_t size = sizeof(tcpHeader) + payload.size();
        if (includeIpHeader)
            size += sizeof(ipHeader);

        std::vector<uint8_t> buffer(size);
        auto it = buffer.begin();

        // ip header
        if (includeIpHeader)
        {
            memcpy(&(*it), &ipHeader, sizeof(ipHeader));
            it += sizeof(ipHeader);
        }

        // tcp header
        memcpy(&(*it), &tcpHeader, sizeof(tcpHeader));
        it += sizeof(tcpHeader);

        // payload
        memcpy(&(*it), payload.data(), payload.size());
        return buffer;
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
    SegmentThread(std::shared_ptr<Tcb> tcb)
    {
        this->tcb = tcb;
        this->sock = initialiseRawSocket();
        if (this->sock < 0)
            throw std::runtime_error("Failed socket creation");
        return;
    }

    void startThread()
    {
        run();
    }

private:
    /**
     * Transmission Control Block (TCB) of this connection.
     */
    std::shared_ptr<Tcb> tcb;

    /**
     * Raw IP socket of this connection.
     */
    int sock;

    /**
     * Opens and initialises this connection's raw IP socket.
     */
    int initialiseRawSocket()
    {
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
        destAddr.sin_addr.s_addr = inet_addr(tcb->sourceAddr.c_str());
        socklen_t destAddrLen = sizeof(destAddr);

        if (bind(sock, (struct sockaddr*)&destAddr, destAddrLen))
        {
            perror("Failed bind");
            return -1;
        }

        return sock;
    }

    /**
     * Retreive next packet from the given raw IP socket.
     */
    ssize_t retreivePacket(std::vector<uint8_t>& packetBuffer)
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

    /**
     * Send the given packet to the given raw IP socket.
     */
    ssize_t sendPacket(Packet &packet, bool includeIpHeader = false)
    {
        std::vector<uint8_t> packetBuffer = packet.serialise(includeIpHeader);

        // destination info
        struct sockaddr_in destAddr;
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(tcb->destPort);
        destAddr.sin_addr.s_addr = inet_addr(tcb->destAddr.c_str());

        ssize_t bytesSent = sendto(
            sock,
            packetBuffer.data(),
            packetBuffer.size(),
            0,
            (struct sockaddr*)&destAddr,
            sizeof(destAddr)
        );

        if (bytesSent < 0 || bytesSent != packetBuffer.size())
        {
            perror("sendto() failed");
            return -1;
        }
        return bytesSent;
    }

    void closedHandler()
    {
        std::cout << "Sending opening SYN" << std::endl;

        // send opening SYN
        Packet packet;

        TcpHeader h = {};
        h.sourcePort = tcb->sourcePort;
        h.destPort = tcb->destPort;
        h.seqNum = 1;
        h.ackNum = 1;
        h.doff = 1; // TODO
        h.SYN = 1;
        h.window = 2048;
        h.checksum = 420;

        packet.tcpHeader = h;

        std::string msg = "SYN'ing you";
        packet.payload.resize(msg.size());
        std::copy(
            packet.payload.begin(), 
            packet.payload.begin() + msg.size(), 
            msg.data()
        );

        sendPacket(packet);

        // transition to SYN-RECEIVED state
        tcb->state = SYN_SENT;
    }

    void listenHandler(TcpHeader segmentHeader)
    {
        // received SYN, send SYN-ACK
        if (segmentHeader.SYN)
        {
            std::cout << "Received SYN, sending SYN-ACK" << std::endl;

            // send SYN-ACK
            Packet packet;

            TcpHeader h = {};
            h.sourcePort = tcb->sourcePort;
            h.destPort = tcb->destPort;
            h.seqNum = 1;
            h.ackNum = 1;
            h.doff = 1; // TODO
            h.SYN = 1;
            h.ACK = 1;
            h.window = 1024;
            h.checksum = 69;

            packet.tcpHeader = h;

            std::string msg = "SYN-ACK'ing you";
            packet.payload.resize(msg.size());
            std::copy(
                packet.payload.begin(), 
                packet.payload.begin() + msg.size(), 
                msg.data()
            );

            sendPacket(packet);

            // transition to SYN-RECEIVED state
            tcb->state = SYN_RECEIVED;
        }
    }

    void synSentHandler(TcpHeader segmentHeader)
    {
        // received SYN-ACK of previously sent SYN
        // TODO: check ackNum lines up
        if (segmentHeader.SYN && segmentHeader.ACK && 1)
        {
            std::cout << "Received SYN-ACK, sending ACK, connection established" << std::endl;

            // send ACK
            Packet packet;

            TcpHeader h = {};
            h.sourcePort = tcb->sourcePort;
            h.destPort = tcb->destPort;
            h.seqNum = 1;
            h.ackNum = 1;
            h.doff = 1; // TODO
            h.ACK = 1;
            h.window = 4096;
            h.checksum = 111;

            packet.tcpHeader = h;

            std::string msg = "ACK'ing you";
            packet.payload.resize(msg.size());
            std::copy(
                packet.payload.begin(), 
                packet.payload.begin() + msg.size(), 
                msg.data()
            );

            sendPacket(packet);
            tcb->state = ESTABLISHED;
        }
    }

    void synReceivedHandler(TcpHeader segmentHeader)
    {
        // received ACK of previously sent SYN-ACK
        // TODO: check ackNum lines up
        if (segmentHeader.ACK && 1)
        {
            std::cout << "Received ACK, connection established" << std::endl;
            tcb->state = ESTABLISHED;
        }
    }

    bool packetValid(Packet &packet)
    {
        return (
            packet.ipHeader.saddr == inet_addr(tcb->destAddr.c_str()) &&
            packet.ipHeader.daddr == inet_addr(tcb->sourceAddr.c_str()) &&
            packet.tcpHeader.sourcePort == tcb->destPort &&
            packet.tcpHeader.destPort == tcb->sourcePort
        );
    }

    void run()
    {
        std::vector<uint8_t> packetBuffer(MTU);
        

        while (1)
        {
            Packet packet;

            bool waitForPacket = true;
            if (tcb->state == CLOSED)
                waitForPacket = false;

            if (waitForPacket)
            {
                ssize_t packetSize = retreivePacket(packetBuffer);

                if (packetSize < 0) 
                    return;
                
                packet = Packet::deserialise(packetBuffer, packetSize);

                if (!packetValid(packet))
                    continue;
                
                std::cout << packet.toString() << std::endl;
            }
            
            switch(tcb->state)
            {
                case CLOSED:
                    closedHandler();
                    break;
                case LISTEN:
                    listenHandler(packet.tcpHeader);
                    break;
                case SYN_SENT:
                    synSentHandler(packet.tcpHeader);
                    break;
                case SYN_RECEIVED:
                    synReceivedHandler(packet.tcpHeader);
                    break;
                case ESTABLISHED:
                    break;
                default:
                    // shouldn't reach here
                    throw std::runtime_error("Undefined state reached");
            }
        }
    }
};

int main()
{
    std::string ip = "10.126.0.2";

    auto tcb = std::make_shared<Tcb>(4096, 4096);

#ifdef THREAD1
    tcb->state = CLOSED;
    tcb->sourceAddr = ip;
    tcb->sourcePort = htons(8100);
    tcb->destAddr = ip;
    tcb->destPort = htons(8101);
    std::cout << "Thread1" << std::endl;
#else // THREAD2
    tcb->state = LISTEN;
    tcb->sourceAddr = ip;
    tcb->sourcePort = htons(8101);
    tcb->destAddr = ip;
    tcb->destPort = htons(8100);
    std::cout << "Thread2" << std::endl;
#endif

    SegmentThread st(tcb);
    st.startThread();
}