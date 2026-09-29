#include "storage/Database.h"
#include "sqlite3.h"
#include <iostream>

namespace cliphub {

Database::Database() = default;

Database::~Database() {
    close();
}

bool Database::isOpen() const {
    return m_db != nullptr;
}

std::string Database::getLastError() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_lastError;
}

void Database::finalizeStatements() {
    if (m_stmtInsert) { sqlite3_finalize(m_stmtInsert); m_stmtInsert = nullptr; }
    if (m_stmtUpdateUsage) { sqlite3_finalize(m_stmtUpdateUsage); m_stmtUpdateUsage = nullptr; }
    if (m_stmtUpdateUsageByHash) { sqlite3_finalize(m_stmtUpdateUsageByHash); m_stmtUpdateUsageByHash = nullptr; }
    if (m_stmtDelete) { sqlite3_finalize(m_stmtDelete); m_stmtDelete = nullptr; }
    if (m_stmtSetPinned) { sqlite3_finalize(m_stmtSetPinned); m_stmtSetPinned = nullptr; }
    if (m_stmtFindByHash) { sqlite3_finalize(m_stmtFindByHash); m_stmtFindByHash = nullptr; }
}

bool Database::open(const std::string& dbPath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    close();

    int rc = sqlite3_open_v2(dbPath.c_str(), &m_db, 
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, 
                             nullptr);
    if (rc != SQLITE_OK) {
        m_lastError = sqlite3_errmsg(m_db);
        if (m_db) {
            sqlite3_close(m_db);
            m_db = nullptr;
        }
        return false;
    }

    // Configure performance PRAGMAs optimized for HDD and Windows 7
    sqlite3_exec(m_db, "PRAGMA journal_mode = WAL;", nullptr, nullptr, nullptr);
    sqlite3_exec(m_db, "PRAGMA synchronous = NORMAL;", nullptr, nullptr, nullptr);
    sqlite3_exec(m_db, "PRAGMA cache_size = -2000;", nullptr, nullptr, nullptr); // ~2MB RAM
    sqlite3_exec(m_db, "PRAGMA temp_store = MEMORY;", nullptr, nullptr, nullptr);
    sqlite3_exec(m_db, "PRAGMA busy_timeout = 3000;", nullptr, nullptr, nullptr);

    return initializeSchema();
}

bool Database::initializeSchema() {
    const char* schemaSql = 
        "CREATE TABLE IF NOT EXISTS clipboard_history ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  type INTEGER NOT NULL,"
        "  created_at INTEGER NOT NULL,"
        "  last_used_at INTEGER NOT NULL,"
        "  content_hash INTEGER NOT NULL,"
        "  text_content TEXT,"
        "  preview_text TEXT,"
        "  char_count INTEGER DEFAULT 0,"
        "  image_width INTEGER DEFAULT 0,"
        "  image_height INTEGER DEFAULT 0,"
        "  image_thumbnail BLOB,"
        "  storage_path TEXT,"
        "  pinned INTEGER DEFAULT 0"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_content_hash ON clipboard_history(content_hash);"
        "CREATE INDEX IF NOT EXISTS idx_last_used ON clipboard_history(last_used_at DESC);";

    char* err = nullptr;
    int rc = sqlite3_exec(m_db, schemaSql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        if (err) {
            m_lastError = err;
            sqlite3_free(err);
        }
        return false;
    }

    // Prepare frequently used statements
    const char* sqlInsert = 
        "INSERT INTO clipboard_history ("
        "  type, created_at, last_used_at, content_hash, text_content, preview_text,"
        "  char_count, image_width, image_height, image_thumbnail, storage_path, pinned"
        ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_prepare_v2(m_db, sqlInsert, -1, &m_stmtInsert, nullptr);

    const char* sqlUpdateUsage = 
        "UPDATE clipboard_history SET last_used_at = ? WHERE id = ?;";
    sqlite3_prepare_v2(m_db, sqlUpdateUsage, -1, &m_stmtUpdateUsage, nullptr);

    const char* sqlUpdateUsageByHash = 
        "UPDATE clipboard_history SET last_used_at = ? WHERE content_hash = ?;";
    sqlite3_prepare_v2(m_db, sqlUpdateUsageByHash, -1, &m_stmtUpdateUsageByHash, nullptr);

    const char* sqlDelete = 
        "DELETE FROM clipboard_history WHERE id = ?;";
    sqlite3_prepare_v2(m_db, sqlDelete, -1, &m_stmtDelete, nullptr);

    const char* sqlSetPinned = 
        "UPDATE clipboard_history SET pinned = ? WHERE id = ?;";
    sqlite3_prepare_v2(m_db, sqlSetPinned, -1, &m_stmtSetPinned, nullptr);

    const char* sqlFindByHash = 
        "SELECT id, type, created_at, last_used_at, content_hash, text_content, preview_text,"
        "       char_count, image_width, image_height, image_thumbnail, storage_path, pinned "
        "FROM clipboard_history WHERE content_hash = ? LIMIT 1;";
    sqlite3_prepare_v2(m_db, sqlFindByHash, -1, &m_stmtFindByHash, nullptr);

    return true;
}

