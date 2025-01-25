#pragma once

#include <cstdint>
#include <mutex>
#include <netinet/ip.h>

#include "buffer.hpp"
#include "config.hpp"

/**
 * TCP header
 */
struct __attribute__((packed)) TcpHeader
{
    uint16_t sourcePort;
    uint16_t destPort;
    uint32_t seqNum;
    uint32_t ackNum;

    // data offset
    uint16_t res1:4;
	uint16_t doff:4;

    // flags
	uint16_t FIN:1;
	uint16_t SYN:1;
	uint16_t RST:1;
	uint16_t PSH:1;
	uint16_t ACK:1;
	uint16_t URG:1;
	uint16_t RES2:2;

    uint16_t window;
    uint16_t checksum;
    uint16_t urgPtr;

    std::string toString();

    void networkToHostOrder();
};

/**
 * Represents the set of all TCP connection states
 */
enum ConnectionState
{
    CLOSED,
    LISTEN,
    SYN_SENT,
    SYN_RECEIVED,
    ESTABLISHED,
    CLOSE_WAIT,
    FIN_WAIT_1,
    FIN_WAIT_2,
    CLOSING,
    LAST_ACK,
    TIME_WAIT
};

/**
 * Represents the send stream of the TCP connection.
 */
struct SendStream
{
    /* stream parameters */
    uint32_t UNA;       // una pointer (to-ack)
    uint32_t NXT;       // next pointer (to-send)
    uint32_t WND;       // send window
    uint32_t ISS;       // initial send sequence number

    /* send buffer */
    CircularBuffer sendBuffer;

    /* TOADD: retransmission queue */

    /* Param constructor */
    SendStream(uint32_t bufferCapacity);

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
    uint32_t NXT;       // next pointer (to-receive)
    uint32_t WND;       // receive window
    uint32_t IRS;       // initial receive sequence number (connection peer's ISS)

    /* recv buffer */
    CircularBuffer recvBuffer;
    uint32_t bufferCapacity;

    /**
     * Param constructor
     */
    RecvStream(uint32_t bufferCapacity);

    std::string toString();
};

/**
 * Transmission Control Block (TCB)
 */
struct Tcb
{
    SendStream sendStream;
    RecvStream recvStream;

    std::string sourceAddr;
    std::string destAddr;
    uint16_t sourcePort;
    uint16_t destPort;
    
    ConnectionState state;

    std::mutex mutex;

    /* Default constructor */
    Tcb()
    : sendStream(SEND_BUFFER_CAPACITY),
      recvStream(RECV_BUFFER_CAPACITY) {}
};