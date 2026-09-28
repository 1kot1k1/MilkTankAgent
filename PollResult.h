#pragma once

#include <cstdint>
#include <string>

struct PollResult
{
    bool success = false;

    double weight = 0;

    std::uint8_t status = 0;

    bool stable = false;

    bool overload = false;

    std::string error;
};