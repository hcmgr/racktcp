#include <cstdint>

/**
 * State relating to the send stream.
 */
struct SendState
{
    uint32_t UNA;
    uint32_t NXT;
    uint32_t WND;

    /* send buffer */

    /* retransmission queue*/
};

/**
 * State relating to the receive stream.
 */
struct RecvState
{
    uint32_t NXT;
    uint32_t WND;

    /* recv buffer */
};


/**
 * Transmission Control Block (TCB)
 */
struct Tcb
{
    SendState sendState;
    RecvState recvState;

    /* source port */

    /* dest port */
};