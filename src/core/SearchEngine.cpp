#include "core/SearchEngine.h"
#include <algorithm>
#include <cctype>

namespace cliphub {

std::string SearchEngine::toLowerUtf8(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : static_cast<char>(c);
    });
    return result;
}

bool SearchEngine::containsSubstring(const std::string& haystackLower, const std::string& needleLower) {
    if (needleLower.empty()) return true;
    return haystackLower.find(needleLower) != std::string::npos;
}

std::vector<ClipboardItem> SearchEngine::filter(
    const std::vector<ClipboardItem>& items, 
    const std::string& query) {
    
    if (query.empty()) {
        return items;
    }

    std::string queryLower = toLowerUtf8(query);
    std::vector<ClipboardItem> matches;
    matches.reserve(items.size());

    for (const auto& item : items) {
        if (item.matchesQuery(queryLower)) {
            matches.push_back(item);
        }
    }

    return matches;
}

} // namespace cliphub
