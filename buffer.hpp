#pragma once
#include <cstdint>
#include <vector>
#include <iostream>

/**
 * General-purpose circular buffer.
 */
class CircularBuffer
{
public:
    std::vector<uint8_t> buffer;
    uint32_t capacity; 

    uint32_t readPos;               
    uint32_t writePos;  

    void initialise(uint32_t bufferCapacity);

    /**
     * Write `N` bytes from `inBuffer` to the circular buffer.
     * 
     * Optionally write `offset` bytes into the buffer 
     * (i.e. `offset` bytes ahead of the write pointer).
     */
    bool write(std::vector<uint8_t> &inBuffer, int N, int offset);

    /**
     * Read `N` bytes from the circular buffer into `outBuffer`.
     * 
     * Optionally read from `offset` bytes into the buffer
     * (i.e. `offset` bytes ahead of the read pointer).
     */
    bool read(std::vector<uint8_t> &outBuffer, int N, int offset);

    /**
     * Returns num. bytes able to be read after the read pointer.
     */
    uint32_t availableToRead();

    /**
     * Returns num. bytes able to be written after the write pointer.
     */
    uint32_t availableToWrite();
};

namespace CircularBufferTests
{
    /**
     * Populates `buffer` with `N` random bytes.
     */
    void populateRandomBuffer(std::vector<uint8_t> &buffer, int N);

    void testSimpleReadWrite();
    void testReadWriteWithOffset();
    void testCapacityReached();

    void runAll();
};