#include "ConfigManager.h"
#include "json.hpp"
#include "Logger.h"
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include <unistd.h>
#include <limits.h>
#include <utility>
#include <set>
#include <cstdint>

using json = nlohmann::json;

ConfigManager::ConfigManager()
{
    m_configPath = "config/config.json";
}
bool ConfigManager::configChanged()
{
    try
    {
        auto current =
            std::filesystem::last_write_time(m_configPath);


        if (current != m_lastWriteTime)
        {
            Logger::info("Config  changed.");
            m_lastWriteTime = current;
            return true;
        }
    }
    catch (...)
    {
        Logger::warning("Cannot check config modification time.");
    }

    return false;
}

bool ConfigManager::initialize()
{
    namespace fs = std::filesystem;

    try
    {
        char executablePath[PATH_MAX] = { 0 };

        const ssize_t length =
            readlink(
                "/proc/self/exe",
                executablePath,
                sizeof(executablePath) - 1);

        if (length <= 0)
        {
            Logger::error(
                "Cannot determine application directory.");

            return false;
        }

        executablePath[length] = '\0';

        const fs::path applicationDirectory =
            fs::path(executablePath).parent_path();
        fs::current_path(applicationDirectory);

        fs::create_directories("config");
        fs::create_directories("logs");
        fs::create_directories("data");

        Logger::info(
            "Application directory: "
            + applicationDirectory.string());
    }
    catch (const std::exception& exception)
    {
        Logger::error(
            "Cannot initialize application folders: "
            + std::string(exception.what()));

        return false;
    }

    return true;
}

bool ConfigManager::configExists() const
{
    return std::filesystem::exists(m_configPath);
}

