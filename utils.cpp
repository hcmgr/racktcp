#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string>

#include "utils.hpp"

namespace SystemUtils
{
    /**
     * Retreive MTU from network interface `interface_name`.
     */
    int getMTU(std::string interface_name) 
    {
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) 
        {
            perror("Socket creation failed");
            return -1;
        }

        struct ifreq ifr;
        strncpy(ifr.ifr_name, interface_name.c_str(), IFNAMSIZ - 1);

        if (ioctl(sock, SIOCGIFMTU, &ifr) == -1) 
        {
            perror("ioctl failed");
            close(sock);
            return -1;
        }

        close(sock);
        return ifr.ifr_mtu;
    }
}

namespace Time
{
    /**
     * Retreive 32-bit unix epoch time.
     */
    uint32_t getUnixEpochTime()
    {
        return static_cast<uint32_t>(time(nullptr));
    }
}
