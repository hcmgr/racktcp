#pragma once

#include <cstdint>
#include <vector>

#include "buffer.hpp"

/**
 * Represents the send stream of the TCP connection.
 */
struct SendStream
{
    /* stream parameters */
    uint32_t UNA;       // una pointer (seq nums to ack)
    uint32_t NXT;       // next pointer (seq nums to send)
    uint32_t WND;       // send window
    uint32_t ISS;       // initial send sequence number

    /* send buffer */
    CircularBuffer sendBuffer;

    /* TOADD: retransmission queue */

    /* Param constructor */
    SendStream(uint32_t bufferCapacity);

    /**
     * Read into the to-be-sent payload `payload` the maximum number of 
     * available bytes in our send buffer.
     */
    bool readPayloadFromSendBuffer(std::vector<uint8_t> &payload);

    /**
     * Generate a new initital sequence number (ISS).
     */
    uint32_t generateISS();

    std::string toString();
};

/**
 * Represents the receive stream of the TCP connection.
 */
struct RecvStream
{
    /* stream paramters */
    uint32_t NXT;       // next pointer (seq nums to receive)
    uint32_t WND;       // receive window
    uint32_t IRS;       // initial receive sequence number (connection peer's ISS)

    /* recv buffer */
    CircularBuffer recvBuffer;

    /**
     * Param constructor
     */
    RecvStream(uint32_t bufferCapacity);

    /**
     * Write received `payload` to the receive buffer.
     */
    bool writePayloadToRecvBuffer(std::vector<uint8_t> &payload);

    std::string toString();
};