bool ConfigManager::load()
{
    std::ifstream file(m_configPath);

    if (!file.is_open())
    {
        Logger::error(
            "Cannot open config file: "
            + m_configPath);

        return false;
    }

    try
    {
        json config;
        file >> config;

        if (!config.contains("serverHost") ||
            !config.contains("companyUuid") ||
            !config.contains("herdUuid") ||
            !config.contains("farmUuid") ||
            !config.contains("pollInterval") ||
            !config.contains("sendInterval") ||
            !config.contains("devices"))
        {
            Logger::error(
                "Config file is missing required fields.");

            return false;
        }

        if (!config["serverHost"].is_string() ||
            !config["companyUuid"].is_string() ||
            !config["herdUuid"].is_string() ||
            !config["farmUuid"].is_string() ||
            !config["pollInterval"].is_number_integer() ||
            !config["sendInterval"].is_number_integer() ||
            !config["devices"].is_array())
        {
            Logger::error(
                "Config file contains invalid field types.");

            return false;
        }

        const int pollInterval =
            config["pollInterval"].get<int>();

        const int sendInterval =
            config["sendInterval"].get<int>();

        if (pollInterval <= 0)
        {
            Logger::error(
                "pollInterval must be greater than zero.");

            return false;
        }

        if (sendInterval <= 0)
        {
            Logger::error(
                "sendInterval must be greater than zero.");

            return false;
        }

        std::vector<Device> loadedDevices;
        std::set<std::string> deviceUuids;
        std::set<std::string> deviceConnections;

        for (std::size_t index = 0;
            index < config["devices"].size();
            ++index)
        {
            const auto& item =
                config["devices"][index];

            if (!item.contains("name") ||
                !item.contains("address") ||
                !item.contains("enabled") ||
                !item.contains("port") ||
                !item.contains("baudRate") ||
                !item.contains("uuid"))
            {
                Logger::error(
                    "Device configuration is missing fields. Index: "
                    + std::to_string(index));

                return false;
            }

            if (!item["name"].is_string() ||
                !item["address"].is_number_integer() ||
                !item["enabled"].is_boolean() ||
                !item["port"].is_string() ||
                !item["baudRate"].is_number_integer() ||
                !item["uuid"].is_string())
            {
                Logger::error(
                    "Device configuration has invalid field types. Index: "
                    + std::to_string(index));

                return false;
            }

            Device device;

            device.name =
                item["name"].get<std::string>();

            device.address =
                item["address"].get<int>();

            device.enabled =
                item["enabled"].get<bool>();

            device.port =
                item["port"].get<std::string>();

            device.baudRate =
                item["baudRate"].get<int>();

            device.uuid =
                item["uuid"].get<std::string>();
            if (item.contains("serialNumber"))
            {
                const auto& serialValue =
                    item["serialNumber"];

                if (!serialValue.is_number_integer() &&
                    !serialValue.is_number_unsigned())
                {
                    Logger::error(
                        "Device serialNumber has invalid type. Index: "
                        + std::to_string(index));

                    return false;
                }

                std::uint64_t serialNumber = 0;

                if (serialValue.is_number_unsigned())
                {
                    serialNumber =
                        serialValue.get<std::uint64_t>();
                }
                else
                {
                    const std::int64_t signedValue =
                        serialValue.get<std::int64_t>();

                    if (signedValue < 0)
                    {
                        Logger::error(
                            "Device serialNumber cannot be negative. Index: "
                            + std::to_string(index));

                        return false;
                    }

                    serialNumber =
                        static_cast<std::uint64_t>(
                            signedValue);
                }

                if (serialNumber > 16777215)
                {
                    Logger::error(
                        "Device serialNumber must be between 0 and 16777215. Index: "
                        + std::to_string(index));

                    return false;
                }

                device.serialNumber =
                    static_cast<std::uint32_t>(
                        serialNumber);
            }
            if (device.name.empty())
            {
                Logger::error(
                    "Device name is empty. Index: "
                    + std::to_string(index));

                return false;
            }

            if (device.uuid.empty())
            {
                Logger::error(
                    "Device UUID is empty. Index: "
                    + std::to_string(index));

                return false;
            }
            if (!deviceUuids.insert(device.uuid).second)
            {
                Logger::error(
                    "Duplicate device UUID: "
                    + device.uuid);

                return false;
            }
            if (device.port.empty())
            {
                Logger::error(
                    "Device COM port is empty. Index: "
                    + std::to_string(index));

                return false;
            }

            if (device.address < 1 ||
                device.address > 159)
            {
                Logger::error(
                    "Device Modbus address must be between 1 and 159. Index: "
                    + std::to_string(index));

                return false;
            }
            const std::string connectionKey =
                device.port
                + ":"
                + std::to_string(device.address);

            if (!deviceConnections.insert(connectionKey).second)
            {
                Logger::error(
                    "Duplicate device connection: "
                    + connectionKey);

                return false;
            }
            if (device.baudRate <= 0)
            {
                Logger::error(
                    "Device baudRate must be greater than zero. Index: "
                    + std::to_string(index));

                return false;
            }

            loadedDevices.push_back(
                std::move(device));
        }

        m_serverHost =
            config["serverHost"].get<std::string>();

        m_companyUuid =
            config["companyUuid"].get<std::string>();

        m_herdUuid =
            config["herdUuid"].get<std::string>();

        m_farmUuid =
            config["farmUuid"].get<std::string>();
        if (m_companyUuid.empty())
        {
            Logger::warning(
                "companyUuid is empty.");
        }

        if (m_farmUuid.empty())
        {
            Logger::warning(
                "farmUuid is empty.");
        }

        if (m_herdUuid.empty())
        {
            Logger::warning(
                "herdUuid is empty.");
        }

        m_pollInterval =
            pollInterval;

        m_sendInterval =
            sendInterval;

        if (config.contains("apiPath") &&
            config["apiPath"].is_string())
        {
            m_apiPath =
                config["apiPath"].get<std::string>();
        }

        if (config.contains("verifyTlsCertificate") &&
            config["verifyTlsCertificate"].is_boolean())
        {
            m_verifyTlsCertificate =
                config["verifyTlsCertificate"].get<bool>();
        }

        if (config.contains("retentionDays") &&
            config["retentionDays"].is_number_integer())
        {
            m_retentionDays =
                config["retentionDays"].get<int>();
        }

        m_devices =
            std::move(loadedDevices);

        m_lastWriteTime =
            std::filesystem::last_write_time(
                m_configPath);

        Logger::info(
            "Configuration loaded successfully.");

        return true;
    }
    catch (const json::parse_error& exception)
    {
        Logger::error(
            "Config JSON syntax error: "
            + std::string(exception.what()));
    }
    catch (const json::exception& exception)
    {
        Logger::error(
            "Config JSON error: "
            + std::string(exception.what()));
    }
    catch (const std::filesystem::filesystem_error& exception)
    {
        Logger::error(
            "Config file system error: "
            + std::string(exception.what()));
    }
    catch (const std::exception& exception)
    {
        Logger::error(
            "Cannot load configuration: "
            + std::string(exception.what()));
    }

    return false;
}

