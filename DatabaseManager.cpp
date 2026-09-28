#include "DatabaseManager.h"

#include "Logger.h"

#include <filesystem>



bool DatabaseManager::initialize()
{
    m_dbPath = "data/history.db";

    int rc =
        sqlite3_open(m_dbPath.c_str(), &m_db);
    if (rc != SQLITE_OK)
    {
        Logger::error("Cannot open database");
        return false;
    }

    Logger::info("SQLite opened.");
    const char* sql =
        "CREATE TABLE IF NOT EXISTS measurements ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "time INTEGER,"
        "device_uuid TEXT,"
        "weight REAL,"
        "status INTEGER,"
        "stable INTEGER,"
        "overload INTEGER,"
        "online INTEGER DEFAULT 1,"
        "sent INTEGER DEFAULT 0"
        ");";

    char* error = nullptr;

    if (sqlite3_exec(m_db, sql, nullptr, nullptr, &error) != SQLITE_OK)
    {
        Logger::error(error);

        sqlite3_free(error);

        return false;
    }

    Logger::info("Measurements table ready.");

    char* migrationError = nullptr;

    if (sqlite3_exec(
        m_db,
        "ALTER TABLE measurements ADD COLUMN online INTEGER DEFAULT 1;",
        nullptr,
        nullptr,
        &migrationError) != SQLITE_OK)
    {
        sqlite3_free(migrationError);
    }
    else
    {
        Logger::info(
            "Measurements table migrated: added online column.");
    }

    const char* adminSql =
        "CREATE TABLE IF NOT EXISTS admin ("
        "id INTEGER PRIMARY KEY CHECK (id = 1),"
        "password_hash TEXT NOT NULL,"
        "password_salt TEXT NOT NULL"
        ");";

    if (sqlite3_exec(m_db, adminSql, nullptr, nullptr, &error) != SQLITE_OK)
    {
        Logger::error(error);

        sqlite3_free(error);

        return false;
    }

    Logger::info("Admin table ready.");


    return true;
}
bool DatabaseManager::addMeasurement(const DeviceData& data)
{
    const char* sql =
        "INSERT INTO measurements "
        "(time, device_uuid, weight, status, stable, overload, online, sent) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, 0);";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        Logger::error("sqlite prepare failed");
        return false;
    }

    sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(data.lastUpdate));
    sqlite3_bind_text(stmt, 2, data.uuid.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, data.weight);
    sqlite3_bind_int(stmt, 4, data.status);
    sqlite3_bind_int(stmt, 5, data.stable);
    sqlite3_bind_int(stmt, 6, data.overload);
    sqlite3_bind_int(stmt, 7, data.online ? 1 : 0);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;

    sqlite3_finalize(stmt);

    return ok;
}
std::vector<DeviceData> DatabaseManager::getPendingMeasurements(int limit)
{
    std::vector<DeviceData> result;

    const char* sql =
        "SELECT "
        "id, "
        "time, "
        "device_uuid, "
        "weight, "
        "status, "
        "stable, "
        "overload, "
        "online "
        "FROM measurements "
        "WHERE sent = 0 "
        "ORDER BY id "
        "LIMIT ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(
        m_db,
        sql,
        -1,
        &stmt,
        nullptr) != SQLITE_OK)
    {
        Logger::error("SELECT prepare failed");

        return result;
    }

    sqlite3_bind_int(stmt, 1, limit);
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        DeviceData data;
        data.id =
            sqlite3_column_int(stmt, 0);

        data.lastUpdate = static_cast<std::time_t>(
            sqlite3_column_int64(stmt, 1));

        const unsigned char* uuidText =
            sqlite3_column_text(stmt, 2);

        if (uuidText == nullptr)
        {
            Logger::warning(
                "Measurement row has NULL device_uuid, skipping. id=" +
                std::to_string(data.id));

            continue;
        }

        data.uuid = reinterpret_cast<const char*>(uuidText);

        data.weight = sqlite3_column_double(stmt, 3);

        data.status = static_cast<std::uint8_t>(
            sqlite3_column_int(stmt, 4));

        data.stable =
            sqlite3_column_int(stmt, 5) != 0;

        data.overload =
            sqlite3_column_int(stmt, 6) != 0;

        data.online =
            sqlite3_column_int(stmt, 7) != 0;

        result.push_back(data);
    }
    sqlite3_finalize(stmt);

    return result;
}
bool DatabaseManager::deleteMeasurement(int id)
{
    const char* sql =
        "DELETE FROM measurements "
        "WHERE id = ?;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(
        m_db,
        sql,
        -1,
        &stmt,
        nullptr) != SQLITE_OK)
    {
        Logger::error("DELETE prepare failed");
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    bool ok =
        sqlite3_step(stmt) == SQLITE_DONE;

    sqlite3_finalize(stmt);

    return ok;
}
void DatabaseManager::deleteOldMeasurements(int retentionDays)
{
    if (retentionDays <= 0)
    {
        retentionDays = 30;
    }

    const std::string sql =
        "DELETE FROM measurements "
        "WHERE time < strftime('%s','now') - " +
        std::to_string(
            static_cast<long long>(retentionDays) * 86400) +
        ";";

    char* err = nullptr;

    if (sqlite3_exec(
        m_db,
        sql.c_str(),
        nullptr,
        nullptr,
        &err) != SQLITE_OK)
    {
        Logger::error(
            "Delete old measurements failed: " +
            std::string(err));

        sqlite3_free(err);
    }
    else
    {
#if DEBUG_LOG
        Logger::info("Old measurements deleted.");
#endif
    }
}

