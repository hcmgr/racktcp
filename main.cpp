#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "tcp.hpp"

// Function to calculate checksum
unsigned short calculateChecksum(unsigned short *ptr, int nbytes) {
    unsigned long sum;
    for (sum = 0; nbytes > 1; nbytes -= 2) {
        sum += *ptr++;
    }
    if (nbytes == 1) {
        sum += *(unsigned char *)ptr;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

int run()
{
    // Create a raw socket
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    std::string ip = "10.49.0.5";

    // Do NOT enable IP_HDRINCL (default behavior is kernel-generated IP header)

    // Prepare the TCP payload (no IP header needed here)
    char packet[4096];
    memset(packet, 0, sizeof(packet));

    // TCP header
    TcpHeader *tcpHeader = (TcpHeader *)packet;

    tcpHeader->sourcePort = htons(8100);
    tcpHeader->destPort = htons(8101);  
    tcpHeader->seqNum = htonl(0);     
    tcpHeader->ackNum = htonl(0); 

    tcpHeader->doff = 5;            

    // flags
    tcpHeader->fin = 0;
    tcpHeader->syn = 1;            
    tcpHeader->rst = 0;
    tcpHeader->psh = 0;
    tcpHeader->ack = 1;
    tcpHeader->urg = 1;

    tcpHeader->window = htons(5840);  
    tcpHeader->checksum = 0;             
    tcpHeader->urgPtr = 0;

    // Destination info
    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = htons(8101);
    dest.sin_addr.s_addr = inet_addr(ip.c_str());

    // Calculate checksum
    struct PseudoHeader {
        uint32_t source_address;
        uint32_t dest_address;
        uint8_t placeholder;
        uint8_t protocol;
        uint16_t tcp_length;
    } psh;

    psh.source_address = inet_addr(ip.c_str());
    psh.dest_address = inet_addr(ip.c_str());
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(sizeof(struct tcphdr));

    char pseudo_packet[4096];
    memcpy(pseudo_packet, &psh, sizeof(psh));
    memcpy(pseudo_packet + sizeof(psh), tcpHeader, sizeof(struct tcphdr));

    tcpHeader->checksum = calculateChecksum((unsigned short *)pseudo_packet, sizeof(psh) + sizeof(struct tcphdr));

    // Send packet
    if (sendto(sock, packet, sizeof(struct tcphdr), 0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
        perror("sendto failed");
        close(sock);
        return 1;
    }

    std::cout << "Packet sent successfully!" << std::endl;

    close(sock);
    return 0;
}

int main() 
{
    run();
}