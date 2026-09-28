#pragma once

#include <string>

enum class DeviceEventType
{
    Online,
    Offline,
    Disabled,

    Added,
    Removed,
    Updated,

    WeightChanged,
    Overload,
    Unstable
};

struct DeviceEvent
{
    DeviceEventType type = DeviceEventType::Updated;

    std::string uuid;
    std::string name;

    double weight = 0.0;
    bool stable = false;
    bool overload = false;
};