#pragma once

#include <cstddef>
#include <functional>
#include <string>

#define DEBUG_LOG 0

class Logger
{
public:
    using Callback =
        std::function<void(const std::string&)>;

    using SubscriptionId =
        std::size_t;

    static void info(
        const std::string& message);

    static void warning(
        const std::string& message);

    static void error(
        const std::string& message);

    static SubscriptionId subscribe(
        Callback callback);

    static void unsubscribe(
        SubscriptionId id);

    // Forces any buffered log lines to be written to disk right now,
    // instead of waiting for the usual batching interval. Safe to
    // call any time; a no-op if nothing is buffered.
    static void flush();

private:
    static void write(
        const std::string& level,
        const std::string& message);
};