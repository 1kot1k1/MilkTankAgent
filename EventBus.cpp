#include "EventBus.h"

#include <algorithm>
#include <utility>

EventBus::SubscriptionId EventBus::subscribe(
    Callback callback)
{
    std::lock_guard<std::mutex> lock(
        m_mutex);

    const SubscriptionId id =
        m_nextId++;

    m_subscriptions.push_back(
        Subscription{
            id,
            std::move(callback)
        });

    return id;
}

void EventBus::unsubscribe(
    SubscriptionId id)
{
    std::lock_guard<std::mutex> lock(
        m_mutex);

    m_subscriptions.erase(
        std::remove_if(
            m_subscriptions.begin(),
            m_subscriptions.end(),
            [id](const Subscription& subscription)
            {
                return subscription.id == id;
            }),
        m_subscriptions.end());
}

void EventBus::publish(
    const DeviceEvent& event)
{
    std::vector<Subscription> subscriptions;

    {
        std::lock_guard<std::mutex> lock(
            m_mutex);

        subscriptions = m_subscriptions;
    }

    for (const auto& subscription : subscriptions)
    {
        subscription.callback(event);
    }
}