#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace cliphub {

enum class ItemType : uint8_t {
    Text = 0,
    Image = 1
};

struct ImageMetadata {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t channels = 4; // RGBA
    size_t byteSize = 0;
};

// Memory-efficient representation of clipboard payload
struct ClipboardData {
    ItemType type = ItemType::Text;
    std::string textContent;              // UTF-8 string for text
    std::vector<uint8_t> binaryContent;   // Raw PNG or encoded thumbnail for image
    ImageMetadata imageMeta;
};

} // namespace cliphub
