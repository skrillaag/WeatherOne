#pragma once
#include <string>

#include "Database.h"
#include "AuthService.h"
#include "SessionManager.h"

// I keep HttpServer focused on request routing and coordination,
// not business logic or persistence.
class HttpServer {
public:
    // I inject all dependencies so ownership and lifetimes
    // are managed by the application, not the server.
    HttpServer(const std::string& address, int port, Database& db, AuthService& auth);

    // I run the server in a blocking loop to keep control flow explicit.
    void run();

private:
    // I store address and port explicitly to avoid hidden configuration.
    std::string address;
    int port;

    // I hold references to shared services instead of owning them.
    Database& db;
    AuthService& auth;

    // I keep session state local to the server boundary.
    SessionManager sessions;
};
