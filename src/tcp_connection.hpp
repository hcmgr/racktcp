#pragma once
#include <cstdint>
#include <memory>
#include <netinet/ip.h>

#include "tcp.hpp"

/**
 * Represents a single TCP connection
 */
class TcpConnection
{
private:

    /* Transmission control block (TCB) of the connection */
    std::shared_ptr<Tcb> tcb;

    // SegmentThread segmentThread; 

    void initialise();
    
public:
    /**
     * Opens a TCP connection.
     * 
     * Optionally provide `destAddr` and `destPort` for an active connection 
     * (i.e. connect to existing peer), omit them for a passive connection 
     * (i.e. listen for incoming connections)
     */
    static TcpConnection open(
        in_addr_t sourceAddr,
        uint16_t sourcePort,
        in_addr_t destAddr = 0,
        uint16_t destPort = 0
    );

    /**
     * Send `N` bytes from `buffer` to the tcp peer.
     * 
     * NOTE: also takes 'push' and 'urgent' flags
     */
    void send(void *buffer, int N);

    /**
     * Read `N` bytes into `buffer` from the tcp peer.
     */
    void recv(void *buffer, int N);

    /**
     * Close the tcp connection.
     */
    void close();
};