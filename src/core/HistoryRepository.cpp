#include "core/HistoryRepository.h"
#include <algorithm>
#include <iostream>

namespace cliphub {

HistoryRepository::HistoryRepository(std::shared_ptr<Database> db, const Config& config)
    : m_db(std::move(db)), m_config(config), m_cache(config.historyLimit) {
}

HistoryRepository::~HistoryRepository() {
    shutdown();
}

bool HistoryRepository::initialize() {
    if (!m_db || !m_db->isOpen()) {
        return false;
    }

    // Warm up hot RAM cache from SQLite (in reverse order so newest is at the top)
    auto recentItems = m_db->getRecentItems(m_config.historyLimit);
    for (auto it = recentItems.rbegin(); it != recentItems.rend(); ++it) {
        m_cache.put(*it);
    }

    // Start single worker thread for background persistence
    m_stopWorker = false;
    m_worker = std::thread(&HistoryRepository::workerThreadLoop, this);

    return true;
}

void HistoryRepository::shutdown() {
    if (m_stopWorker) return;

    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_stopWorker = true;
    }
    m_cv.notify_all();

    if (m_worker.joinable()) {
        m_worker.join();
    }
}

void HistoryRepository::queueDbTask(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        if (m_stopWorker) return;
        m_taskQueue.push(std::move(task));
    }
    m_cv.notify_one();
}

void HistoryRepository::workerThreadLoop() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_cv.wait(lock, [this]() {
                return m_stopWorker || !m_taskQueue.empty();
            });

            if (m_stopWorker && m_taskQueue.empty()) {
                break;
            }

            if (!m_taskQueue.empty()) {
                task = std::move(m_taskQueue.front());
                m_taskQueue.pop();
            }
        }

        if (task) {
            try {
                task();
            } catch (...) {
                // Catch any unexpected exceptions to keep the thread alive
            }
        }
    }
}

bool HistoryRepository::addItem(ClipboardItem item) {
    // 1. Check for duplicate in hot cache
    auto existing = m_cache.getByHash(item.contentHash);
    if (existing.has_value()) {
        // Item exists! Update timestamp and move to front in RAM instantly
        item.id = existing->id;
        item.pinned = existing->pinned;
        m_cache.put(item);

        // Queue DB timestamp update asynchronously (non-blocking for UI)
        uint64_t hash = item.contentHash;
        int64_t lastUsed = item.lastUsedAt;
        queueDbTask([this, hash, lastUsed]() {
            int64_t outId = 0;
            m_db->updateItemUsageByHash(hash, lastUsed, outId);
        });

        return false; // Duplicate refreshed
    }

    // 2. New item: add to RAM cache immediately
    m_cache.put(item);

    // 3. Queue DB insertion and pruning asynchronously
    queueDbTask([this, item]() mutable {
        if (m_db->insertItem(item)) {
            m_db->pruneToLimit(m_config.historyLimit * 3);
        }
    });

    return true; // Newly added
}

bool HistoryRepository::setPinned(int64_t id, bool pinned) {
    // Update in-memory cache
    auto items = m_cache.getAll();
    for (auto& it : items) {
        if (it.id == id) {
            it.pinned = pinned;
            m_cache.put(it);
            break;
        }
    }

    // Queue DB persistence
    queueDbTask([this, id, pinned]() {
        m_db->setPinned(id, pinned);
    });

    return true;
}

bool HistoryRepository::deleteItem(int64_t id) {
    m_cache.remove(id);

    queueDbTask([this, id]() {
        m_db->deleteItem(id);
    });

    return true;
}

bool HistoryRepository::clearAll(bool keepPinned) {
    if (!keepPinned) {
        m_cache.clear();
    } else {
        auto items = m_cache.getAll();
        m_cache.clear();
        for (const auto& item : items) {
            if (item.pinned) {
                m_cache.put(item);
            }
        }
    }

    queueDbTask([this, keepPinned]() {
        m_db->clearAll(keepPinned);
    });

    return true;
}

std::vector<ClipboardItem> HistoryRepository::getCachedItems() const {
    return m_cache.getAll();
}

std::vector<ClipboardItem> HistoryRepository::search(const std::string& query) {
    if (query.empty()) {
        return getCachedItems();
    }

    // Search in-memory cache first for lightning fast response
    std::string queryLower = query;
    std::transform(queryLower.begin(), queryLower.end(), queryLower.begin(), 
                   [](unsigned char c) -> char { return static_cast<char>(std::tolower(c)); });

    std::vector<ClipboardItem> results;
    auto cached = m_cache.getAll();
    for (const auto& item : cached) {
        if (item.matchesQuery(queryLower)) {
            results.push_back(item);
        }
    }

    // If cache results are small, query SQLite for older items
    if (results.size() < 10 && m_db && m_db->isOpen()) {
        auto dbResults = m_db->searchItems(query, 20);
        for (const auto& dbItem : dbResults) {
            bool alreadyInResults = false;
            for (const auto& r : results) {
                if (r.contentHash == dbItem.contentHash) {
                    alreadyInResults = true;
                    break;
                }
            }
            if (!alreadyInResults) {
                results.push_back(dbItem);
            }
        }
    }

    return results;
}

void HistoryRepository::updateCapacity(size_t newCapacity) {
    m_config.historyLimit = static_cast<int>(newCapacity);
    m_cache.setCapacity(newCapacity);
}

} // namespace cliphub
