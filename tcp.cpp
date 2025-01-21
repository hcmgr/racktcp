#include <unistd.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <mutex>
#include <memory>

#include "tcp.hpp"
#include "buffer.hpp"

////////////////////////////////////////////
// Tcb methods
////////////////////////////////////////////
SendStream::SendStream(uint32_t bufferCapacity)
    : sendBuffer(bufferCapacity) { }

RecvStream::RecvStream(uint32_t bufferCapacity)
    : recvBuffer(bufferCapacity) { }

Tcb::Tcb(
    uint32_t sendBufferCapacity,
    uint32_t recvBufferCapacity
)
    : sendStream(sendBufferCapacity),
      recvStream(recvBufferCapacity) { }

/**
 * Represents the TCP thread responsible for sending/receiving packets,
 * and updating the state accordingly.
 * 
 * Given we are doing this in userland, this separate thread mimics the role
 * that the kernel process plays in usual TCP stacks.
 */
class SegmentThread
{
public:
    std::shared_ptr<Tcb> tcb;

    SegmentThread(std::shared_ptr<Tcb> tcb)
    {
        this->tcb = tcb;
    }

    void startThread()
    {
        run();
    }

    void run()
    {
        int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
        if (sock < 0)
        {
            std::cout << "socket could not be opened" << std::endl;
            return;
        }

        struct sockaddr_in destAddr;
        destAddr.sin_family = AF_INET;
        destAddr.sin_addr.s_addr = tcb->destAddr;
        socklen_t destAddrLen = sizeof(destAddr);

        if (bind(sock, (struct sockaddr*)&destAddr, destAddrLen))
        {
            perror("Failed bind");
            return;
        }

        while (1)
        {
            switch(tcb->state)
            {
                case CLOSED:

                case LISTEN:

                case SYN_SENT:

                case SYN_RECEIVED:
                    break;
            }

        }
    }
};

int main()
{
    auto tcb = std::make_shared<Tcb>(4096, 4096);
    tcb->state = ESTABLISHED;

    SegmentThread st(tcb);
    st.startThread();
}