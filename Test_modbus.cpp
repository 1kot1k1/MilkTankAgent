#include "ModbusManager.h"

#include <iostream>

int main()
{
    Device device;
    device.name = "Test Scale";
    device.port = "/dev/ttyUSB0";
    device.baudRate = 115200;
    device.address = 1;
    device.enabled = true;

    ModbusManager modbus;

    const PollResult result = modbus.poll(device);

    if (!result.success)
    {
        std::cout << "Poll FAILED: " << result.error << std::endl;
        return 1;
    }

    std::cout << "Poll OK" << std::endl;
    std::cout << "weight: " << result.weight << std::endl;
    std::cout << "status: " << static_cast<int>(result.status) << std::endl;
    std::cout << "stable: " << (result.stable ? "true" : "false") << std::endl;
    std::cout << "overload: " << (result.overload ? "true" : "false") << std::endl;

    return 0;
}