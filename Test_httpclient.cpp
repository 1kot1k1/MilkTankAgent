#include "HttpClient.h"

#include <iostream>

int main()
{
    DeviceData device;
    device.uuid = "test-uuid-1234";
    device.weight = 65.5;
    device.stable = true;
    device.overload = false;
    device.online = true;
    device.lastUpdate = std::time(nullptr);

    std::vector<DeviceData> devices = { device };

    HttpClient client;

    const bool ok =
        client.send(
            "httpbin.org",
            "/post",
            "test-company",
            "test-herd",
            "test-farm",
            devices,
            true);

    if (!ok)
    {
        std::cout << "send() FAILED" << std::endl;
        return 1;
    }

    std::cout << "send() OK" << std::endl;
    return 0;
}
