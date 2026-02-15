#include "AuthService.h"
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

// I inject the Database dependency so AuthService stays focused
// only on auth logic and not persistence details.
AuthService::AuthService(Database& database) : db(database) {}

// I hash passwords using SHA-256 before storing or comparing them.
// This keeps plaintext passwords out of the database entirely.
std::string AuthService::hashPassword(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()),
           password.size(), hash);

    // I convert the raw hash bytes into a hex string for storage.
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

// I hash the password before passing it to the database
// so the database never sees the raw password.
bool AuthService::registerUser(const std::string& username, const std::string& password) {
    return db.createUser(username, hashPassword(password));
}

// I authenticate by hashing the input password and comparing hashes,
// not by comparing plaintext credentials.
int AuthService::loginUser(const std::string& username, const std::string& password) {
    return db.authenticateUser(username, hashPassword(password));
}
