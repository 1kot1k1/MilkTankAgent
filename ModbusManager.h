#pragma once
#include "Device.h"
#include "PollResult.h"
#include <windows.h>
class ModbusManager
{
public:
    PollResult poll(const Device& device);

private:
    HANDLE m_serial = INVALID_HANDLE_VALUE;
};