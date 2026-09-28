#pragma once

#include <vector>
#include <string>
#include "Device.h"
#include "PollResult.h"

class DeviceManager
{
public:

    void setDevices(const std::vector<Device>& devices);

    void addDevice(const Device& device);

    void removeDevice(const std::string& uuid);

    void updateDevice(const Device& device);
    bool setState(
        const std::string& uuid,
        DeviceState state);
    Device* findDevice(const std::string& uuid);
    void updatePollResult(
        const std::string& uuid,
        const PollResult& result);

    const std::vector<Device>& devices() const;

private:

    std::vector<Device> m_devices;
};