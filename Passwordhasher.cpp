#include "PasswordHasher.h"

#include <windows.h>
#include <bcrypt.h>

#include <iomanip>
#include <sstream>
#include <vector>

#pragma comment(lib, "bcrypt.lib")

namespace
{
    std::string toHex(
        const std::vector<BYTE>& data)
    {
        std::ostringstream stream;

        for (BYTE byte : data)
        {
            stream
                << std::hex
                << std::setw(2)
                << std::setfill('0')
                << static_cast<int>(byte);
        }

        return stream.str();
    }
}

std::string PasswordHasher::generateSalt()
{
    std::vector<BYTE> buffer(16);

    const NTSTATUS status =
        BCryptGenRandom(
            nullptr,
            buffer.data(),
            static_cast<ULONG>(buffer.size()),
            BCRYPT_USE_SYSTEM_PREFERRED_RNG);

    if (status != 0)
    {
        return "";
    }

    return toHex(buffer);
}

std::string PasswordHasher::hash(
    const std::string& password,
    const std::string& salt)
{
    const std::string combined =
        salt + password;

    BCRYPT_ALG_HANDLE algorithm = nullptr;

    NTSTATUS status =
        BCryptOpenAlgorithmProvider(
            &algorithm,
            BCRYPT_SHA256_ALGORITHM,
            nullptr,
            0);

    if (status != 0 ||
        algorithm == nullptr)
    {
        return "";
    }

    BYTE hashValue[32] = { 0 };

    status =
        BCryptHash(
            algorithm,
            nullptr,
            0,
            reinterpret_cast<PUCHAR>(
                const_cast<char*>(
                    combined.data())),
            static_cast<ULONG>(
                combined.size()),
            hashValue,
            sizeof(hashValue));

    BCryptCloseAlgorithmProvider(
        algorithm,
        0);

    if (status != 0)
    {
        return "";
    }

    const std::vector<BYTE> hashVector(
        hashValue,
        hashValue + sizeof(hashValue));

    return toHex(hashVector);
}