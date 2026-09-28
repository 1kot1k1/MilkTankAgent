#pragma once

#include <cstddef>
#include <functional>
#include <mutex>
#include <vector>

#include "DeviceEvent.h"

class EventBus
{
public:
    using Callback =
        std::function<void(const DeviceEvent&)>;

    using SubscriptionId = std::size_t;

    SubscriptionId subscribe(
        Callback callback);

    void unsubscribe(
        SubscriptionId id);

    void publish(
        const DeviceEvent& event);

private:
    struct Subscription
    {
        SubscriptionId id;
        Callback callback;
    };

    std::vector<Subscription> m_subscriptions;

    SubscriptionId m_nextId = 1;

    std::mutex m_mutex;
};