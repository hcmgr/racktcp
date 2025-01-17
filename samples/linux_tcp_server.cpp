#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int serverExample()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Socket failed");
        return -1;
    }

    std::string ip = "10.126.0.2";
    int port = 8102;

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = inet_addr(ip.c_str());

    if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0) 
    {
        perror("Bind failed");
        return -1;
    }
    
    // listen for incoming connections
    if (listen(sock, 3) < 0) {
        perror("Listen");
        return -1;
    }
    
    std::cout << "Server is listening on port 8102" << std::endl;

    // accept incoming connection
    socklen_t addrlen;
    int new_socket = accept(sock, (struct sockaddr *)&address, &addrlen);
    if (new_socket < 0)
    {
        perror("Accept");
        return -1;
    }

    // receive message from client
    char buffer[1024] = {0};
    int valread = read(new_socket, buffer, 1024);
    std::cout << "Message from client: " << buffer << std::endl;

    // close both
    close(new_socket);
    close(sock);

    return 0;
}