#pragma once

#include <string>
#include <map>
#include <unordered_set>
#include <sstream>
#include <cstdint>
#include <iostream>
#include <vector>

#define DEFAULT_MSS 536

namespace SystemUtils
{
    /**
     * Retreive MTU from network interface `interface_name`.
     */
    int getMTU(std::string interface_name);
};

namespace Time
{
    /**
     * Retreive 32-bit unix epoch time.
     */
    uint32_t getUnixEpochTime();
}

namespace PrintUtils {

    /**
     * Pretty-print std::vector<T>
     */
    template<typename T>
    std::string printVector(const std::vector<T>& vec) 
    {
        std::ostringstream oss;
        oss << "[ ";
        for (size_t i = 0; i < vec.size(); ++i) 
        {
            oss << static_cast<int>(vec[i]);
            if (i != vec.size() - 1) {
                oss << ", ";
            }
        }
        oss << " ]";
        return oss.str();
    }

    /**
     * Pretty-print std::unordered_set<T>
     */
    template<typename T>
    void printUnorderedSet(const std::unordered_set<T>& set) 
    {
        std::cout << "{ ";
        int cnt = 0;
        for (auto &el : set)
        {
            std::cout << el;
            if (cnt++ != set.size() - 1)
                std::cout << ", ";
        }
        std::cout << " }" << std::endl;
    }

    /**
     * Pretty-print std::map<K, V>
     */
    template <typename K, typename V>
    void printMap(const std::map<K, V>& m) 
    {
        std::cout << "{\n";
        for (const auto& pair : m) {
            std::cout << "  " << pair.first << ": " << pair.second << "\n";
        }
        std::cout << "}\n";
    }
}