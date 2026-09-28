#pragma once

#include <string>

struct AppSettings
{
    std::string serverHost;
    std::string apiPath = "/dash/test/post/768a61e4-df8a-4866-be2a-8ab6f78ce328";
    std::string companyUuid;
    std::string farmUuid;
    std::string herdUuid;

    int pollInterval = 5;
    int sendInterval = 30;
    int retentionDays = 30;

    bool verifyTlsCertificate = false;
};