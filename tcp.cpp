#include "tcp.hpp"

/**
 * Represents a single TCP connection
 */
class TcpStream
{
public:
    /**
     * Opens a TCP connection
     */
    static TcpStream open()
    {

    }

    /**
     * Send `N` bytes from `buffer` to the tcp peer.
     * 
     * NOTE: also takes 'push' and 'urgent' flags
     */
    void send(void *buffer, int N)
    {

    }

    /**
     * Read `N` bytes into `buffer` from the tcp peer.
     */
    void recv(void *buffer, int N)
    {

    }

    /**
     * Close the tcp connection.
     */
    void close()
    {

    }
};