#pragma once

#include "core/ClipboardItem.h"
#include "core/LRUCache.h"
#include "core/Config.h"
#include "storage/Database.h"

#include <vector>
#include <string>
#include <memory>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

namespace cliphub {

class HistoryRepository {
public:
    HistoryRepository(std::shared_ptr<Database> db, const Config& config);
    ~HistoryRepository();

    // Initialization: loads recent items from SQLite into LRU cache
    bool initialize();
    void shutdown();

    // Adding/updating items
    // Returns true if new, false if duplicate (which was bumped to front)
    bool addItem(ClipboardItem item);

    // Pinning / Unpinning
    bool setPinned(int64_t id, bool pinned);

    // Deletion
    bool deleteItem(int64_t id);
    bool clearAll(bool keepPinned = true);

    // Query methods (served directly from RAM cache for 0ms latency!)
    std::vector<ClipboardItem> getCachedItems() const;
    std::vector<ClipboardItem> search(const std::string& query);

    // Reconfigure cache size
    void updateCapacity(size_t newCapacity);

private:
    void workerThreadLoop();
    void queueDbTask(std::function<void()> task);

private:
    std::shared_ptr<Database> m_db;
    Config m_config;
    LRUCache m_cache;

    // Single background worker thread for non-blocking HDD I/O (Section 21 & 22)
    std::thread m_worker;
    std::queue<std::function<void()>> m_taskQueue;
    mutable std::mutex m_queueMutex;
    std::condition_variable m_cv;
    std::atomic<bool> m_stopWorker{false};
};

} // namespace cliphub
