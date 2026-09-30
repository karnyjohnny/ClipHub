#include "core/ClipboardItem.h"
#include <chrono>
#include <algorithm>
#include <cctype>

namespace cliphub {

static int64_t getCurrentTimeMs() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

uint64_t ClipboardItem::calculateFnv1a(const void* data, size_t length) {
    const uint64_t FNV_OFFSET = 14695981039346656037ULL;
    const uint64_t FNV_PRIME = 1099511628211ULL;
    uint64_t hash = FNV_OFFSET;
    const auto* bytes = static_cast<const uint8_t*>(data);
    for (size_t i = 0; i < length; ++i) {
        hash ^= bytes[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

ClipboardItem ClipboardItem::createText(const std::string& text, int64_t timestamp) {
    ClipboardItem item;
    item.type = ItemType::Text;
    item.createdAt = (timestamp > 0) ? timestamp : getCurrentTimeMs();
    item.lastUsedAt = item.createdAt;
    item.textContent = text;
    item.charCount = text.length();
    item.updatePreview();
    item.computeContentHash();
    return item;
}

ClipboardItem ClipboardItem::createImage(const std::vector<uint8_t>& thumbnail, 
                                         uint32_t width, 
                                         uint32_t height, 
                                         const std::string& diskPath, 
                                         int64_t timestamp) {
    ClipboardItem item;
    item.type = ItemType::Image;
    item.createdAt = (timestamp > 0) ? timestamp : getCurrentTimeMs();
    item.lastUsedAt = item.createdAt;
    item.imageMeta.width = width;
    item.imageMeta.height = height;
    item.imageMeta.channels = 4;
    item.imageMeta.byteSize = thumbnail.size();
    item.thumbnailData = thumbnail;
    item.storagePath = diskPath;
    item.previewText = "[Image " + std::to_string(width) + "x" + std::to_string(height) + "]";
    item.computeContentHash();
    return item;
}

void ClipboardItem::updatePreview() {
    if (type == ItemType::Image) {
        previewText = "[Image " + std::to_string(imageMeta.width) + "x" + std::to_string(imageMeta.height) + "]";
        return;
    }

    if (textContent.empty()) {
        previewText = "(Empty)";
        return;
    }

    // Clean whitespace and newlines for fast single-line display
    std::string preview;
    preview.reserve(std::min<size_t>(textContent.length(), 140));

    bool prevSpace = false;
    for (char c : textContent) {
        if (c == '\r' || c == '\n' || c == '\t' || c == ' ') {
            if (!prevSpace && !preview.empty()) {
                preview.push_back(' ');
                prevSpace = true;
            }
        } else {
            preview.push_back(c);
            prevSpace = false;
        }

        if (preview.length() >= 90) {
            // Ensure we never split a multi-byte UTF-8 sequence
            while (!preview.empty() && (static_cast<unsigned char>(preview.back()) & 0xC0) == 0x80) {
                preview.pop_back();
            }
            if (!preview.empty() && (static_cast<unsigned char>(preview.back()) & 0x80) != 0) {
                preview.pop_back();
            }
            preview.append("...");
            break;
        }
    }

    previewText = preview.empty() ? "(Whitespace)" : preview;
}

void ClipboardItem::computeContentHash() {
    if (type == ItemType::Text) {
        contentHash = calculateFnv1a(textContent.data(), textContent.size());
    } else {
        if (!thumbnailData.empty()) {
            contentHash = calculateFnv1a(thumbnailData.data(), thumbnailData.size());
        } else {
            std::string key = std::to_string(imageMeta.width) + "x" + std::to_string(imageMeta.height) + storagePath;
            contentHash = calculateFnv1a(key.data(), key.size());
        }
    }
}

bool ClipboardItem::matchesQuery(const std::string& queryLower) const {
    if (queryLower.empty()) {
        return true;
    }

    if (type == ItemType::Text) {
        // Simple case-insensitive search
        auto it = std::search(
            textContent.begin(), textContent.end(),
            queryLower.begin(), queryLower.end(),
            [](char ch1, char ch2) {
                return std::tolower(static_cast<unsigned char>(ch1)) == 
                       std::tolower(static_cast<unsigned char>(ch2));
            }
        );
        return it != textContent.end();
    } else {
        // Image matches if preview text or dimensions match query
        std::string dimStr = std::to_string(imageMeta.width) + "x" + std::to_string(imageMeta.height);
        return dimStr.find(queryLower) != std::string::npos || 
               queryLower == "image" || queryLower == "img" || queryLower == "png";
    }
}

} // namespace cliphub
