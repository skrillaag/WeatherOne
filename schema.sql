-- I explicitly enable foreign key enforcement.
-- SQLite does not enforce foreign keys by default, so this is required
-- for ON DELETE CASCADE to actually work.
PRAGMA foreign_keys = ON;

-- I store users separately to keep authentication concerns isolated.
-- Usernames are unique, and passwords are stored as hashes (not plaintext).
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT NOT NULL UNIQUE,
    password TEXT NOT NULL
);

-- I log every weather query to support history and auditing.
-- Each entry is tied to a user and automatically removed if the user
-- is deleted (via ON DELETE CASCADE).
CREATE TABLE IF NOT EXISTS query_logs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    city TEXT NOT NULL,
    summary TEXT NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

-- I index user_id because history lookups are always user-scoped.
CREATE INDEX IF NOT EXISTS idx_query_logs_user_id ON query_logs(user_id);

-- I index timestamp to keep recent-history queries fast.
CREATE INDEX IF NOT EXISTS idx_query_logs_timestamp ON query_logs(timestamp);