void Database::close() {
    finalizeStatements();
    if (m_db) {
        sqlite3_close_v2(m_db);
        m_db = nullptr;
    }
}

static ClipboardItem rowToItem(sqlite3_stmt* stmt) {
    ClipboardItem item;
    item.id = sqlite3_column_int64(stmt, 0);
    item.type = static_cast<ItemType>(sqlite3_column_int(stmt, 1));
    item.createdAt = sqlite3_column_int64(stmt, 2);
    item.lastUsedAt = sqlite3_column_int64(stmt, 3);
    item.contentHash = static_cast<uint64_t>(sqlite3_column_int64(stmt, 4));

    const unsigned char* text = sqlite3_column_text(stmt, 5);
    if (text) item.textContent = reinterpret_cast<const char*>(text);

    const unsigned char* prev = sqlite3_column_text(stmt, 6);
    if (prev) item.previewText = reinterpret_cast<const char*>(prev);

    item.charCount = static_cast<size_t>(sqlite3_column_int64(stmt, 7));
    item.imageMeta.width = static_cast<uint32_t>(sqlite3_column_int(stmt, 8));
    item.imageMeta.height = static_cast<uint32_t>(sqlite3_column_int(stmt, 9));

    const void* blob = sqlite3_column_blob(stmt, 10);
    int blobBytes = sqlite3_column_bytes(stmt, 10);
    if (blob && blobBytes > 0) {
        const uint8_t* p = static_cast<const uint8_t*>(blob);
        item.thumbnailData.assign(p, p + blobBytes);
        item.imageMeta.byteSize = blobBytes;
    }

    const unsigned char* path = sqlite3_column_text(stmt, 11);
    if (path) item.storagePath = reinterpret_cast<const char*>(path);

    item.pinned = (sqlite3_column_int(stmt, 12) != 0);
    return item;
}

bool Database::insertItem(ClipboardItem& item) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_db || !m_stmtInsert) return false;

    sqlite3_reset(m_stmtInsert);
    sqlite3_clear_bindings(m_stmtInsert);

    sqlite3_bind_int(m_stmtInsert, 1, static_cast<int>(item.type));
    sqlite3_bind_int64(m_stmtInsert, 2, item.createdAt);
    sqlite3_bind_int64(m_stmtInsert, 3, item.lastUsedAt);
    sqlite3_bind_int64(m_stmtInsert, 4, static_cast<int64_t>(item.contentHash));
    
    if (item.type == ItemType::Text) {
        sqlite3_bind_text(m_stmtInsert, 5, item.textContent.c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(m_stmtInsert, 5);
    }

    sqlite3_bind_text(m_stmtInsert, 6, item.previewText.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(m_stmtInsert, 7, static_cast<int64_t>(item.charCount));
    sqlite3_bind_int(m_stmtInsert, 8, static_cast<int>(item.imageMeta.width));
    sqlite3_bind_int(m_stmtInsert, 9, static_cast<int>(item.imageMeta.height));

    if (!item.thumbnailData.empty()) {
        sqlite3_bind_blob(m_stmtInsert, 10, item.thumbnailData.data(), 
                          static_cast<int>(item.thumbnailData.size()), SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(m_stmtInsert, 10);
    }

    if (!item.storagePath.empty()) {
        sqlite3_bind_text(m_stmtInsert, 11, item.storagePath.c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(m_stmtInsert, 11);
    }

    sqlite3_bind_int(m_stmtInsert, 12, item.pinned ? 1 : 0);

    int rc = sqlite3_step(m_stmtInsert);
    if (rc != SQLITE_DONE) {
        m_lastError = sqlite3_errmsg(m_db);
        return false;
    }

    item.id = sqlite3_last_insert_rowid(m_db);
    return true;
}

bool Database::updateItemUsage(int64_t id, int64_t timestamp) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_db || !m_stmtUpdateUsage) return false;

    sqlite3_reset(m_stmtUpdateUsage);
    sqlite3_bind_int64(m_stmtUpdateUsage, 1, timestamp);
    sqlite3_bind_int64(m_stmtUpdateUsage, 2, id);

    int rc = sqlite3_step(m_stmtUpdateUsage);
    return rc == SQLITE_DONE;
}

bool Database::updateItemUsageByHash(uint64_t hash, int64_t timestamp, int64_t& outId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_db || !m_stmtUpdateUsageByHash) return false;

    // First find ID
    sqlite3_reset(m_stmtFindByHash);
    sqlite3_bind_int64(m_stmtFindByHash, 1, static_cast<int64_t>(hash));
    if (sqlite3_step(m_stmtFindByHash) == SQLITE_ROW) {
        outId = sqlite3_column_int64(m_stmtFindByHash, 0);
    } else {
        return false;
    }

    sqlite3_reset(m_stmtUpdateUsageByHash);
    sqlite3_bind_int64(m_stmtUpdateUsageByHash, 1, timestamp);
    sqlite3_bind_int64(m_stmtUpdateUsageByHash, 2, static_cast<int64_t>(hash));

    return sqlite3_step(m_stmtUpdateUsageByHash) == SQLITE_DONE;
}

