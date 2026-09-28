#pragma once

#include <string>

class PasswordHasher
{
public:
    static std::string generateSalt();

    static std::string hash(
        const std::string& password,
        const std::string& salt);
};