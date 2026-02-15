// User.cpp
#include "User.h"

// I treat User as a simple value object with no behavior beyond accessors.
User::User(int id, const std::string& username, const std::string& hashedPassword)
    : id(id), username(username), hashedPassword(hashedPassword) {}

int User::getId() const { return id; }
const std::string& User::getUsername() const { return username; }
const std::string& User::getHashedPassword() const { return hashedPassword; }
