#pragma once

#include <string>
#include <vector>

#include "DeviceData.h"

class HttpClient
{
public:

    bool send(
        const std::string& serverHost,
        const std::string& apiPath,
        const std::string& companyUuid,
        const std::string& herdUuid,
        const std::string& farmUuid,
        const std::vector<DeviceData>& devices,
        bool verifyTlsCertificate);

};