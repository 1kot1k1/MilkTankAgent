#include "LoggerSubscriber.h"
#include "Logger.h"

#include <iomanip>
#include <sstream>

void LoggerSubscriber::subscribe(EventBus& bus)
{
    bus.subscribe(
        [](const DeviceEvent& event)
        {
            switch (event.type)
            {
            case DeviceEventType::Online:
                Logger::info(
                    event.name + " ONLINE");
                break;

            case DeviceEventType::Offline:
                Logger::warning(
                    event.name + " OFFLINE");
                break;

            case DeviceEventType::Disabled:
                Logger::info(
                    event.name + " DISABLED");
                break;

            case DeviceEventType::Added:
                Logger::info(
                    "[+] New device: " +
                    event.name);
                break;

            case DeviceEventType::Removed:
                Logger::info(
                    "[-] Removed device: " +
                    event.name);
                break;

            case DeviceEventType::Updated:
                Logger::info(
                    "[*] Updated device: " +
                    event.name);
                break;

            case DeviceEventType::Overload:
                Logger::warning(
                    event.name +
                    " : OVERLOAD");
                break;

            case DeviceEventType::Unstable:
                Logger::warning(
                    event.name +
                    " : UNSTABLE");
                break;

            case DeviceEventType::WeightChanged:
            {
                std::ostringstream ss;

                std::string unit;
                double value = event.weight;

                if (value >= 1000.0)
                {
                    value /= 1000.0;
                    unit = " kg";

                    if (value < 10.0)
                    {
                        ss << std::fixed
                            << std::setprecision(2);
                    }
                    else if (value < 100.0)
                    {
                        ss << std::fixed
                            << std::setprecision(1);
                    }
                    else
                    {
                        ss << std::fixed
                            << std::setprecision(0);
                    }
                }
                else
                {
                    unit = " g";

                    ss << std::fixed
                        << std::setprecision(0);
                }

                ss << value;

                Logger::info(
                    event.name +
                    " : " +
                    ss.str() +
                    unit);

                break;
            }
            }
        });
}