#pragma once

#include <string>

#define DEFAULT_MSS 536

namespace SystemUtils
{
    /**
     * Retreive MTU from network interface `interface_name`.
     */
    int getMTU(std::string interface_name);
};