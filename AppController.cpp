#include "AppController.h"
#include "DatabaseManager.h"
#include "Logger.h"
#include "LoggerSubscriber.h"
#include "PollResult.h"
#include "PasswordHasher.h"

#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

AppController::AppController()
    : m_queue(
        m_database,
        m_httpClient,
        m_configManager)
{
    LoggerSubscriber::subscribe(
        m_eventBus);
}

EventBus& AppController::eventBus()
{
    return m_eventBus;
}

const std::vector<Device>& AppController::devices() const
{
    return m_deviceManager.devices();
}

void AppController::publishDeviceEvent(
    DeviceEventType type,
    const Device& device,
    const PollResult* result)
{
    DeviceEvent event{};

    event.type = type;
    event.uuid = device.uuid;
    event.name = device.name;

    if (result != nullptr)
    {
        event.weight = result->weight;
        event.stable = result->stable;
        event.overload = result->overload;
    }

    m_eventBus.publish(event);
}

void AppController::processDevice(
    const Device& device)
{
    if (!device.enabled)
    {
        if (m_deviceManager.setState(
            device.uuid,
            DeviceState::Disabled))
        {
            publishDeviceEvent(
                DeviceEventType::Disabled,
                device);
        }

        return;
    }

    const PollResult result =
        m_modbusManager.poll(device);

    m_deviceManager.updatePollResult(
        device.uuid,
        result);

    const DeviceState newState =
        result.success
        ? DeviceState::Online
        : DeviceState::Offline;

    if (m_deviceManager.setState(
        device.uuid,
        newState))
    {
        publishDeviceEvent(
            newState == DeviceState::Online
            ? DeviceEventType::Online
            : DeviceEventType::Offline,
            device);
    }

    m_dataManager.update(
        device,
        result);

    const auto& dataDevices =
        m_dataManager.devices();

    for (const auto& data : dataDevices)
    {
        if (data.address == device.address)
        {
            m_queue.enqueue(data);
            break;
        }
    }

    if (!result.success)
    {
        return;
    }

    if (result.overload)
    {
        publishDeviceEvent(
            DeviceEventType::Overload,
            device,
            &result);
    }
    else if (!result.stable)
    {
        publishDeviceEvent(
            DeviceEventType::Unstable,
            device,
            &result);
    }
    else
    {
        publishDeviceEvent(
            DeviceEventType::WeightChanged,
            device,
            &result);
    }
}

void AppController::sendPendingMeasurements()
{
    m_queue.sendPending();
}

bool AppController::addDevice(
    const Device& device)
{
    std::lock_guard<std::mutex> lock(
        m_operationMutex);

    if (!m_configManager.addDevice(device))
    {
        return false;
    }

    return reloadConfigurationUnlocked();
}

std::string AppController::generateDeviceUuid()
{
    std::lock_guard<std::mutex> lock(
        m_operationMutex);

    return m_configManager.generateUuid();
}

bool AppController::getDevice(
    const std::string& uuid,
    Device& device)
{
    std::lock_guard<std::mutex> lock(
        m_operationMutex);

    for (const Device& existingDevice :
        m_deviceManager.devices())
    {
        if (existingDevice.uuid == uuid)
        {
            device = existingDevice;
            return true;
        }
    }

    return false;
}

bool AppController::updateDevice(
    const Device& device)
{
    std::lock_guard<std::mutex> lock(
        m_operationMutex);

    if (!m_configManager.updateDevice(device))
    {
        return false;
    }

    return reloadConfigurationUnlocked();
}

bool AppController::setDeviceEnabled(
    const std::string& uuid,
    bool enabled)
{
    std::lock_guard<std::mutex> lock(
        m_operationMutex);

    if (!m_configManager.setDeviceEnabled(
        uuid,
        enabled))
    {
        return false;
    }

    return reloadConfigurationUnlocked();
}

bool AppController::removeDevice(
    const std::string& uuid)
{
    std::lock_guard<std::mutex> lock(
        m_operationMutex);

    if (!m_configManager.removeDevice(uuid))
    {
        return false;
    }

    return reloadConfigurationUnlocked();
}

