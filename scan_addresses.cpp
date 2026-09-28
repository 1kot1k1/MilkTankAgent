#include "ModbusManager.h"
#include "Logger.h"

#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{
    std::string port = "/dev/ttyUSB0";
    int baudRate = 115200;

    if (argc >= 2)
    {
        port = argv[1];
    }

    if (argc >= 3)
    {
        baudRate = std::stoi(argv[2]);
    }

    std::cout << "Scanning addresses 1-159 on "
        << port << " at " << baudRate << " baud." << std::endl;
    std::cout << "This will take a minute or two, please wait..." << std::endl;
    std::cout << std::endl;

    ModbusManager modbus;
    std::vector<int> foundAddresses;

    for (int address = 1; address <= 159; ++address)
    {
        Device device;
        device.name = "scan";
        device.port = port;
        device.baudRate = baudRate;
        device.address = address;
        device.enabled = true;

        const PollResult result = modbus.poll(device);

        if (result.success)
        {
            std::cout
                << ">>> FOUND response at address "
                << address
                << " (weight: " << result.weight << ")"
                << std::endl;

            foundAddresses.push_back(address);
        }

        if (address % 20 == 0)
        {
            std::cout << "... checked up to address " << address << std::endl;
        }
    }

    std::cout << std::endl;
    std::cout << "Scan complete." << std::endl;

    if (foundAddresses.empty())
    {
        std::cout << "No devices responded on any address 1-159." << std::endl;
        std::cout << "Check wiring, baud rate, and that the device is powered on." << std::endl;
    }
    else
    {
        std::cout << "Responding addresses: ";

        for (std::size_t i = 0; i < foundAddresses.size(); ++i)
        {
            if (i > 0) std::cout << ", ";
            std::cout << foundAddresses[i];
        }

        std::cout << std::endl;
    }

    return 0;
}
