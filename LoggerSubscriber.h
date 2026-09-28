#pragma once

#include "EventBus.h"

class LoggerSubscriber
{
public:
    static void subscribe(EventBus& bus);
};