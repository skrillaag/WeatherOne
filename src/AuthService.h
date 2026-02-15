#pragma once
#include <string>
#include "Database.h"

// I keep AuthService responsible only for auth logic,
// and delegate all persistence to the Database.
class AuthService {
public:
    explicit AuthService(Database& db);

    // I register users by hashing the password before storing it.
    bool registerUser(const std::string& username, const std::string& password);

    // I return the user id on successful authentication,
    // and a negative value to signal failure.
    int loginUser(const std::string& username, const std::string& password);

private:
    // I store a reference to the database instead of owning it
    // to keep lifetime management external.
    Database& db;

    // I centralize password hashing so it stays consistent everywhere.
    std::string hashPassword(const std::string& password);
};
