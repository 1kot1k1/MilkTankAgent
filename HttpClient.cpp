#include "HttpClient.h"
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#include "json.hpp"
#include "Logger.h"

using json = nlohmann::json;

bool HttpClient::send(
    const std::string& serverHost,
    const std::string& apiPath,
    const std::string& companyUuid,
    const std::string& herdUuid,
    const std::string& farmUuid,
    const std::vector<DeviceData>& devices,
    bool verifyTlsCertificate)
{
    json request;

    request["c_uuid"] = companyUuid;
    request["h_uuid"] = herdUuid;
    request["f_uuid"] = farmUuid;
    request["data"] = json::array();

    for (const auto& device : devices)
    {
        json item;

        item["d_uuid"] = device.uuid;
        item["time"] = device.lastUpdate;
        item["weight"] = device.weight;
        item["stable"] = device.stable;
        item["overload"] = device.overload;
        item["online"] = device.online;

        request["data"].push_back(item);
    }

    std::string body = request.dump();

    HINTERNET hSession =
        WinHttpOpen(
            L"MilkTankAgent/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS,
            0);

    if (!hSession)
    {
        Logger::error("WinHttpOpen failed");
        return false;
    }

    std::wstring host(serverHost.begin(), serverHost.end());

    HINTERNET hConnect =
        WinHttpConnect(
            hSession,
            host.c_str(),
            INTERNET_DEFAULT_HTTPS_PORT,
            0);

    if (!hConnect)
    {
        Logger::error("WinHttpConnect failed");

        WinHttpCloseHandle(hSession);

        return false;
    }

    std::wstring path(apiPath.begin(), apiPath.end());

    HINTERNET hRequest =
        WinHttpOpenRequest(
            hConnect,
            L"POST",
            path.c_str(),
            NULL,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            WINHTTP_FLAG_SECURE);

    DWORD protocols =
        WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2 |
        WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_3;

    WinHttpSetOption(
        hSession,
        WINHTTP_OPTION_SECURE_PROTOCOLS,
        &protocols,
        sizeof(protocols));

    if (!hRequest)
    {
        Logger::error("WinHttpOpenRequest failed");

        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);

        return false;
    }

    if (!verifyTlsCertificate)
    {
        Logger::warning(
            "TLS certificate validation is disabled. "
            "This must not be used in production.");

        DWORD flags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
            SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
            SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
            SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;

        WinHttpSetOption(
            hRequest,
            WINHTTP_OPTION_SECURITY_FLAGS,
            &flags,
            sizeof(flags));
    }

    const wchar_t* headers =
        L"Content-Type: application/json\r\n";

    BOOL ok =
        WinHttpSendRequest(
            hRequest,
            headers,
            -1L,
            (LPVOID)body.c_str(),
            (DWORD)body.size(),
            (DWORD)body.size(),
            0);

    if (!ok)
    {
        DWORD err = GetLastError();

        Logger::error(
            "WinHttpSendRequest failed (" +
            std::to_string(err) + ")");
        Logger::error("Trying to connect to: https://" + serverHost);

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);

        return false;
    }

    ok = WinHttpReceiveResponse(
        hRequest,
        NULL);

    if (!ok)
    {
        Logger::error("WinHttpReceiveResponse failed");

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);

        return false;
    }

    DWORD statusCode = 0;
    DWORD statusSize = sizeof(statusCode);

    WinHttpQueryHeaders(
        hRequest,
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX,
        &statusCode,
        &statusSize,
        WINHTTP_NO_HEADER_INDEX);

    DWORD size = 0;
    std::string response;

    do
    {
        size = 0;

        WinHttpQueryDataAvailable(
            hRequest,
            &size);

        if (size == 0)
            break;

        std::string buffer(size, '\0');

        DWORD downloaded = 0;

        WinHttpReadData(
            hRequest,
            buffer.data(),
            size,
            &downloaded);

        response.append(buffer.data(), downloaded);

    } while (size > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    if (statusCode < 200 || statusCode >= 300)
    {
        Logger::error(
            "Server returned HTTP status " +
            std::to_string(statusCode) +
            ". Response: " +
            response);

        return false;
    }

    Logger::info(
        "Server: OK (HTTP " +
        std::to_string(statusCode) +
        ")");

#if DEBUG_LOG
    Logger::info(body);
    Logger::info(response);
#endif

    return true;
}