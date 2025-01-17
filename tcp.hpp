#pragma once
#include <cstdint>

struct TcpHeader
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
};