bool ConfigManager::save()
{
    try
    {
        json config;

        config["serverHost"] =
            m_serverHost;

        config["companyUuid"] =
            m_companyUuid;

        config["herdUuid"] =
            m_herdUuid;

        config["farmUuid"] =
            m_farmUuid;

        config["pollInterval"] =
            m_pollInterval;

        config["sendInterval"] =
            m_sendInterval;

        config["apiPath"] =
            m_apiPath;

        config["verifyTlsCertificate"] =
            m_verifyTlsCertificate;

        config["retentionDays"] =
            m_retentionDays;

        config["devices"] =
            json::array();

        for (const Device& device : m_devices)
        {
            json item;

            item["name"] =
                device.name;

            item["address"] =
                device.address;

            item["enabled"] =
                device.enabled;

            item["port"] =
                device.port;

            item["baudRate"] =
                device.baudRate;

            item["uuid"] =
                device.uuid;
            if (device.serialNumber.has_value())
            {
                item["serialNumber"] =
                    device.serialNumber.value();
            }


            config["devices"].push_back(
                std::move(item));
        }

        const std::filesystem::path configPath =
            m_configPath;

        const std::filesystem::path temporaryPath =
            configPath.string() + ".tmp";

        std::filesystem::create_directories(
            configPath.parent_path());

        {
            std::ofstream file(
                temporaryPath,
                std::ios::trunc);

            if (!file.is_open())
            {
                Logger::error(
                    "Cannot open temporary config file: "
                    + temporaryPath.string());

                return false;
            }

            file
                << config.dump(4)
                << std::endl;

            file.flush();

            if (!file.good())
            {
                Logger::error(
                    "Cannot write temporary config file.");

                file.close();

                std::filesystem::remove(
                    temporaryPath);

                return false;
            }
        }

        std::error_code renameErrorCode;

        std::filesystem::rename(
            temporaryPath,
            configPath,
            renameErrorCode);

        if (renameErrorCode)
        {
            Logger::error(
                "Cannot replace config file: "
                + renameErrorCode.message());

            std::filesystem::remove(
                temporaryPath);

            return false;
        }

        m_lastWriteTime =
            std::filesystem::last_write_time(
                configPath);

        Logger::info(
            "Configuration saved successfully.");

        return true;
    }
    catch (const std::exception& exception)
    {
        Logger::error(
            "Cannot save configuration: "
            + std::string(exception.what()));

        return false;
    }
}

