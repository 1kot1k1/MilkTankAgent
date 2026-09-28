#pragma once

#include <cstdint>
#include <ctime>
#include <string>


struct DeviceData
{
    int id = 0;

    int address = 0;


    std::string name;

    std::string uuid;

    double weight = 0.0;

    bool online = false;

    std::uint8_t status = 0;

    bool stable = false;

    bool overload = false;



    std::time_t lastUpdate = 0;
};