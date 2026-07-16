#pragma once

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "../include/all.hpp"

inline dynRLSLP::DictionaryMode get_mode_from_string_int64(const std::string &mode_str)
{
    if (mode_str == "standard" || mode_str == "Standard" || mode_str == "STANDARD")
    {
        return dynRLSLP::DictionaryMode::Standard;
    }
    else if (mode_str == "lightweight" || mode_str == "Lightweight" || mode_str == "LIGHTWEIGHT")
    {
        return dynRLSLP::DictionaryMode::Lightweight;
    }
    else if (mode_str == "fast" || mode_str == "Fast" || mode_str == "FAST")
    {
        return dynRLSLP::DictionaryMode::Fast;
    }
    else
    {
        throw std::runtime_error("Invalid mode: " + mode_str + " (specify one of standard, lightweight, fast)");
    }
}

inline std::vector<int64_t> get_int64_alphabet_by_type(uint64_t alphabet_type)
{
    switch (alphabet_type)
    {
    case 1:
        return std::vector<int64_t>{10, 20};
    case 2:
        return std::vector<int64_t>{-3, 0, 7, 100};
    case 3:
        return std::vector<int64_t>{1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009};
    case 4:
    {
        std::vector<int64_t> alphabet;
        for (int64_t c = 100000; c < 100026; c++)
        {
            alphabet.push_back(c);
        }
        return alphabet;
    }
    default:
        throw std::runtime_error("Invalid alphabet type: " + std::to_string(alphabet_type) + " (specify a value in the range 1-4)");
    }
}

inline std::string int64_vector_to_string(const std::vector<int64_t> &values, size_t max_items = 20)
{
    std::ostringstream oss;
    oss << "[";
    size_t limit = std::min(values.size(), max_items);
    for (size_t i = 0; i < limit; i++)
    {
        if (i > 0)
        {
            oss << ", ";
        }
        oss << values[i];
    }
    if (values.size() > limit)
    {
        oss << ", ...";
    }
    oss << "]";
    return oss.str();
}

inline std::string mode_to_string_int64(dynRLSLP::DictionaryMode mode)
{
    return (mode == dynRLSLP::DictionaryMode::Standard) ? "standard" : (mode == dynRLSLP::DictionaryMode::Lightweight) ? "lightweight"
                                                                                                                          : "fast";
}
