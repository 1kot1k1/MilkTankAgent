#include "SessionManager.h"
#include "PasswordHasher.h"

std::string SessionManager::issueToken()
{
    // Reuse the same secure random generator already used for
    // password salts — good enough entropy for a session token.
    std::string token = PasswordHasher::generateSalt();

    // generateSalt() returns 16 random bytes as hex; combine two
    // for a longer, harder to guess token.
    token += PasswordHasher::generateSalt();

    std::lock_guard<std::mutex> lock(m_mutex);
    m_validTokens.insert(token);

    return token;
}

bool SessionManager::isValid(const std::string& token) const
{
    if (token.empty())
    {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    return m_validTokens.find(token) != m_validTokens.end();
}