#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void displayIpTcpHeaders(struct iphdr* ip_header, struct tcphdr* tcp_header)
{
    std::cout << "Received Packet:" << std::endl;
    std::cout << "Source IP: " << inet_ntoa(*(struct in_addr*)&ip_header->saddr) << std::endl;
    std::cout << "Destination IP: " << inet_ntoa(*(struct in_addr*)&ip_header->daddr) << std::endl;
    std::cout << "Source Port: " << ntohs(tcp_header->source) << std::endl;
    std::cout << "Destination Port: " << ntohs(tcp_header->dest) << std::endl;
    std::cout << "Sequence Number: " << ntohl(tcp_header->seq) << std::endl;
    std::cout << "Acknowledgment Number: " << ntohl(tcp_header->ack_seq) << std::endl;
    std::cout << "Flags: ";

    if (tcp_header->syn) std::cout << "SYN ";
    if (tcp_header->ack) std::cout << "ACK ";
    if (tcp_header->fin) std::cout << "FIN ";
    if (tcp_header->rst) std::cout << "RST ";
    if (tcp_header->psh) std::cout << "PSH ";
    if (tcp_header->urg) std::cout << "URG ";

    std::cout << std::endl;
    std::cout << "-----------------------------------" << std::endl;
}

// Function to parse TCP headers
void parseHeaders(const char* packet, struct iphdr **ip_header_ptr, struct tcphdr **tcp_header_ptr) 
{
    *ip_header_ptr = (struct iphdr*)packet;
    *tcp_header_ptr = (struct tcphdr*)(packet  + (*ip_header_ptr)->ihl * 4);
}

int main() 
{
    // Create a raw socket to capture TCP packets
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    char buffer[4096];
    struct sockaddr_in source_addr;
    socklen_t addr_len = sizeof(source_addr);

    std::cout << "Listening for packets on port 8101..." << std::endl;

    while (true) {
        // Receive packets
        ssize_t packet_size = recvfrom(sock, buffer, sizeof(buffer), 0, 
                                       (struct sockaddr*)&source_addr, &addr_len);
        if (packet_size < 0) {
            perror("Packet receive failed");
            continue;
        }

        struct iphdr *ip_header;
        struct tcphdr *tcp_header;

        parseHeaders(buffer, &ip_header, &tcp_header); 
        if (ntohs(tcp_header->source) == 8100 || ntohs(tcp_header->dest) == 8101)
            displayIpTcpHeaders(ip_header, tcp_header);
    }

    close(sock);
    return 0;
}
