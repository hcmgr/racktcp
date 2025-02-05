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
#include <random>

#include "tcp.hpp"
#include "ip.hpp"
#include "utils.hpp"
#include "crypto.hpp"
#include "config.hpp"
#include "packet.hpp"
#include "stream.hpp"

////////////////////////////////////////////
// TcpHeader methods
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
    sourcePort = ntohs(sourcePort);
    destPort = ntohs(destPort);
    seqNum = ntohl(seqNum);
    ackNum = ntohl(ackNum);
    window = ntohs(window);
    checksum = ntohs(checksum);
    urgPtr = ntohs(urgPtr);
}

void TcpHeader::hostToNetworkOrder()
{
    sourcePort = htons(sourcePort);
    destPort = htons(destPort);
    seqNum = htonl(seqNum);
    ackNum = htonl(ackNum);
    window = htons(window);
    checksum = htons(checksum);
    urgPtr = htons(urgPtr);
}



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
     * Retreive a packet from `packetBuffer`, which holds the most
     * recent packet from the raw IP socket.
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
     * Send `packet` to the connection's raw IP socket.
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

    bool packetValid(Packet &packet)
    {
        return (
            packet.ipHeader.saddr == inet_addr(tcb->destAddr.c_str()) &&
            packet.ipHeader.daddr == inet_addr(tcb->sourceAddr.c_str()) &&
            packet.tcpHeader.sourcePort == tcb->destPort &&
            packet.tcpHeader.destPort == tcb->sourcePort
        );
    }

    void processAck(uint32_t ackNum)
    {
        // duplicate ACK - ignore
        if (ackNum < tcb->sendStream.UNA)
            return;

        // ACK'ing bytes not yet sent - send duplicate ACK
        if (tcb->sendStream.NXT < ackNum)
        {
            /** TODO: send duplicate ACK */
            return;
        }

        /**
         * Valid ACK. Update UNA and remove now-ack'd segments 
         * from rtx queue.
         * 
         * TODO:
         */
        tcb->sendStream.UNA = ackNum;
    }

    void processRecveivedPayload(Packet &packet)
    {
        TcpHeader &segHdr = packet.tcpHeader;

        // TODO
        // verify SEG.SEQ is valid in the context of RCV.NXT and RCV.WND
        // for now, no out-of-order reception
        if (segHdr.seqNum != tcb->recvStream.NXT)
        {
            std::cout << "Invalid sequence number received" << std::endl;
            return;
        }

        // write payload to recv buffer
        bool res = tcb->recvStream.writePayloadToRecvBuffer(packet.payload);
        if (!res)
            return;

        // TODO
        // notify user that some bytes are available to read
    }

    void closedHandler()
    {
        std::cout << "CLOSED: sending initial SYN" << std::endl;
        std::cout << tcb->sendStream.toString() << std::endl;

        /**
         * Send initial SYN packet
         */
        TcpHeader h = {};
        h.sourcePort = tcb->sourcePort;
        h.destPort = tcb->destPort;
        h.doff = sizeof(h) / 4;

        // advertise ISS and window size
        h.SYN = 1;
        h.seqNum = tcb->sendStream.ISS;
        h.window = tcb->recvStream.WND;

        Packet packet;
        packet.tcpHeader = h;

        sendPacket(packet);

        // transition to SYN-RECEIVED state
        tcb->state = SYN_SENT;
    }

    void listenHandler(Packet &packet)
    {
        std::cout << tcb->sendStream.toString() << std::endl;

        TcpHeader &segHdr = packet.tcpHeader;

        // received initial SYN
        if (segHdr.SYN)
        {
            std::cout << "LISTEN: received SYN" << std::endl;

            // initialse recv stream based on peer's ISS and window size
            tcb->recvStream.IRS = segHdr.seqNum;
            tcb->recvStream.NXT = segHdr.seqNum + 1;
            tcb->recvStream.WND = segHdr.window;

            /**
             * Build and send SYN-ACK
             */
            TcpHeader hdr = {};
            hdr.sourcePort = tcb->sourcePort;
            hdr.destPort = tcb->destPort;
            hdr.doff = sizeof(hdr) / 4;

            // advertise ISS and window size
            hdr.SYN = 1;
            hdr.seqNum = tcb->sendStream.ISS;
            hdr.window = tcb->recvStream.WND;

            // acknowledge peer's ISS
            hdr.ACK = 1;
            hdr.ackNum = tcb->recvStream.NXT;

            Packet packet;
            packet.tcpHeader = hdr;

            sendPacket(packet);

            // transition to SYN-RECEIVED state
            tcb->state = SYN_RECEIVED;
        }

        else 
        {
            /**
             * TODO: 
             * 
             * As we are in the LISTEN state, we reply to any
             * non-SYN packets with a RST. 
             */
            std::cout << "LISTEN: non-SYN received, send RST" << std::endl;
        }
    }

    void synSentHandler(Packet &packet)
    {
        TcpHeader &segHdr = packet.tcpHeader;

        // received SYN-ACK
        if (segHdr.SYN && segHdr.ACK)
        {
            std::cout << "SYN-SENT: received SYN-ACK" << std::endl;

            /**
             * Validate ack. num. is correct.
             * 
             * NOTE: 
             * 
             * We don't send any data whilst connection is being
             * established, so: 
             *      SEG.ACK == SND.NXT == ISS + 1
             * should hold.
             */
            if (!(segHdr.ackNum == tcb->sendStream.NXT && 
                  segHdr.ackNum == tcb->sendStream.ISS + 1))
            {
                std::cout << "SYN-SENT: bad ack, send RST, -> CLOSED" << std::endl;
                std::cout << segHdr.ackNum << " " 
                          << tcb->sendStream.NXT << " " 
                          << tcb->sendStream.ISS + 1 
                          << std::endl;
                return;
            }

            // initialise recv stream based on peer's ISS and window size
            tcb->recvStream.IRS = segHdr.seqNum;
            tcb->recvStream.NXT = segHdr.seqNum + 1;
            tcb->recvStream.WND = segHdr.window;

            /**
             * Send ACK
             */
            TcpHeader hdr = {};
            hdr.sourcePort = tcb->sourcePort;
            hdr.destPort = tcb->destPort;
            hdr.doff = sizeof(hdr) / 4;

            // acknowledge peer's ISS and window size
            hdr.ACK = 1;
            hdr.ackNum = tcb->recvStream.NXT;

            Packet packet;
            packet.tcpHeader = hdr;

            sendPacket(packet);

            // transition to established state
            tcb->state = ESTABLISHED;
            std::cout << "Connection established" << std::endl;
        }
    }

    void synReceivedHandler(Packet &packet)
    {
        TcpHeader &segHdr = packet.tcpHeader;

        // received ACK
        if (segHdr.ACK)
        {
            std::cout << "SYN-RECEIVED: received ACK" << std::endl;

            /**
             * Validate ack. num. is correct. 
             * 
             * NOTE:
             * 
             * See synSentHandler (above) for explanation of validation.
             */
            if (!(segHdr.ackNum == tcb->sendStream.NXT && 
                  segHdr.ackNum == tcb->sendStream.ISS + 1))
            {
                std::cout << "SYN-RECEIVED: bad ack, send RST, -> LISTEN" << std::endl;
                std::cout << segHdr.ackNum << " " 
                          << tcb->sendStream.NXT << " " 
                          << tcb->sendStream.ISS + 1 
                          << std::endl;
                return;
            }

            tcb->state = ESTABLISHED;
            std::cout << "Connection established" << std::endl;
        }
    }

    void establishedHandler(Packet &packet)
    {
        std::cout << "ESTABLISHED: received packet" << std::endl;

        TcpHeader &segHdr = packet.tcpHeader;

        /**
         * Handle RST set (i.e. reset request).
         */
        if (segHdr.RST)
        {
            return;
        }

        /**
         * Handle SYN set.
         * 
         * This is considered an error in the ESTABLISHED state,
         * so we send a reset.
         * 
         * TODO:
         */
        if (segHdr.SYN)
        {
            return;
        }
        
        /**
         * Handle 'no-ACK' (i.e. ACK bit off).
         * 
         * This is considered an error in the ESTABLISHED state,
         * so we drop the segment.
         */
        if (!segHdr.ACK)
        {
            return;
        }

        /* process acknowlegement */
        processAck(segHdr.ackNum);

        /* process payload, if any */
        if (packet.payloadSize() > 0)
            processRecveivedPayload(packet);
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
                
                std::cout << packet.toString(false, true) << std::endl;
            }
            
            switch(tcb->state)
            {
                case CLOSED:
                    closedHandler();
                    break;
                case LISTEN:
                    listenHandler(packet);
                    break;
                case SYN_SENT:
                    synSentHandler(packet);
                    break;
                case SYN_RECEIVED:
                    synReceivedHandler(packet);
                    break;
                case ESTABLISHED:
                    establishedHandler(packet);
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
    auto tcb = std::make_shared<Tcb>();

#ifdef THREAD1
    tcb->state = CLOSED;
    tcb->sourceAddr = ip;
    tcb->sourcePort = 8100;
    tcb->destAddr = ip;
    tcb->destPort = 8101;
#else // THREAD2
    tcb->state = LISTEN;
    tcb->sourceAddr = ip;
    tcb->sourcePort = 8101;
    tcb->destAddr = ip;
    tcb->destPort = 8100;
#endif

    SegmentThread st(tcb);
    st.startThread();
}