bool ConfigManager::addDevice(
    const Device& device)
{
    if (device.name.empty())
    {
        Logger::error(
            "Cannot add device: name is empty.");

        return false;
    }

    if (device.uuid.empty())
    {
        Logger::error(
            "Cannot add device: UUID is empty.");

        return false;
    }

    if (device.port.empty())
    {
        Logger::error(
            "Cannot add device: COM port is empty.");

        return false;
    }

    if (device.address < 1 ||
        device.address > 159)
    {
        Logger::error(
            "Cannot add device: invalid Modbus address.");

        return false;
    }

    if (device.baudRate <= 0)
    {
        Logger::error(
            "Cannot add device: invalid baud rate.");

        return false;
    }

    for (const Device& existingDevice :
        m_devices)
    {
        if (existingDevice.uuid ==
            device.uuid)
        {
            Logger::error(
                "Cannot add device: duplicate UUID: "
                + device.uuid);

            return false;
        }

        if (existingDevice.port ==
            device.port &&
            existingDevice.address ==
            device.address)
        {
            Logger::error(
                "Cannot add device: connection already exists: "
                + device.port
                + ":"
                + std::to_string(device.address));

            return false;
        }
    }

    m_devices.push_back(
        device);

    if (!save())
    {
        m_devices.pop_back();

        Logger::error(
            "Device was not added because configuration could not be saved.");

        return false;
    }

    Logger::info(
        "Device added to configuration: "
        + device.name);

    return true;
}
bool ConfigManager::updateDevice(
    const Device& device)
{
    if (device.name.empty())
    {
        Logger::error(
            "Cannot update device: name is empty.");

        return false;
    }

    if (device.uuid.empty())
    {
        Logger::error(
            "Cannot update device: UUID is empty.");

        return false;
    }

    if (device.port.empty())
    {
        Logger::error(
            "Cannot update device: COM port is empty.");

        return false;
    }

    if (device.address < 1 ||
        device.address > 159)
    {
        Logger::error(
            "Cannot update device: invalid Modbus address.");

        return false;
    }

    if (device.baudRate <= 0)
    {
        Logger::error(
            "Cannot update device: invalid baud rate.");

        return false;
    }

    std::size_t deviceIndex =
        m_devices.size();

    for (std::size_t index = 0;
        index < m_devices.size();
        ++index)
    {
        const Device& existingDevice =
            m_devices[index];

        if (existingDevice.uuid ==
            device.uuid)
        {
            deviceIndex = index;
            continue;
        }

        if (existingDevice.port ==
            device.port &&
            existingDevice.address ==
            device.address)
        {
            Logger::error(
                "Cannot update device: connection already exists: "
                + device.port
                + ":"
                + std::to_string(device.address));

            return false;
        }
    }

    if (deviceIndex == m_devices.size())
    {
        Logger::error(
            "Cannot update device: UUID not found: "
            + device.uuid);

        return false;
    }

    const Device oldDevice =
        m_devices[deviceIndex];

    m_devices[deviceIndex] =
        device;

    if (!save())
    {
        m_devices[deviceIndex] =
            oldDevice;

        Logger::error(
            "Device was not updated because configuration could not be saved.");

        return false;
    }

    Logger::info(
        "Device updated in configuration: "
        + device.name);

    return true;
}

bool ConfigManager::setDeviceEnabled(
    const std::string& uuid,
    bool enabled)
{
    for (Device& device : m_devices)
    {
        if (device.uuid != uuid)
        {
            continue;
        }

        const bool previousValue =
            device.enabled;

        device.enabled =
            enabled;

        if (!save())
        {
            device.enabled =
                previousValue;

            Logger::error(
                "Device state was not changed because configuration could not be saved.");

            return false;
        }

        Logger::info(
            std::string(
                enabled
                ? "Device enabled: "
                : "Device disabled: ")
            + device.name);

        return true;
    }

    Logger::error(
        "Cannot change device state: UUID not found: "
        + uuid);

    return false;
}
bool ConfigManager::removeDevice(
    const std::string& uuid)
{
    for (std::size_t index = 0;
        index < m_devices.size();
        ++index)
    {
        if (m_devices[index].uuid != uuid)
        {
            continue;
        }

        const Device removedDevice =
            m_devices[index];

        m_devices.erase(
            m_devices.begin() + index);

        if (!save())
        {
            m_devices.insert(
                m_devices.begin() + index,
                removedDevice);

            Logger::error(
                "Device was not removed because configuration could not be saved.");

            return false;
        }

        Logger::info(
            "Device removed from configuration: "
            + removedDevice.name);

        return true;
    }

    Logger::error(
        "Cannot remove device: UUID not found: "
        + uuid);

    return false;
}
bool ConfigManager::saveAgentId(const std::string& id)
{
    std::ofstream file("config/agent.id");

    if (!file.is_open())
        return false;

    file << id;

    return true;
}
AppSettings ConfigManager::settings() const
{
    AppSettings result;

    result.serverHost =
        m_serverHost;

    result.apiPath =
        m_apiPath;

    result.companyUuid =
        m_companyUuid;

    result.farmUuid =
        m_farmUuid;

    result.herdUuid =
        m_herdUuid;

    result.pollInterval =
        m_pollInterval;

    result.sendInterval =
        m_sendInterval;

    result.verifyTlsCertificate =
        m_verifyTlsCertificate;

    result.retentionDays =
        m_retentionDays;

    return result;
}

