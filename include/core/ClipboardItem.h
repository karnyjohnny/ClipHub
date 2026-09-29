#pragma once

#include "Types.h"
#include <string>
#include <vector>
#include <cstdint>

namespace cliphub {

class ClipboardItem {
public:
    int64_t id = 0;
    ItemType type = ItemType::Text;
    int64_t createdAt = 0;     // Timestamp in milliseconds
    int64_t lastUsedAt = 0;    // Timestamp in milliseconds
    std::string textContent;   // Full text content (UTF-8)
    std::string previewText;   // Single-line or clipped preview for low-latency UI rendering
    size_t charCount = 0;
    
    // Image specific attributes
    ImageMetadata imageMeta;
    std::vector<uint8_t> thumbnailData; // Compressed or downscaled thumbnail bytes
    std::string storagePath;            // Disk path for heavy original image payload
    
    bool pinned = false;
    uint64_t contentHash = 0;

public:
    ClipboardItem() = default;

    // Factory methods
    static ClipboardItem createText(const std::string& text, int64_t timestamp = 0);
    static ClipboardItem createImage(const std::vector<uint8_t>& thumbnail, 
                                     uint32_t width, 
                                     uint32_t height, 
                                     const std::string& diskPath = "", 
                                     int64_t timestamp = 0);

    // Helpers
    void updatePreview();
    void computeContentHash();
    bool matchesQuery(const std::string& queryLower) const;
    
    // 64-bit FNV-1a hash calculation
    static uint64_t calculateFnv1a(const void* data, size_t length);
};

} // namespace cliphub
