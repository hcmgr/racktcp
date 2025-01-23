#pragma once

#include <cstdint>
#include <mutex>
#include <netinet/ip.h>

#include "buffer.hpp"

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
	uint16_t fin:1;
	uint16_t syn:1;
	uint16_t rst:1;
	uint16_t psh:1;
	uint16_t ack:1;
	uint16_t urg:1;
	uint16_t res2:2;

    uint16_t window;
    uint16_t checksum;
    uint16_t urgPtr;

    // default constructor
    TcpHeader() = default;

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
 * Represents the send stream
 */
struct SendStream
{
    uint32_t UNA;       // una pointer (to-ack)
    uint32_t NXT;       // next pointer (to-send)
    uint32_t WND;       // send window
    uint32_t ISS;       // initial send sequence number

    /* send buffer */
    CircularBuffer sendBuffer;

    /* TOADD: retransmission queue */

    /* Param constructor */
    SendStream(uint32_t bufferCapacity);
};

/**
 * Represents the receive stream
 */
struct RecvStream
{
    uint32_t NXT;       // next pointer (to-receive)
    uint32_t WND;       // receive window
    uint32_t IRS;       // initial receive sequence number

    /* recv buffer */
    CircularBuffer recvBuffer;

    /* Param constructor */
    RecvStream(uint32_t bufferCapacity);
};

/**
 * Transmission Control Block (TCB)
 */
struct Tcb
{
    SendStream sendStream;
    RecvStream recvStream;

    in_addr_t sourceAddr;
    in_addr_t destAddr;
    uint16_t sourcePort;
    uint16_t destPort;
    
    ConnectionState state;

    std::mutex mutex;

    Tcb(
        uint32_t sendBufferCapacity,
        uint32_t recvBufferCapacity
    );
};