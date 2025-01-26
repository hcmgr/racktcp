#include <netinet/ip.h>
#include <memory>

#include "tcp_connection.hpp"
#include "tcp.hpp"

////////////////////////////////////////////
// TcpConnection private methods
////////////////////////////////////////////

void TcpConnection::initialise()
{
    this->tcb = std::make_shared<Tcb>();
}

////////////////////////////////////////////
// TcpConnection public methods
////////////////////////////////////////////

/**
 * Opens a TCP connection.
 * 
 * Optionally provide `destAddr` and `destPort` for an active connection 
 * (i.e. connect to existing peer), omit them for a passive connection 
 * (i.e. listen for incoming connections)
 */
TcpConnection TcpConnection::open(
    in_addr_t sourceAddr,
    uint16_t sourcePort,
    in_addr_t destAddr,
    uint16_t destPort
)
{
    TcpConnection tcpConnection;
    return tcpConnection;
}

/**
 * Send `N` bytes from `buffer` to the tcp peer.
 * 
 * NOTE: TODO: also takes 'push' and 'urgent' flags
 */
void TcpConnection::send(void *buffer, int N)
{

}

/**
 * Read `N` bytes into `buffer` from the tcp peer.
 */
void TcpConnection::recv(void *buffer, int N)
{

}

/**
 * Close the tcp connection.
 */
void TcpConnection::close()
{

}

////////////////////////////////////////////
// Run
////////////////////////////////////////////

void run()
{
    in_addr_t sourceAddr;
    uint16_t sourcePort;

    TcpConnection tcpConnection = TcpConnection::open(
        sourceAddr,
        sourcePort
    );
}

// int main()
// {
//     run();
//     return 0;
// }