#pragma once
#include <string>
#include <unordered_map>
#include <mutex>

// I keep session data minimal and decoupled from persistence.
struct Session {
    int userId;
    std::string username;
};

class SessionManager {
public:
    // I create sessions by issuing opaque tokens mapped to user state.
    std::string createSession(int userId, const std::string& username);

    // I validate tokens by resolving them into session data.
    bool validateToken(const std::string& token, Session& outSession);

private:
    // I centralize token generation so format and entropy stay consistent.
    std::string generateToken();

    // I store sessions in memory since they are short-lived
    // and scoped to the server runtime.
    std::unordered_map<std::string, Session> sessions;

    // I protect session access to allow safe concurrent requests.
    std::mutex mutex;
};
