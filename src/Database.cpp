#include "Database.h"
#include <stdexcept>

// I open the database immediately so failure is explicit and fatal.
// This keeps the rest of the application from running in a bad state.
Database::Database(const std::string& filename) : db(nullptr) {
    if (sqlite3_open(filename.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Failed to open database");
    }

    // I create tables on startup so the application can run
    // without requiring a separate migration step.
    execute(
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE,"
        "password TEXT);"
    );

    execute(
        "CREATE TABLE IF NOT EXISTS query_logs ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER,"
        "city TEXT,"
        "summary TEXT,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);"
    );
}

// I close the database explicitly to avoid leaking resources.
Database::~Database() {
    if (db) sqlite3_close(db);
}

// I centralize raw SQL execution so error handling stays consistent.
void Database::execute(const std::string& sql) {
    char* err = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err;
        sqlite3_free(err);
        throw std::runtime_error(msg);
    }
}

// I insert users using prepared statements to avoid SQL injection
// and to keep credential handling safe.
bool Database::createUser(const std::string& username, const std::string& passwordHash) {
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO users (username, password) VALUES (?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, passwordHash.c_str(), -1, SQLITE_TRANSIENT);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

// I authenticate by matching hashed credentials and returning
// the user id instead of a boolean for downstream use.
int Database::authenticateUser(const std::string& username, const std::string& passwordHash) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT id FROM users WHERE username = ? AND password = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return -1;

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, passwordHash.c_str(), -1, SQLITE_TRANSIENT);

    int userId = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        userId = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return userId;
}

// I log each weather query so history can be reconstructed later.
// Failures here are intentionally ignored to avoid blocking the main flow.
void Database::logQuery(int userId, const std::string& city, const std::string& summary) {
    sqlite3_stmt* stmt;
    const char* sql =
        "INSERT INTO query_logs (user_id, city, summary) VALUES (?, ?, ?);";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_text(stmt, 2, city.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, summary.c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

// I return history ordered by most recent first since that’s
// the only way it’s consumed by the UI.
std::vector<HistoryRow> Database::getHistory(int userId) {
    sqlite3_stmt* stmt;
    const char* sql =
        "SELECT timestamp, city, summary FROM query_logs "
        "WHERE user_id = ? ORDER BY timestamp DESC;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, userId);

    std::vector<HistoryRow> rows;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        HistoryRow r;
        r.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        r.city      = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        r.summary   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        rows.push_back(r);
    }

    sqlite3_finalize(stmt);
    return rows;
}