AppSettings AppController::settings()
{
    std::lock_guard<std::mutex> lock(
        m_operationMutex);

    return m_configManager.settings();
}

bool AppController::updateSettings(
    const AppSettings& settings)
{
    std::lock_guard<std::mutex> lock(
        m_operationMutex);

    if (!m_configManager.updateSettings(
        settings))
    {
        return false;
    }

    return reloadConfigurationUnlocked();
}

bool AppController::reloadConfiguration()
{
    std::lock_guard<std::mutex> lock(
        m_operationMutex);

    return reloadConfigurationUnlocked();
}

bool AppController::reloadConfigurationUnlocked()
{
    if (!m_configManager.load())
    {
        Logger::error(
            "Configuration reload failed.");

        return false;
    }

    compareConfiguration(
        m_configManager.devices());

    Logger::info(
        "Configuration loaded.");

    Logger::info(
        "Server: "
        + m_configManager.serverHost());

    Logger::info(
        "Poll interval: "
        + std::to_string(
            m_configManager.pollInterval()));

    Logger::info(
        "Send interval: "
        + std::to_string(
            m_configManager.sendInterval()));

    return true;
}

void AppController::compareConfiguration(
    const std::vector<Device>& newDevices)
{
    std::vector<std::string> devicesToRemove;

    for (const Device& oldDevice :
        m_deviceManager.devices())
    {
        bool found = false;

        for (const Device& newDevice :
            newDevices)
        {
            if (oldDevice.uuid == newDevice.uuid)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            devicesToRemove.push_back(
                oldDevice.uuid);

            publishDeviceEvent(
                DeviceEventType::Removed,
                oldDevice);
        }
    }

    for (const std::string& uuid :
        devicesToRemove)
    {
        m_deviceManager.removeDevice(uuid);
    }

    for (const Device& newDevice :
        newDevices)
    {
        Device* existingDevice =
            m_deviceManager.findDevice(
                newDevice.uuid);

        if (existingDevice == nullptr)
        {
            m_deviceManager.addDevice(
                newDevice);

            publishDeviceEvent(
                DeviceEventType::Added,
                newDevice);

            continue;
        }

        if (*existingDevice != newDevice)
        {
            m_deviceManager.updateDevice(
                newDevice);

            publishDeviceEvent(
                DeviceEventType::Updated,
                newDevice);
        }
    }

    Logger::info(
        "=========== Devices ===========");

    for (const Device& device :
        m_deviceManager.devices())
    {
        Logger::info(
            device.name
            + " | "
            + device.port
            + " | Addr "
            + std::to_string(device.address)
            + (device.enabled
                ? " | ENABLED"
                : " | DISABLED"));
    }

    Logger::info(
        "===============================");
}

bool AppController::initializeStorage()
{
    if (!m_configManager.initialize())
    {
        Logger::error(
            "Failed to initialize application folders.");

        return false;
    }

    Logger::info(
        "Application folders initialized.");

    if (!m_database.initialize())
    {
        Logger::error(
            "Database init failed.");

        return false;
    }

    return true;
}

