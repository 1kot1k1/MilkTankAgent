#pragma once

#include <mutex>
#include <string>
#include <unordered_set>

class SessionManager
{
public:
    // Generates a new random token and remembers it as valid.
    std::string issueToken();

    bool isValid(const std::string& token) const;

private:
    mutable std::mutex m_mutex;
    std::unordered_set<std::string> m_validTokens;
};