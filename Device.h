#pragma once

#include <cstdint>
#include <optional>
#include <string>

enum class DeviceState
{
    Unknown,
    Online,
    Offline,
    Disabled
};

struct Device
{
    bool operator==(
        const Device& other) const
    {
        return
            name == other.name &&
            uuid == other.uuid &&
            serialNumber == other.serialNumber &&
            address == other.address &&
            enabled == other.enabled &&
            port == other.port &&
            baudRate == other.baudRate;
    }

    DeviceState state =
        DeviceState::Unknown;

    double lastWeight = 0.0;

    bool stable = false;
    bool overload = false;
    bool success = false;

    std::string lastError;

    std::string name;
    std::string uuid;

    std::optional<std::uint32_t> serialNumber;

    int address = 0;
    bool enabled = false;

    std::string port;

    int baudRate = 115200;
};