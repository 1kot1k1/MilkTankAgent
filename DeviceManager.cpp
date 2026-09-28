#include "DeviceManager.h"
#include "PollResult.h"

void DeviceManager::setDevices(
    const std::vector<Device>& devices)
{
    m_devices = devices;
}

void DeviceManager::addDevice(
    const Device& device)
{
    m_devices.push_back(device);
}

void DeviceManager::removeDevice(
    const std::string& uuid)
{
    for (auto it = m_devices.begin();
        it != m_devices.end();
        ++it)
    {
        if (it->uuid == uuid)
        {
            m_devices.erase(it);
            return;
        }
    }
}

void DeviceManager::updateDevice(
    const Device& device)
{
    for (auto& d : m_devices)
    {
        if (d.uuid == device.uuid)
        {
            d = device;
            return;
        }
    }
}
bool DeviceManager::setState(
    const std::string& uuid,
    DeviceState state)
{
    for (auto& d : m_devices)
    {
        if (d.uuid != uuid)
            continue;

        if (d.state == state)
            return false;

        d.state = state;

        return true;
    }

    return false;
}
Device* DeviceManager::findDevice(
    const std::string& uuid)
{
    for (auto& d : m_devices)
    {
        if (d.uuid == uuid)
        {
            return &d;
        }
    }

    return nullptr;
}
void DeviceManager::updatePollResult(
    const std::string& uuid,
    const PollResult& result)
{
    Device* device = findDevice(uuid);

    if (!device)
        return;

    device->success = result.success;
    device->lastWeight = result.weight;
    device->stable = result.stable;
    device->overload = result.overload;
    device->lastError = result.error;
}
const std::vector<Device>& DeviceManager::devices() const
{
    return m_devices;
}