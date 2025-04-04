#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "tcp.hpp"

void displayIpTcpHeaders(struct iphdr* ip_header, struct TcpHeader* tcp_header)
{
    std::cout << "Received Packet:" << std::endl;
    std::cout << "Source IP: " << inet_ntoa(*(struct in_addr*)&ip_header->saddr) << std::endl;
    std::cout << "Destination IP: " << inet_ntoa(*(struct in_addr*)&ip_header->daddr) << std::endl;
    std::cout << "Source Port: " << ntohs(tcp_header->sourcePort) << std::endl;
    std::cout << "Destination Port: " << ntohs(tcp_header->destPort) << std::endl;
    std::cout << "Total length: " << ntohs(ip_header->tot_len) << std::endl;
    std::cout << "Sequence Number: " << ntohl(tcp_header->seqNum) << std::endl;
    std::cout << "Acknowledgment Number: " << ntohl(tcp_header->ackNum) << std::endl;
    std::cout << "Flags: ";

    if (tcp_header->SYN) std::cout << "SYN ";
    if (tcp_header->ACK) std::cout << "ACK ";
    if (tcp_header->FIN) std::cout << "FIN ";
    if (tcp_header->RST) std::cout << "RST ";
    if (tcp_header->PSH) std::cout << "PSH ";
    if (tcp_header->URG) std::cout << "URG ";

    std::cout << std::endl;
    std::cout << "-----------------------------------" << std::endl;
}

// Function to parse TCP headers
void parseHeaders(const char* packet, struct iphdr **ip_header_ptr, struct TcpHeader **tcp_header_ptr) 
{
    *ip_header_ptr = (struct iphdr*)packet;
    *tcp_header_ptr = (struct TcpHeader*)(packet  + (*ip_header_ptr)->ihl * 4);
}


int manualReceive() 
{
    // Create a raw socket to capture TCP packets
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    char buffer[4096];

    std::string ip = "10.49.0.5";

    struct sockaddr_in destAddr;
    destAddr.sin_family = AF_INET;
    destAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    socklen_t destAddrLen = sizeof(destAddr);

    if (bind(sock, (struct sockaddr*)&destAddr, destAddrLen))
    {
        perror("Bind failed");
        return 0;
    }

    struct sockaddr_in sourceAddr;
    socklen_t sourceAddrLen;

    std::cout << "Listening for packets on port 8101..." << std::endl;

    while (true) 
    {
        // receive packets
        ssize_t packet_size = recvfrom(sock, buffer, sizeof(buffer), 0, 
                                       (struct sockaddr*)&sourceAddr, 
                                        &sourceAddrLen);
        if (packet_size < 0) 
        {
            perror("Packet receive failed");
            continue;
        }

        struct iphdr *ip_header;
        struct TcpHeader *tcp_header;

        parseHeaders(buffer, &ip_header, &tcp_header); 
        displayIpTcpHeaders(ip_header, tcp_header);
        if (ntohs(tcp_header->sourcePort) == 8100 || ntohs(tcp_header->destPort) == 8101)
        {
            // displayIpTcpHeaders(ip_header, tcp_header);
        }
    }

    close(sock);
    return 0;
}

int main()
{
    manualReceive();
}
