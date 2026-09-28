#include "ModbusManager.h"
#include "Logger.h"
#include <windows.h>


int parseBcd(const BYTE* data, int length)
{
    int result = 0;
    int multiplier = 1; 

    for (int i = 0; i < length; i++)
    {
        int lowNibble = data[i] & 0x0F;       
        int highNibble = (data[i] >> 4) & 0x0F; 

        result += (highNibble * 10 + lowNibble) * multiplier;

        multiplier *= 100;
    }

    return result;
}

double decodeWeight(const BYTE* buffer, DWORD bytesRead)
{
    int startIndex = -1;

    for (DWORD i = 0; i + 1 < bytesRead; i++)
    {
        if (buffer[i] == 0x03 &&
            buffer[i + 1] == 0xC3)
        {
            startIndex = static_cast<int>(i);
            break;
        }
    }

    if (startIndex == -1)
        return 0.0;

    int rawWeight = parseBcd(buffer + startIndex + 2, 3);

    BYTE flags = buffer[startIndex + 5];

    bool negative = (flags & 0x80) != 0;

    int pointBits = flags & 0x07;

    double scale = 1.0;

    switch (pointBits)
    {
    case 0:
        scale = 1.0;
        break;

    case 1:
        scale = 10.0;
        break;

    case 2:
        scale = 100.0;
        break;

    case 3:
        scale = 1000.0;
        break;

    case 4:
        scale = 10000.0;
        break;

    default:
        scale = 1000.0; 
        break;
    }

    double weight = rawWeight / scale;

    if (negative)
        weight = -weight;

    return weight;
}

PollResult ModbusManager::poll(const Device& device)
{

    m_serial = CreateFileA(
        device.port.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr);

    if (m_serial == INVALID_HANDLE_VALUE)
    {
        Logger::error("Cannot open " + device.port);

        PollResult result;
        result.success = false;
        result.error = "Cannot open COM port";


        return result;
    }


    DCB dcb = { 0 };

    dcb.DCBlength = sizeof(dcb);


    

    if (!GetCommState(m_serial, &dcb))
    {
        Logger::error("GetCommState failed.");

        CloseHandle(m_serial);
        m_serial = INVALID_HANDLE_VALUE;

        PollResult result;
        result.success = false;
        result.error = "GetCommState failed";

        return result;
    }
    dcb.BaudRate = device.baudRate;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    if (!SetCommState(m_serial, &dcb))
    {
        Logger::error("SetCommState failed.");

        CloseHandle(m_serial);
        m_serial = INVALID_HANDLE_VALUE;

        PollResult result;
        result.success = false;
        result.error = "SetCommState failed";

        return result;
    }

    COMMTIMEOUTS timeouts = {};

    timeouts.ReadIntervalTimeout = 500;
    timeouts.ReadTotalTimeoutConstant = 500;
    timeouts.ReadTotalTimeoutMultiplier = 0;

    timeouts.WriteTotalTimeoutConstant = 500;
    timeouts.WriteTotalTimeoutMultiplier = 0;


    if (!SetCommTimeouts(m_serial, &timeouts))
    {
        Logger::error("SetCommTimeouts failed.");

        CloseHandle(m_serial);
        m_serial = INVALID_HANDLE_VALUE;

        PollResult result;
        result.success = false;
        result.error = "SetCommTimeouts failed";

        return result;
    }
    
    PurgeComm(
        m_serial,
        PURGE_RXCLEAR | PURGE_TXCLEAR);

    BYTE command[] =
    {
        0xFF,
        0x03,
        0xC3,
        0xE5,
        0xFF,
        0xFF
    };

    DWORD written = 0;

    if (!WriteFile(
        m_serial,
        command,
        sizeof(command),
        &written,
        nullptr))
    {
        Logger::error("WriteFile failed.");

        CloseHandle(m_serial);
        m_serial = INVALID_HANDLE_VALUE;

        PollResult result;
        result.success = false;
        result.error = "Write failed";

        return result;
    }


    BYTE buffer[16] = { 0 };

    DWORD bytesRead = 0;

    if (!ReadFile(
        m_serial,
        buffer,
        sizeof(buffer),
        &bytesRead,
        nullptr))
    {
        Logger::error("ReadFile failed.");

        CloseHandle(m_serial);
        m_serial = INVALID_HANDLE_VALUE;

        PollResult result;
        result.success = false;
        result.error = "Read failed";

        return result;
    }
    if (bytesRead < 7)
    {
        Logger::warning(
            "No valid response from "
            + device.name
            + ". Bytes received: "
            + std::to_string(bytesRead));

        CloseHandle(m_serial);
        m_serial = INVALID_HANDLE_VALUE;

        PollResult result;
        result.success = false;
        result.error = "Response timeout or incomplete response";

        return result;
    }
#if DEBUG_LOG
    std::string hex;

    char temp[4];

    for (DWORD i = 0; i < bytesRead; i++)
    {
        sprintf_s(temp, "%02X ", buffer[i]);
        hex += temp;
    }

    Logger::info("HEX: " + hex);

    if (bytesRead >= 7)
    {
        BYTE status = buffer[6];

        Logger::info(
            "Status = " +
            std::to_string(status));

        std::string bits;

        for (int i = 7; i >= 0; --i)
        {
            bits += ((status >> i) & 1) ? '1' : '0';
        }

        Logger::info("Status bits = " + bits);
    }
#endif

    CloseHandle(m_serial);
    m_serial = INVALID_HANDLE_VALUE;

    PollResult result;
          

    

    result.success = true;
    result.weight = decodeWeight(buffer, bytesRead);
    result.status = buffer[6];

    result.stable = (result.status & 0x10) != 0;

    result.overload = (result.status & 0x40) != 0;

    

    return result;
    
}
