#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int clientExample()
{
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) 
    {
        perror("Socket creation failed");
        return 1;
    }

    std::cout << "Socket created" << std::endl;

    std::string serverIp = "10.126.0.2";
    int serverPort = 8102;

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(serverPort);
    server_addr.sin_addr.s_addr = inet_addr(serverIp.c_str());

    if (connect(sock, (struct sockaddr*) &server_addr, sizeof(server_addr)))
    {
        perror("Connection failed");
        return 1;
    }

    std::cout << "Connection successful" << std::endl;

    std::string msg = "Hello there";

    // if (send(sock, msg.c_str(), msg.size(), 0, (struct sockaddr*) &server_addr, sizeof(server_addr)))
    if (send(sock, msg.c_str(), msg.size(), 0) < 0)
    {
        perror("Send failed");
        return 1;
    }

    std::cout << "Message sent successfully" << std::endl;

    close(sock);
    return 0;
}