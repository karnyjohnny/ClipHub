#include "TestHarness.h"
#include "core/LRUCache.h"
#include "core/HistoryRepository.h"
#include "storage/Database.h"
#include <cstdio>
#include <thread>
#include <chrono>

using namespace cliphub;

TEST(History, LRUCapacityLimit) {
    LRUCache cache(3); // capacity 3

    cache.put(ClipboardItem::createText("Item 1"));
    cache.put(ClipboardItem::createText("Item 2"));
    cache.put(ClipboardItem::createText("Item 3"));
    ASSERT_EQ(cache.size(), 3);

    // Adding 4th item should evict Item 1
    cache.put(ClipboardItem::createText("Item 4"));
    ASSERT_EQ(cache.size(), 3);

    auto all = cache.getAll();
    ASSERT_EQ(all[0].textContent, "Item 4");
    ASSERT_EQ(all[1].textContent, "Item 3");
    ASSERT_EQ(all[2].textContent, "Item 2");
}

TEST(History, DuplicateRefreshesTimestampAndPosition) {
    LRUCache cache(5);

    auto itemA = ClipboardItem::createText("Alpha", 100);
    auto itemB = ClipboardItem::createText("Beta", 200);
    auto itemC = ClipboardItem::createText("Gamma", 300);

    cache.put(itemA);
    cache.put(itemB);
    cache.put(itemC);

    // Duplicate of Alpha arrives at timestamp 400
    auto itemADup = ClipboardItem::createText("Alpha", 400);
    bool addedNew = cache.put(itemADup);
    ASSERT_FALSE(addedNew); // Duplicate refreshed

    auto all = cache.getAll();
    ASSERT_EQ(all.size(), 3);
    // Alpha must now be at the front!
    ASSERT_EQ(all[0].textContent, "Alpha");
    ASSERT_EQ(all[0].lastUsedAt, 400);
    ASSERT_EQ(all[1].textContent, "Gamma");
    ASSERT_EQ(all[2].textContent, "Beta");
}

TEST(History, RepositoryEndToEnd) {
    const char* dbFile = "test_temp_repo.db";
    std::remove(dbFile);

    Config cfg;
    cfg.historyLimit = 10;
    cfg.dbPath = dbFile;

    auto db = std::make_shared<Database>();
    db->open(dbFile);

    HistoryRepository repo(db, cfg);
    ASSERT_TRUE(repo.initialize());

    repo.addItem(ClipboardItem::createText("First entry"));
    repo.addItem(ClipboardItem::createText("Second entry"));

    auto cached = repo.getCachedItems();
    ASSERT_EQ(cached.size(), 2);
    ASSERT_EQ(cached[0].textContent, "Second entry");

    // Allow worker thread to flush
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    repo.shutdown();
    db->close();
    std::remove(dbFile);
}
