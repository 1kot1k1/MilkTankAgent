#pragma once

#include "DatabaseManager.h"
#include "HttpClient.h"
#include "ConfigManager.h"

#include <vector>

class MeasurementQueue
{
public:

    MeasurementQueue(
        DatabaseManager& database,
        HttpClient& httpClient,
        ConfigManager& config);

    void enqueue(const DeviceData& data);
    void sendPending();

private:

    void sendBacklog();
    void sendBuffered();

    DatabaseManager& m_database;
    HttpClient& m_httpClient;
    ConfigManager& m_config;

    std::vector<DeviceData> m_buffer;
};