#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "AppSettings.h"
#include "Device.h"

class ConfigManager
{
public:
    ConfigManager();

    bool initialize();
    bool configExists() const;
    bool load();
    bool save();

    bool addDevice(const Device& device);
    bool updateDevice(const Device& device);
    bool saveAgentId(const std::string& id);
    bool configChanged();

    bool setDeviceEnabled(
        const std::string& uuid,
        bool enabled);

    bool removeDevice(
        const std::string& uuid);

    AppSettings settings() const;

    bool updateSettings(
        const AppSettings& settings);

    std::string generateUuid();
    std::string getAgentId();

    std::string serverHost() const;
    int pollInterval() const;
    int sendInterval() const;

    std::string apiPath() const;
    bool verifyTlsCertificate() const;
    int retentionDays() const;

    std::string companyUuid() const;
    std::string herdUuid() const;
    std::string farmUuid() const;

    const std::vector<Device>& devices() const;

private:
    std::string m_configPath;
    std::string m_serverHost;
    std::string m_apiPath =
        "/dash/test/post/768a61e4-df8a-4866-be2a-8ab6f78ce328";
    bool m_verifyTlsCertificate = false;
    int m_retentionDays = 30;

    std::string m_companyUuid;
    std::string m_herdUuid;
    std::string m_farmUuid;

    std::filesystem::file_time_type m_lastWriteTime;

    int m_pollInterval = 5;
    int m_sendInterval = 30;

    std::vector<Device> m_devices;
};