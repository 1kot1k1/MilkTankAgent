#include "Logger.h"

#include <Windows.h>
#include <utility>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <algorithm>
#include <vector>

namespace
{
    std::mutex logMutex;
    struct LogSubscription
    {
        Logger::SubscriptionId id = 0;
        Logger::Callback callback;
    };

    std::vector<LogSubscription> logSubscriptions;

    Logger::SubscriptionId nextSubscriptionId = 1;

    constexpr std::uintmax_t maxLogSize =
        5 * 1024 * 1024;

    constexpr int maxBackupFiles = 3;

    std::filesystem::path getLogFilePath()
    {
        wchar_t executablePath[MAX_PATH];

        const DWORD length =
            GetModuleFileNameW(
                nullptr,
                executablePath,
                MAX_PATH);

        if (length == 0 || length == MAX_PATH)
        {
            return std::filesystem::path(
                "logs/MilkTankAgent.log");
        }

        const std::filesystem::path applicationDirectory =
            std::filesystem::path(
                executablePath)
            .parent_path();

        const std::filesystem::path logDirectory =
            applicationDirectory / "logs";

        std::filesystem::create_directories(
            logDirectory);

        return logDirectory /
            "MilkTankAgent.log";
    }

    void rotateLogs(
        const std::filesystem::path& logFilePath)
    {
        namespace fs = std::filesystem;

        if (!fs::exists(logFilePath))
        {
            return;
        }

        if (fs::file_size(logFilePath) < maxLogSize)
        {
            return;
        }

        const fs::path oldestBackup =
            logFilePath.string()
            + "."
            + std::to_string(maxBackupFiles);

        if (fs::exists(oldestBackup))
        {
            fs::remove(oldestBackup);
        }

        for (int index = maxBackupFiles - 1;
            index >= 1;
            --index)
        {
            const fs::path source =
                logFilePath.string()
                + "."
                + std::to_string(index);

            const fs::path destination =
                logFilePath.string()
                + "."
                + std::to_string(index + 1);

            if (fs::exists(source))
            {
                fs::rename(
                    source,
                    destination);
            }
        }

        const fs::path firstBackup =
            logFilePath.string() + ".1";

        fs::rename(
            logFilePath,
            firstBackup);
    }
}
Logger::SubscriptionId Logger::subscribe(
    Callback callback)
{
    if (!callback)
    {
        return 0;
    }

    std::lock_guard<std::mutex> lock(
        logMutex);

    const SubscriptionId id =
        nextSubscriptionId++;

    logSubscriptions.push_back(
        { id, std::move(callback) });

    return id;
}

void Logger::unsubscribe(
    SubscriptionId id)
{
    std::lock_guard<std::mutex> lock(
        logMutex);

    logSubscriptions.erase(
        std::remove_if(
            logSubscriptions.begin(),
            logSubscriptions.end(),
            [id](const LogSubscription& subscription)
            {
                return subscription.id == id;
            }),
        logSubscriptions.end());
}
void Logger::info(
    const std::string& message)
{
    write("INFO", message);
}

void Logger::warning(
    const std::string& message)
{
    write("WARNING", message);
}

void Logger::error(
    const std::string& message)
{
    write("ERROR", message);
}

void Logger::write(
    const std::string& level,
    const std::string& message)
{
    const auto now =
        std::chrono::system_clock::now();

    const std::time_t time =
        std::chrono::system_clock::to_time_t(
            now);

    std::tm localTime{};

    localtime_s(
        &localTime,
        &time);

    std::ostringstream line;

    line
        << "["
        << std::put_time(
            &localTime,
            "%Y-%m-%d %H:%M:%S")
        << "] "
        << level
        << " : "
        << message;

    const std::string formattedLine =
        line.str();

    std::vector<Callback> callbacks;

    {
        std::lock_guard<std::mutex> lock(
            logMutex);

        std::cout
            << formattedLine
            << std::endl;

        try
        {
            const std::filesystem::path logFilePath =
                getLogFilePath();

            rotateLogs(logFilePath);

            std::ofstream file(
                logFilePath,
                std::ios::app);

            if (file.is_open())
            {
                file
                    << formattedLine
                    << std::endl;
            }
        }
        catch (const std::exception& exception)
        {
            std::cerr
                << "Cannot write log file: "
                << exception.what()
                << std::endl;
        }

        callbacks.reserve(
            logSubscriptions.size());

        for (const auto& subscription :
            logSubscriptions)
        {
            callbacks.push_back(
                subscription.callback);
        }
    }

    for (const auto& callback : callbacks)
    {
        if (callback)
        {
            callback(formattedLine);
        }
    }
}