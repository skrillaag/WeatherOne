// User.h
#ifndef USER_H
#define USER_H

#include <string>

// I model User as a lightweight data holder with no business logic.
class User {
public:
    User(int id, const std::string& username, const std::string& hashedPassword);

    int getId() const;
    const std::string& getUsername() const;
    const std::string& getHashedPassword() const;

private:
    int id;
    std::string username;
    std::string hashedPassword;
};

#endif // USER_H