void AppController::run()
{
    Logger::info(
        "MilkTankAgent started.");

    if (!initializeStorage())
    {
        return;
    }

    m_database.deleteOldMeasurements(
        m_configManager.retentionDays());

    bool configurationLoaded = false;

    while (!m_stopRequested.load())
    {
        if (!m_configManager.configExists())
        {
            Logger::warning(
                "Config file not found. Waiting for configuration.");
        }
        else if (reloadConfiguration())
        {
            Logger::info(
                "Configuration loaded. Agent is starting.");

            configurationLoaded = true;
            break;
        }
        else
        {
            Logger::warning(
                "Configuration is invalid. Waiting for correction.");
        }

        std::unique_lock<std::mutex> lock(
            m_stopMutex);

        m_stopCondition.wait_for(
            lock,
            std::chrono::seconds(2),
            [this]()
            {
                return m_stopRequested.load();
            });
    }

    if (!configurationLoaded)
    {
        return;
    }

    std::string id =
        m_configManager.getAgentId();

    if (id.empty())
    {
        Logger::warning(
            "Agent ID not found.");

        id =
            m_configManager.generateUuid();

        if (m_configManager.saveAgentId(id))
        {
            Logger::info(
                "New Agent ID created.");
        }
        else
        {
            Logger::error(
                "Cannot save Agent ID.");
        }
    }
    else
    {
        Logger::info(
            "Agent ID: "
            + id);
    }

    auto lastSend =
        std::chrono::steady_clock::now();

    while (!m_stopRequested.load())
    {
        int pollInterval = 1;

        {
            std::lock_guard<std::mutex> lock(
                m_operationMutex);

            for (const Device& device :
                m_deviceManager.devices())
            {
                processDevice(device);
            }

            if (m_configManager.configChanged())
            {
                Logger::info(
                    "Configuration changed.");

                if (!reloadConfigurationUnlocked())
                {
                    Logger::error(
                        "Reload failed.");
                }
            }

            const auto now =
                std::chrono::steady_clock::now();

            if (now - lastSend >=
                std::chrono::seconds(
                    m_configManager.sendInterval()))
            {
                sendPendingMeasurements();

                lastSend = now;
            }

            pollInterval =
                m_configManager.pollInterval();
        }

        std::unique_lock<std::mutex> lock(
            m_stopMutex);

        m_stopCondition.wait_for(
            lock,
            std::chrono::seconds(
                pollInterval),
            [this]()
            {
                return m_stopRequested.load();
            });
    }
}

void AppController::stop()
{
    m_stopRequested.store(true);

    m_stopCondition.notify_all();
}

bool AppController::verifyPassword(
    const std::string& password)
{
    std::lock_guard<std::mutex> lock(
        m_operationMutex);

    std::string storedHash;
    std::string storedSalt;

    if (!m_database.getAdminPassword(
        storedHash,
        storedSalt))
    {
        const std::string defaultSalt =
            PasswordHasher::generateSalt();

        const std::string defaultHash =
            PasswordHasher::hash(
                "1234",
                defaultSalt);

        if (defaultSalt.empty() ||
            defaultHash.empty())
        {
            Logger::error(
                "Cannot generate default admin password.");

            return false;
        }

        if (!m_database.setAdminPassword(
            defaultHash,
            defaultSalt))
        {
            Logger::error(
                "Cannot save default admin password.");

            return false;
        }

        Logger::info(
            "Default admin password created.");

        storedHash = defaultHash;
        storedSalt = defaultSalt;
    }

    const std::string computedHash =
        PasswordHasher::hash(
            password,
            storedSalt);

    return
        !computedHash.empty() &&
        computedHash == storedHash;
}

bool AppController::changePassword(
    const std::string& oldPassword,
    const std::string& newPassword)
{
    std::lock_guard<std::mutex> lock(
        m_operationMutex);

    if (newPassword.empty())
    {
        Logger::error(
            "Cannot change password: new password is empty.");

        return false;
    }

    std::string storedHash;
    std::string storedSalt;

    if (!m_database.getAdminPassword(
        storedHash,
        storedSalt))
    {
        Logger::error(
            "Cannot change password: admin password not set.");

        return false;
    }

    const std::string computedOldHash =
        PasswordHasher::hash(
            oldPassword,
            storedSalt);

    if (computedOldHash.empty() ||
        computedOldHash != storedHash)
    {
        Logger::warning(
            "Password change rejected: incorrect current password.");

        return false;
    }

    const std::string newSalt =
        PasswordHasher::generateSalt();

    const std::string newHash =
        PasswordHasher::hash(
            newPassword,
            newSalt);

    if (newSalt.empty() ||
        newHash.empty())
    {
        Logger::error(
            "Cannot generate new admin password hash.");

        return false;
    }

    if (!m_database.setAdminPassword(
        newHash,
        newSalt))
    {
        Logger::error(
            "Cannot save new admin password.");

        return false;
    }

    Logger::info(
        "Admin password changed.");

    return true;
}

std::uintmax_t AppController::databaseSizeBytes()
{
    std::lock_guard<std::mutex> lock(
        m_operationMutex);

    return m_database.databaseSizeBytes();
}

bool AppController::clearDatabase()
{
    std::lock_guard<std::mutex> lock(
        m_operationMutex);

    return m_database.clearAllMeasurements();
}