bool ConfigManager::updateSettings(
    const AppSettings& settings)
{
    if (settings.serverHost.empty())
    {
        Logger::error(
            "Cannot update settings: server host is empty.");

        return false;
    }

    if (settings.pollInterval <= 0)
    {
        Logger::error(
            "Cannot update settings: invalid poll interval.");

        return false;
    }

    if (settings.sendInterval <= 0)
    {
        Logger::error(
            "Cannot update settings: invalid send interval.");

        return false;
    }

    const AppSettings oldSettings =
        this->settings();

    m_serverHost =
        settings.serverHost;

    m_apiPath =
        settings.apiPath;

    m_companyUuid =
        settings.companyUuid;

    m_farmUuid =
        settings.farmUuid;

    m_herdUuid =
        settings.herdUuid;

    m_pollInterval =
        settings.pollInterval;

    m_sendInterval =
        settings.sendInterval;

    m_verifyTlsCertificate =
        settings.verifyTlsCertificate;

    m_retentionDays =
        settings.retentionDays;

    if (!save())
    {
        m_serverHost =
            oldSettings.serverHost;

        m_apiPath =
            oldSettings.apiPath;

        m_companyUuid =
            oldSettings.companyUuid;

        m_farmUuid =
            oldSettings.farmUuid;

        m_herdUuid =
            oldSettings.herdUuid;

        m_pollInterval =
            oldSettings.pollInterval;

        m_sendInterval =
            oldSettings.sendInterval;

        m_verifyTlsCertificate =
            oldSettings.verifyTlsCertificate;

        m_retentionDays =
            oldSettings.retentionDays;

        Logger::error(
            "Settings were not updated because configuration could not be saved.");

        return false;
    }

    Logger::info(
        "Application settings updated.");

    return true;
}

std::string ConfigManager::generateUuid()
{
    static const char hex[] = "0123456789ABCDEF";

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;

    for (int i = 0; i < 32; i++)
    {
        ss << hex[dis(gen)];
    }

    return ss.str();
}

std::string ConfigManager::getAgentId()
{
    const std::string path = "config/agent.id";

    std::ifstream file(path);

    if (file.is_open())
    {
        std::string id;
        std::getline(file, id);
        return id;
    }

    return "";
}
std::string ConfigManager::serverHost() const
{
    return m_serverHost;
}

int ConfigManager::pollInterval() const
{
    return m_pollInterval;
}

int ConfigManager::sendInterval() const
{
    return m_sendInterval;
}
std::string ConfigManager::apiPath() const
{
    return m_apiPath;
}

bool ConfigManager::verifyTlsCertificate() const
{
    return m_verifyTlsCertificate;
}

int ConfigManager::retentionDays() const
{
    return m_retentionDays;
}
std::string ConfigManager::companyUuid() const
{
    return m_companyUuid;
}

std::string ConfigManager::herdUuid() const
{
    return m_herdUuid;
}

std::string ConfigManager::farmUuid() const
{
    return m_farmUuid;
}
const std::vector<Device>& ConfigManager::devices() const
{
    return m_devices;
}