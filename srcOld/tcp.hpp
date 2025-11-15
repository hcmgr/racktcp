#pragma once

#include <cstdint>
#include <mutex>
#include <netinet/ip.h>

#include "buffer.hpp"
#include "config.hpp"
#include "stream.hpp"

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
    void hostToNetworkOrder();
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