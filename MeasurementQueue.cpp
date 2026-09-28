#include "MeasurementQueue.h"

#include "Logger.h"

MeasurementQueue::MeasurementQueue(
    DatabaseManager& database,
    HttpClient& httpClient,
    ConfigManager& config)
    :
    m_database(database),
    m_httpClient(httpClient),
    m_config(config)
{
}

void MeasurementQueue::enqueue(
    const DeviceData& data)
{
    m_buffer.push_back(data);
}

void MeasurementQueue::sendPending()
{
    sendBacklog();
    sendBuffered();
}

void MeasurementQueue::sendBacklog()
{
    auto pending =
        m_database.getPendingMeasurements(50);

    if (pending.empty())
        return;

    if (!m_httpClient.send(
        m_config.serverHost(),
        m_config.apiPath(),
        m_config.companyUuid(),
        m_config.herdUuid(),
        m_config.farmUuid(),
        pending,
        m_config.verifyTlsCertificate()))
    {
        return;
    }

    for (const auto& item : pending)
    {
        m_database.deleteMeasurement(item.id);
    }

    Logger::info(
        "Sent " +
        std::to_string(pending.size()) +
        " backlog measurements.");
}

void MeasurementQueue::sendBuffered()
{
    if (m_buffer.empty())
        return;

    if (m_httpClient.send(
        m_config.serverHost(),
        m_config.apiPath(),
        m_config.companyUuid(),
        m_config.herdUuid(),
        m_config.farmUuid(),
        m_buffer,
        m_config.verifyTlsCertificate()))
    {
        Logger::info(
            "Sent " +
            std::to_string(m_buffer.size()) +
            " measurements.");

        m_buffer.clear();

        return;
    }

    Logger::warning(
        "Failed to send current measurements. "
        "Saving " +
        std::to_string(m_buffer.size()) +
        " records to local database.");

    for (const auto& data : m_buffer)
    {
        m_database.addMeasurement(data);
    }

    m_buffer.clear();
}