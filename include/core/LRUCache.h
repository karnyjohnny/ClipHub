#pragma once

#include "ClipboardItem.h"
#include <list>
#include <unordered_map>
#include <optional>
#include <mutex>

namespace cliphub {

class LRUCache {
public:
    explicit LRUCache(size_t capacity = 20) : m_capacity(capacity) {}

    void setCapacity(size_t newCapacity) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_capacity = newCapacity;
        while (m_list.size() > m_capacity) {
            // Do not evict pinned items if possible
            auto it = m_list.end();
            --it;
            while (it != m_list.begin() && it->pinned) {
                --it;
            }
            if (it->pinned) {
                break; // all items are pinned
            }
            m_map.erase(it->contentHash);
            m_list.erase(it);
        }
    }

    size_t capacity() const {
        return m_capacity;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_list.size();
    }

    // Insert or update item. If duplicate hash exists, bumps to front and updates timestamp.
    // Returns true if new item was added, false if existing item was updated.
    bool put(const ClipboardItem& item) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_map.find(item.contentHash);
        if (it != m_map.end()) {
            // Duplicate detected! Refresh lastUsedAt and move to front
            auto listIt = it->second;
            listIt->lastUsedAt = item.lastUsedAt;
            m_list.splice(m_list.begin(), m_list, listIt);
            return false;
        }

        // Evict LRU if capacity exceeded
        if (m_list.size() >= m_capacity) {
            auto evictIt = m_list.end();
            --evictIt;
            while (evictIt != m_list.begin() && evictIt->pinned) {
                --evictIt;
            }
            if (!evictIt->pinned) {
                m_map.erase(evictIt->contentHash);
                m_list.erase(evictIt);
            }
        }

        m_list.push_front(item);
        m_map[item.contentHash] = m_list.begin();
        return true;
    }

    std::optional<ClipboardItem> getByHash(uint64_t hash) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_map.find(hash);
        if (it == m_map.end()) {
            return std::nullopt;
        }
        // Move to front
        m_list.splice(m_list.begin(), m_list, it->second);
        return *it->second;
    }

    std::vector<ClipboardItem> getAll() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return std::vector<ClipboardItem>(m_list.begin(), m_list.end());
    }

    bool remove(int64_t id) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto it = m_list.begin(); it != m_list.end(); ++it) {
            if (it->id == id) {
                m_map.erase(it->contentHash);
                m_list.erase(it);
                return true;
            }
        }
        return false;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_list.clear();
        m_map.clear();
    }

private:
    size_t m_capacity;
    std::list<ClipboardItem> m_list;
    std::unordered_map<uint64_t, std::list<ClipboardItem>::iterator> m_map;
    mutable std::mutex m_mutex;
};

} // namespace cliphub
