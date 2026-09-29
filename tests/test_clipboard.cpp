#include "TestHarness.h"
#include "core/ClipboardItem.h"

using namespace cliphub;

TEST(Clipboard, CreateText) {
    ClipboardItem item = ClipboardItem::createText("https://github.com/cliphub/cliphub");
    ASSERT_EQ(item.type, ItemType::Text);
    ASSERT_EQ(item.charCount, 34);
    ASSERT_EQ(item.previewText, "https://github.com/cliphub/cliphub");
    ASSERT_NE(item.contentHash, 0);
}

TEST(Clipboard, EmptyText) {
    ClipboardItem item = ClipboardItem::createText("");
    ASSERT_EQ(item.charCount, 0);
    ASSERT_EQ(item.previewText, "(Empty)");
}

TEST(Clipboard, UnicodeSupport) {
    std::string polish = "Zażółć gęślą jaźń - test polskich znaków schowka";
    ClipboardItem item = ClipboardItem::createText(polish);
    ASSERT_EQ(item.textContent, polish);
    ASSERT_TRUE(item.matchesQuery("zażółć"));
    ASSERT_TRUE(item.matchesQuery("polskich"));
}

TEST(Clipboard, LongTextTruncation) {
    std::string longText(1000, 'A');
    ClipboardItem item = ClipboardItem::createText(longText);
    ASSERT_EQ(item.charCount, 1000);
    ASSERT_TRUE(item.previewText.length() <= 125);
    ASSERT_TRUE(item.previewText.find("...") != std::string::npos);
}

TEST(Clipboard, DuplicateHashCollisionResistance) {
    ClipboardItem item1 = ClipboardItem::createText("First string snippet");
    ClipboardItem item2 = ClipboardItem::createText("First string snippet");
    ClipboardItem item3 = ClipboardItem::createText("Second string snippet");

    ASSERT_EQ(item1.contentHash, item2.contentHash);
    ASSERT_NE(item1.contentHash, item3.contentHash);
}
