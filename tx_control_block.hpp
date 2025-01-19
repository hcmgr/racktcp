#include <cstdint>

#include "buffer.hpp"

/**
 * Represents the send stream
 */
struct SendStream
{
    uint32_t UNA;
    uint32_t NXT;
    uint32_t WND;

    /* send buffer */
    CircularBuffer sendBuffer;

    /* retransmission queue */
};

/**
 * Represents the receive stream
 */
struct RecvStream
{
    uint32_t NXT;
    uint32_t WND;

    /* recv buffer */
    CircularBuffer recvBuffer;
};


/**
 * Transmission Control Block (TCB)
 */
struct Tcb
{
    SendStream sendStream;
    RecvStream recvStream;

    /* source port */

    /* dest port */
};