#include <cstdint>
#include <vector>
#include <string.h>
#include <iostream>
#include <random>

#include "test_utils.hpp"

/**
 * General-purpose circular buffer.
 */
class CircularBuffer
{
public:
    uint8_t *buffer;
    uint32_t capacity;             
    uint32_t readPos;               
    uint32_t writePos;  

    CircularBuffer(uint32_t capacity)
        : capacity(capacity), 
          readPos(0), 
          writePos(0)
    {
        buffer = new uint8_t[capacity];
    }

    ~CircularBuffer()
    {
        delete[] buffer;
    }

    /**
     * Write `N` bytes from `inBuffer` to the circular buffer.
     * 
     * Optionally write `offset` bytes into the buffer 
     * (i.e. `offset` bytes ahead of the write pointer).
     */
    bool write(void* inBuffer, int N, int offset)
    {
        if (availableToWrite() < N)
            return false;

        uint8_t* inBufferBytes = static_cast<uint8_t*>(inBuffer);

        for (int i = 0; i < N; i++)
        {
            int ind = (readPos + i) % capacity;
            buffer[ind] = inBufferBytes[i];
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
    bool read(void* outBuffer, int N, int offset)
    {
        if (availableToRead() < N)
            return false;
        
        uint8_t* outBufferBytes = static_cast<uint8_t*>(outBuffer);

        for (int i = 0; i < N; i++)
        {
            int ind = (readPos + i) % capacity;
            outBufferBytes[i] = buffer[ind];
        }

        readPos = (readPos + N) % capacity;
        return true;
    }

    /**
     * Returns num. bytes able to be read after the read pointer.
     */
    uint32_t availableToRead()
    {
        return (writePos - readPos + capacity) % capacity;
    }

    /**
     * Returns num. bytes able to be written after the write pointer.
     */
    uint32_t availableToWrite()
    {
        return capacity - availableToRead();
    }
};

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
        CircularBuffer cb(capacity);

        ASSERT_THAT(cb.availableToRead() == 0);
        ASSERT_THAT(cb.availableToWrite() == cb.capacity);

        /**
         * Write N bytes, try and read back out
         */
        int N = 3000;
        std::vector<uint8_t> inBuffer(N);
        populateRandomBuffer(inBuffer, N);

        cb.write(inBuffer.data(), N, 0);
        ASSERT_THAT(cb.availableToRead() == N);
        ASSERT_THAT(cb.availableToWrite() == capacity - N);

        std::vector<uint8_t> outBuffer(N);
        cb.read(outBuffer.data(), N, 0);
        ASSERT_THAT(inBuffer == outBuffer);

        /**
         * Write M more bytes such that pointers wrap around
         */
        int M = 2000;
        inBuffer.resize(M);
        populateRandomBuffer(inBuffer, M);

        cb.write(inBuffer.data(), M, 0);

        outBuffer.resize(M);
        cb.read(outBuffer.data(), M, 0);
        ASSERT_THAT(inBuffer == outBuffer);
        ASSERT_THAT(cb.writePos == (N + M) % cb.capacity);
        ASSERT_THAT(cb.readPos == (N + M) % cb.capacity);
    }

    void testReadWriteWithOffset()
    {

    }

    void testCapacityReached()
    {
        int capacity = 2000;
        CircularBuffer cb(capacity);

        int N = 1500;
        std::vector<uint8_t> inBuffer;
        populateRandomBuffer(inBuffer, N);
        cb.write(inBuffer.data(), N, 0);

        int M = 500;
        std::vector<uint8_t> outBuffer(M);
        cb.read(outBuffer.data(), M, 0);

        ASSERT_THAT(cb.availableToWrite() == 1000);

        /**
         * Write X > availableToWrite() bytes
         */
        int X = 1100;
        bool res = cb.write(inBuffer.data(), X, 0);
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

int main()
{
    CircularBufferTests::testSimpleReadWrite();
}

/*
ideas / todo
    - implement read/write at particular offset
    - maybe need a way to tell if parts are occupied
        - bitmap?
    - maybe make writes more efficient with segmented memcpy() ops?
        - i.e. first half / second half if it wraps around?
*/