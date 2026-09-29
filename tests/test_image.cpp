#include "TestHarness.h"
#include "core/ClipboardItem.h"

using namespace cliphub;

TEST(Image, CreateImageMetadata) {
    std::vector<uint8_t> dummyThumb = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
    ClipboardItem item = ClipboardItem::createImage(dummyThumb, 1920, 1080);

    ASSERT_EQ(item.type, ItemType::Image);
    ASSERT_EQ(item.imageMeta.width, 1920);
    ASSERT_EQ(item.imageMeta.height, 1080);
    ASSERT_EQ(item.previewText, "[Image 1920x1080]");
    ASSERT_EQ(item.thumbnailData.size(), 8);
    ASSERT_NE(item.contentHash, 0);
}

TEST(Image, QueryMatching) {
    std::vector<uint8_t> dummyThumb = { 0x01, 0x02, 0x03 };
    ClipboardItem item = ClipboardItem::createImage(dummyThumb, 800, 600);

    ASSERT_TRUE(item.matchesQuery("800x600"));
    ASSERT_TRUE(item.matchesQuery("image"));
    ASSERT_TRUE(item.matchesQuery("img"));
    ASSERT_FALSE(item.matchesQuery("1920x1080"));
}
