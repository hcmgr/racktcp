#include <cstdint>
#include <vector>
#include <string.h>
#include <iostream>
#include <random>
#include <cassert>

#include "buffer.hpp"

#include "test_utils.hpp"

////////////////////////////////////////////
// CircularBuffer methods
////////////////////////////////////////////
void CircularBuffer::initialise(uint32_t bufferCapacity)
{
    capacity = bufferCapacity;
    buffer.resize(capacity);
    readPos = 0;
    writePos = 0;
}

/**
 * Write `N` bytes from `inBuffer` to the circular buffer.
 * 
 * Optionally write `offset` bytes into the buffer 
 * (i.e. `offset` bytes ahead of the write pointer).
 */
bool CircularBuffer::writeN(std::vector<uint8_t> &inBuffer, int N, int offset)
{
    assert(N <= inBuffer.size());
    if (availableToWrite() < N)
        return false;

    for (int i = 0; i < N; i++)
    {
        int ind = (readPos + i) % capacity;
        buffer[ind] = inBuffer[i];
    }

    writePos = (writePos + N) % capacity;
    return true;
}

/**
 * Read `N` bytes from the circular buffer into `outBuffer`.
 * 
 * Optionally read from `offset` bytes into the buffer
 * (i.e. `offset` bytes ahead of the read pointer).
 */
bool CircularBuffer::readN(std::vector<uint8_t> &outBuffer, int N, int offset)
{
    assert(N <= outBuffer.size());
    if (availableToRead() < N)
        return false;
    
    for (int i = 0; i < N; i++)
    {
        int ind = (readPos + i) % capacity;
        outBuffer[i] = buffer[ind];
    }

    readPos = (readPos + N) % capacity;
    return true;
}

/**
 * Returns num. bytes able to be read after the read pointer.
 */
uint32_t CircularBuffer::availableToRead()
{
    return (writePos - readPos + capacity) % capacity;
}

/**
 * Returns num. bytes able to be written after the write pointer.
 */
uint32_t CircularBuffer::availableToWrite()
{
    return capacity - availableToRead();
}

////////////////////////////////////////////
// CircularBuffer tests
////////////////////////////////////////////

namespace CircularBufferTests
{
    /**
     * Populates `buffer` with `N` random bytes.
     */
    void populateRandomBuffer(std::vector<uint8_t> &buffer, int N)
    {
        std::random_device rd;                          // gives random seed
        std::mt19937 gen(rd());                         // init. pseudo random number gen. with seed
        std::uniform_int_distribution<> dis(65, 90);    // init. distribution over uppercase ASCIIs

        buffer.resize(N);
        for (int i = 0; i < N; i++)
            buffer[i] = static_cast<uint8_t>(dis(gen)); // sample distribution
        return;
    }

    void testSimpleReadWrite()
    {
        /**
         * Initialise
         */
        uint32_t capacity = 4000;
        CircularBuffer cb;
        cb.initialise(capacity);

        ASSERT_THAT(cb.availableToRead() == 0);
        ASSERT_THAT(cb.availableToWrite() == cb.capacity);

        /**
         * Write N bytes, try and read back out
         */
        int N = 3000;
        std::vector<uint8_t> inBuffer;
        populateRandomBuffer(inBuffer, N);

        cb.writeN(inBuffer, N, 0);
        ASSERT_THAT(cb.availableToRead() == N);
        ASSERT_THAT(cb.availableToWrite() == capacity - N);

        std::vector<uint8_t> outBuffer(N);
        cb.readN(outBuffer, N, 0);
        ASSERT_THAT(inBuffer == outBuffer);

        /**
         * Write M more bytes such that pointers wrap around
         */
        int M = 2000;
        populateRandomBuffer(inBuffer, M);

        cb.writeN(inBuffer, M, 0);

        outBuffer.resize(M);
        cb.readN(outBuffer, M, 0);
        ASSERT_THAT(inBuffer == outBuffer);
        ASSERT_THAT(cb.writePos == (N + M) % cb.capacity);
        ASSERT_THAT(cb.readPos == (N + M) % cb.capacity);
    }

    /**
     * TODO:
     */
    void testReadWriteWithOffset()
    {

    }

    void testCapacityReached()
    {
        int capacity = 2000;
        CircularBuffer cb;
        cb.initialise(capacity);

        int N = 1500;
        std::vector<uint8_t> inBuffer;
        populateRandomBuffer(inBuffer, N);
        cb.writeN(inBuffer, N, 0);

        int M = 500;
        std::vector<uint8_t> outBuffer(M);
        cb.readN(outBuffer, M, 0);

        ASSERT_THAT(cb.availableToWrite() == 1000);

        /**
         * Write X > availableToWrite() bytes
         */
        int X = 1100;
        bool res = cb.writeN(inBuffer, X, 0);
        ASSERT_THAT(!res);
        ASSERT_THAT(cb.writePos == 1500 && cb.readPos == 500);
    }

    void runAll()
    {
        std::cerr << "###################################" << std::endl;
        std::cerr << "Buffer Tests" << std::endl;
        std::cerr << "###################################" << std::endl;

        std::vector<std::pair<std::string, std::function<void()>>> tests = 
        {
            TEST(testSimpleReadWrite),
            TEST(testCapacityReached)
        };

        for (auto &[name, func] : tests)
        {
            TestUtils::runTest(name, func);
        }
    }
};

/*
ideas / todo
    - implement read/write at particular offset
    - maybe need a way to tell if parts are occupied
        - bitmap?
    - maybe make writes more efficient with segmented memcpy() ops?
        - i.e. first half / second half if it wraps around?
*/