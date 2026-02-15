#include "SessionManager.h"
#include <random>
#include <sstream>

// I generate opaque session tokens instead of deriving them
// from user data to avoid leaking information.
std::string SessionManager::generateToken() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    // I generate a fixed-length hex string to keep tokens uniform.
    std::stringstream ss;
    for (int i = 0; i < 32; ++i) {
        ss << std::hex << dis(gen);
    }
    return ss.str();
}

std::string SessionManager::createSession(int userId, const std::string& username) {
    // I lock here to keep session creation thread-safe.
    std::lock_guard<std::mutex> lock(mutex);

    std::string token = generateToken();
    sessions[token] = { userId, username };
    return token;
}

bool SessionManager::validateToken(const std::string& token, Session& outSession) {
    // I lock here as well since validation reads shared state.
    std::lock_guard<std::mutex> lock(mutex);

    auto it = sessions.find(token);
    if (it == sessions.end())
        return false;

    outSession = it->second;
    return true;
}
