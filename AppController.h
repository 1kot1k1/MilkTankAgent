#pragma once

#include "ConfigManager.h"
#include "ModbusManager.h"
#include "DataManager.h"
#include "HttpClient.h"
#include "DatabaseManager.h"
#include "DeviceManager.h"
#include "MeasurementQueue.h"
#include "EventBus.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include "AppSettings.h"
class AppController
{
public:
    AppController();

    void run();
    void stop();

    // Opens the config/data/logs folders and the SQLite database
    // without starting the polling loop. Needed by anything that
    // wants to use verifyPassword()/changePassword()/devices() etc.
    // independently of run() — e.g. a web server started before or
    // instead of the polling loop.
    bool initializeStorage();
    EventBus& eventBus();
    const std::vector<Device>& devices() const;
    bool addDevice(const Device& device);

    // Generates a fresh device UUID, for use by callers building a
    // new Device before calling addDevice() (e.g. the web API).
    std::string generateDeviceUuid();
    bool getDevice(
        const std::string& uuid,
        Device& device);

    bool updateDevice(
        const Device& device);
    bool setDeviceEnabled(
        const std::string& uuid,
        bool enabled);
    bool removeDevice(
        const std::string& uuid);
    AppSettings settings();

    bool updateSettings(
        const AppSettings& settings);

    bool verifyPassword(
        const std::string& password);

    bool changePassword(
        const std::string& oldPassword,
        const std::string& newPassword);

    std::uintmax_t databaseSizeBytes();

    bool clearDatabase();
private:
    std::atomic_bool m_stopRequested{ false };
    std::mutex m_stopMutex;
    std::condition_variable m_stopCondition;
    void processDevice(const Device& device);
    void sendPendingMeasurements();
    ConfigManager m_configManager;
    ModbusManager m_modbusManager;
    DataManager m_dataManager;
    HttpClient m_httpClient;
    DatabaseManager m_database;
    MeasurementQueue m_queue;
    bool reloadConfiguration();
    std::mutex m_operationMutex;

    bool reloadConfigurationUnlocked();
    void compareConfiguration(
        const std::vector<Device>& newDevices);
    DeviceManager m_deviceManager;
    EventBus m_eventBus;
    void publishDeviceEvent(
        DeviceEventType type,
        const Device& device,
        const PollResult* result = nullptr);
};