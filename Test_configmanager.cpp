#include "ConfigManager.h"

#include <iostream>

int main()
{
    ConfigManager config;

    if (!config.initialize())
    {
        std::cout << "initialize() FAILED" << std::endl;
        return 1;
    }

    std::cout << "initialize() OK" << std::endl;

    AppSettings settings;
    settings.serverHost = "example.com";
    settings.companyUuid = "test-company";
    settings.farmUuid = "test-farm";
    settings.herdUuid = "test-herd";
    settings.pollInterval = 7;
    settings.sendInterval = 42;

    if (!config.updateSettings(settings))
    {
        std::cout << "updateSettings() FAILED" << std::endl;
        return 1;
    }

    std::cout << "updateSettings() OK" << std::endl;

    ConfigManager reloaded;
    reloaded.initialize();

    if (!reloaded.load())
    {
        std::cout << "load() FAILED" << std::endl;
        return 1;
    }

    std::cout << "load() OK" << std::endl;

    const AppSettings loaded = reloaded.settings();

    std::cout << "serverHost: " << loaded.serverHost << std::endl;
    std::cout << "apiPath: " << loaded.apiPath << std::endl;
    std::cout << "pollInterval: " << loaded.pollInterval << std::endl;
    std::cout << "sendInterval: " << loaded.sendInterval << std::endl;
    std::cout << "retentionDays: " << loaded.retentionDays << std::endl;

    Device device;
    device.name = "Test Tank";
    device.uuid = reloaded.generateUuid();
    device.address = 1;
    device.port = "/dev/ttyUSB0";
    device.enabled = true;

    if (!reloaded.addDevice(device))
    {
        std::cout << "addDevice() FAILED" << std::endl;
        return 1;
    }

    std::cout << "addDevice() OK, devices count: "
        << reloaded.devices().size() << std::endl;

    std::cout << "OK" << std::endl;
    return 0;
}