bool DatabaseManager::clearAllMeasurements()
{
    char* err = nullptr;

    if (sqlite3_exec(
        m_db,
        "DELETE FROM measurements;",
        nullptr,
        nullptr,
        &err) != SQLITE_OK)
    {
        Logger::error(
            "Clear measurements failed: " +
            std::string(err));

        sqlite3_free(err);

        return false;
    }

    Logger::info(
        "Local measurements database cleared by user.");

    return true;
}

std::uintmax_t DatabaseManager::databaseSizeBytes() const
{
    std::error_code errorCode;

    const std::uintmax_t size =
        std::filesystem::file_size(
            m_dbPath,
            errorCode);

    if (errorCode)
    {
        return 0;
    }

    return size;
}

bool DatabaseManager::getAdminPassword(
    std::string& hash,
    std::string& salt)
{
    if (!m_db)
    {
        return false;
    }

    const char* sql =
        "SELECT password_hash, password_salt "
        "FROM admin "
        "WHERE id = 1;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(
        m_db,
        sql,
        -1,
        &stmt,
        nullptr) != SQLITE_OK)
    {
        Logger::error("Admin SELECT prepare failed");

        return false;
    }

    bool found = false;

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const unsigned char* hashText =
            sqlite3_column_text(stmt, 0);

        const unsigned char* saltText =
            sqlite3_column_text(stmt, 1);

        if (hashText && saltText)
        {
            hash =
                reinterpret_cast<const char*>(
                    hashText);

            salt =
                reinterpret_cast<const char*>(
                    saltText);

            found = true;
        }
    }

    sqlite3_finalize(stmt);

    return found;
}

bool DatabaseManager::setAdminPassword(
    const std::string& hash,
    const std::string& salt)
{
    if (!m_db)
    {
        return false;
    }

    const char* sql =
        "INSERT INTO admin (id, password_hash, password_salt) "
        "VALUES (1, ?, ?) "
        "ON CONFLICT(id) DO UPDATE SET "
        "password_hash = excluded.password_hash, "
        "password_salt = excluded.password_salt;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(
        m_db,
        sql,
        -1,
        &stmt,
        nullptr) != SQLITE_OK)
    {
        Logger::error("Admin UPSERT prepare failed");

        return false;
    }

    sqlite3_bind_text(stmt, 1, hash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, salt.c_str(), -1, SQLITE_TRANSIENT);

    const bool ok =
        sqlite3_step(stmt) == SQLITE_DONE;

    sqlite3_finalize(stmt);

    return ok;
}