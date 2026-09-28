#pragma once

#include "sqlite3.h"
#include "DeviceData.h"
#include <cstdint>
#include <string>
#include <vector>


class DatabaseManager
{
public:

    bool initialize();
    bool deleteMeasurement(int id);
    bool addMeasurement(const DeviceData& data);
    void deleteOldMeasurements(int retentionDays);

    bool clearAllMeasurements();
    std::uintmax_t databaseSizeBytes() const;

    std::vector<DeviceData> getPendingMeasurements(int limit);

    bool getAdminPassword(
        std::string& hash,
        std::string& salt);

    bool setAdminPassword(
        const std::string& hash,
        const std::string& salt);

private:

    sqlite3* m_db = nullptr;
    std::string m_dbPath;
};