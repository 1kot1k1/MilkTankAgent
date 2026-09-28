#include "PasswordHasher.h"

#include <iostream>

int main()
{
    const std::string salt = PasswordHasher::generateSalt();
    const std::string hash = PasswordHasher::hash("1234", salt);

    std::cout << "salt: " << salt << std::endl;
    std::cout << "hash: " << hash << std::endl;

    if (salt.empty() || hash.empty())
    {
        std::cout << "FAILED" << std::endl;
        return 1;
    }

    std::cout << "OK" << std::endl;
    return 0;
}