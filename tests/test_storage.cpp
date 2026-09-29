#include "TestHarness.h"
#include "storage/Database.h"
#include <cstdio>

using namespace cliphub;

TEST(Storage, OpenAndClose) {
    const char* dbFile = "test_temp_1.db";
    std::remove(dbFile);

    Database db;
    ASSERT_TRUE(db.open(dbFile));
    ASSERT_TRUE(db.isOpen());
    db.close();
    ASSERT_FALSE(db.isOpen());

    std::remove(dbFile);
}

TEST(Storage, InsertAndRetrieve) {
    const char* dbFile = "test_temp_2.db";
    std::remove(dbFile);

    Database db;
    ASSERT_TRUE(db.open(dbFile));

    ClipboardItem item = ClipboardItem::createText("Storage Persistence Item");
    ASSERT_TRUE(db.insertItem(item));
    ASSERT_TRUE(item.id > 0);

    auto recent = db.getRecentItems(10);
    ASSERT_EQ(recent.size(), 1);
    ASSERT_EQ(recent[0].textContent, "Storage Persistence Item");
    ASSERT_EQ(recent[0].id, item.id);

    db.close();
    std::remove(dbFile);
}

TEST(Storage, UpdateUsageByHash) {
    const char* dbFile = "test_temp_3.db";
    std::remove(dbFile);

    Database db;
    db.open(dbFile);

    ClipboardItem item = ClipboardItem::createText("Duplicate Candidate", 1000);
    db.insertItem(item);

    int64_t foundId = 0;
    bool updated = db.updateItemUsageByHash(item.contentHash, 5000, foundId);
    ASSERT_TRUE(updated);
    ASSERT_EQ(foundId, item.id);

    auto recent = db.getRecentItems(1);
    ASSERT_EQ(recent[0].lastUsedAt, 5000);

    db.close();
    std::remove(dbFile);
}

TEST(Storage, DeleteAndClear) {
    const char* dbFile = "test_temp_4.db";
    std::remove(dbFile);

    Database db;
    db.open(dbFile);

    ClipboardItem item1 = ClipboardItem::createText("Item 1");
    ClipboardItem item2 = ClipboardItem::createText("Item 2");
    db.insertItem(item1);
    db.insertItem(item2);

    ASSERT_EQ(db.getRecentItems(10).size(), 2);

    ASSERT_TRUE(db.deleteItem(item1.id));
    auto remaining = db.getRecentItems(10);
    ASSERT_EQ(remaining.size(), 1);
    ASSERT_EQ(remaining[0].id, item2.id);

    db.close();
    std::remove(dbFile);
}

TEST(Storage, PinnedPreservedOnPrune) {
    const char* dbFile = "test_temp_5.db";
    std::remove(dbFile);

    Database db;
    db.open(dbFile);

    ClipboardItem pinItem = ClipboardItem::createText("Pinned Important Item", 100);
    pinItem.pinned = true;
    db.insertItem(pinItem);

    for (int i = 0; i < 10; ++i) {
        ClipboardItem item = ClipboardItem::createText("Temp Item " + std::to_string(i), 200 + i);
        db.insertItem(item);
    }

    // Prune to 3 items
    db.pruneToLimit(3);

    auto list = db.getRecentItems(50);
    bool pinnedFound = false;
    for (const auto& it : list) {
        if (it.textContent == "Pinned Important Item") pinnedFound = true;
    }
    ASSERT_TRUE(pinnedFound);

    db.close();
    std::remove(dbFile);
}
