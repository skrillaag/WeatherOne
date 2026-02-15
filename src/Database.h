#pragma once
#include <string>
#include <vector>
#include <sqlite3.h>

// I use a simple data struct to move history rows
// between the database layer and the rest of the app.
struct HistoryRow {
    std::string timestamp;
    std::string city;
    std::string summary;
};

class Database {
public:
    // I require the database filename at construction
    // so the connection is always valid after creation.
    Database(const std::string& filename);
    ~Database();

    // I expose only high-level operations instead of raw SQL.
    bool createUser(const std::string& username, const std::string& passwordHash);
    int authenticateUser(const std::string& username, const std::string& passwordHash);

    void logQuery(int userId, const std::string& city, const std::string& summary);
    std::vector<HistoryRow> getHistory(int userId);

private:
    // I keep the raw SQLite handle private to avoid leaking DB concerns.
    sqlite3* db;

    // I centralize raw SQL execution to keep error handling consistent.
    void execute(const std::string& sql);
};
