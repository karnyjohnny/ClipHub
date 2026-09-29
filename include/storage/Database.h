#pragma once

#include "core/ClipboardItem.h"
#include <string>
#include <vector>
#include <memory>
#include <mutex>

struct sqlite3;
struct sqlite3_stmt;

namespace cliphub {

class Database {
public:
    Database();
    ~Database();

    // Lifecycle
    bool open(const std::string& dbPath);
    void close();
    bool isOpen() const;

    // Operations
    bool insertItem(ClipboardItem& item);
    bool updateItemUsage(int64_t id, int64_t timestamp);
    bool updateItemUsageByHash(uint64_t hash, int64_t timestamp, int64_t& outId);
    bool deleteItem(int64_t id);
    bool setPinned(int64_t id, bool pinned);
    bool clearAll(bool keepPinned = true);

    // Queries
    std::vector<ClipboardItem> getRecentItems(int limit = 50);
    std::vector<ClipboardItem> searchItems(const std::string& query, int limit = 50);
    bool findByHash(uint64_t hash, ClipboardItem& outItem);

    // Prune old unpinned items beyond retention limit
    bool pruneToLimit(int maxEntries);

    // Error diagnostics
    std::string getLastError() const;

private:
    bool initializeSchema();
    void finalizeStatements();

private:
    sqlite3* m_db = nullptr;
    mutable std::mutex m_mutex;
    std::string m_lastError;

    // Cached Prepared Statements
    sqlite3_stmt* m_stmtInsert = nullptr;
    sqlite3_stmt* m_stmtUpdateUsage = nullptr;
    sqlite3_stmt* m_stmtUpdateUsageByHash = nullptr;
    sqlite3_stmt* m_stmtDelete = nullptr;
    sqlite3_stmt* m_stmtSetPinned = nullptr;
    sqlite3_stmt* m_stmtFindByHash = nullptr;
};

} // namespace cliphub
