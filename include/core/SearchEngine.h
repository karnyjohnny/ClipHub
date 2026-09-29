#pragma once

#include "core/ClipboardItem.h"
#include <string>
#include <vector>

namespace cliphub {

class SearchEngine {
public:
    static std::vector<ClipboardItem> filter(
        const std::vector<ClipboardItem>& items, 
        const std::string& query);

    static std::string toLowerUtf8(const std::string& str);
    static bool containsSubstring(const std::string& haystackLower, const std::string& needleLower);
};

} // namespace cliphub