bool Database::deleteItem(int64_t id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_db || !m_stmtDelete) return false;

    sqlite3_reset(m_stmtDelete);
    sqlite3_bind_int64(m_stmtDelete, 1, id);

    return sqlite3_step(m_stmtDelete) == SQLITE_DONE;
}

bool Database::setPinned(int64_t id, bool pinned) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_db || !m_stmtSetPinned) return false;

    sqlite3_reset(m_stmtSetPinned);
    sqlite3_bind_int(m_stmtSetPinned, 1, pinned ? 1 : 0);
    sqlite3_bind_int64(m_stmtSetPinned, 2, id);

    return sqlite3_step(m_stmtSetPinned) == SQLITE_DONE;
}

bool Database::clearAll(bool keepPinned) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_db) return false;

    const char* sql = keepPinned 
        ? "DELETE FROM clipboard_history WHERE pinned = 0;" 
        : "DELETE FROM clipboard_history;";

    return sqlite3_exec(m_db, sql, nullptr, nullptr, nullptr) == SQLITE_OK;
}

std::vector<ClipboardItem> Database::getRecentItems(int limit) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<ClipboardItem> items;
    if (!m_db) return items;

    const char* sql = 
        "SELECT id, type, created_at, last_used_at, content_hash, text_content, preview_text,"
        "       char_count, image_width, image_height, image_thumbnail, storage_path, pinned "
        "FROM clipboard_history ORDER BY last_used_at DESC LIMIT ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, limit);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            items.push_back(rowToItem(stmt));
        }
        sqlite3_finalize(stmt);
    }

    return items;
}

std::vector<ClipboardItem> Database::searchItems(const std::string& query, int limit) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<ClipboardItem> items;
    if (!m_db || query.empty()) return items;

    const char* sql = 
        "SELECT id, type, created_at, last_used_at, content_hash, text_content, preview_text,"
        "       char_count, image_width, image_height, image_thumbnail, storage_path, pinned "
        "FROM clipboard_history "
        "WHERE (type = 0 AND text_content LIKE ?) OR preview_text LIKE ? "
        "ORDER BY last_used_at DESC LIMIT ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        std::string pattern = "%" + query + "%";
        sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, pattern.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, limit);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            items.push_back(rowToItem(stmt));
        }
        sqlite3_finalize(stmt);
    }

    return items;
}

bool Database::findByHash(uint64_t hash, ClipboardItem& outItem) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_db || !m_stmtFindByHash) return false;

    sqlite3_reset(m_stmtFindByHash);
    sqlite3_bind_int64(m_stmtFindByHash, 1, static_cast<int64_t>(hash));

    if (sqlite3_step(m_stmtFindByHash) == SQLITE_ROW) {
        outItem = rowToItem(m_stmtFindByHash);
        return true;
    }
    return false;
}

bool Database::pruneToLimit(int maxEntries) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_db) return false;

    // Prune oldest unpinned items that exceed maxEntries
    const char* sql = 
        "DELETE FROM clipboard_history "
        "WHERE pinned = 0 AND id NOT IN ("
        "  SELECT id FROM clipboard_history ORDER BY last_used_at DESC LIMIT ?"
        ");";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, maxEntries);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return true;
    }
    return false;
}

} // namespace cliphub
