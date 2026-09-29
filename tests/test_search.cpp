#include "TestHarness.h"
#include "core/SearchEngine.h"

using namespace cliphub;

TEST(Search, SubstringAndCaseInsensitive) {
    std::vector<ClipboardItem> items = {
        ClipboardItem::createText("https://google.com"),
        ClipboardItem::createText("SELECT * FROM users;"),
        ClipboardItem::createText("git commit -m 'Initial commit'"),
        ClipboardItem::createText("Lorem Ipsum Dolor Sit Amet")
    };

    auto matches = SearchEngine::filter(items, "sql");
    ASSERT_EQ(matches.size(), 0);

    matches = SearchEngine::filter(items, "select");
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ(matches[0].textContent, "SELECT * FROM users;");

    matches = SearchEngine::filter(items, "GIT");
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ(matches[0].textContent, "git commit -m 'Initial commit'");
}

TEST(Search, EmptyQueryReturnsAll) {
    std::vector<ClipboardItem> items = {
        ClipboardItem::createText("A"),
        ClipboardItem::createText("B"),
        ClipboardItem::createText("C")
    };

    auto matches = SearchEngine::filter(items, "");
    ASSERT_EQ(matches.size(), 3);
}

TEST(Search, NoMatchReturnsEmpty) {
    std::vector<ClipboardItem> items = {
        ClipboardItem::createText("Apple"),
        ClipboardItem::createText("Banana")
    };

    auto matches = SearchEngine::filter(items, "Cherry");
    ASSERT_EQ(matches.size(), 0);
}
