#pragma once

#include <map>
#include <vector>

#include "DeviceData.h"
#include "Device.h"
#include "ModbusManager.h"

class DataManager
{
public:

    void update(const Device& device, const PollResult& result);

    std::vector<DeviceData> devices() const;

private:

    std::map<int, DeviceData> m_devices;
};