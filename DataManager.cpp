#include "DataManager.h"

void DataManager::update(
    const Device& device,
    const PollResult& result)
{
    DeviceData& data = m_devices[device.address];

    data.address = device.address;
    data.name = device.name;
    data.uuid = device.uuid;
    data.lastUpdate = std::time(nullptr);

    if (result.success)
    {
        data.weight = result.weight;
        data.online = true;
        data.status = result.status;
        data.stable = result.stable;
        data.overload = result.overload;
    }
    else
    {
        data.online = false;
    }
}

std::vector<DeviceData> DataManager::devices() const
{
    std::vector<DeviceData> result;

    for (const auto& pair : m_devices)
        result.push_back(pair.second);

    return result;
}