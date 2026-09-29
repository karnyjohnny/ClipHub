#pragma once

#include <cstdint>

namespace cliphub {

struct ColorRGBA {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;

    float rF() const { return r / 255.0f; }
    float gF() const { return g / 255.0f; }
    float bF() const { return b / 255.0f; }
    float aF() const { return a / 255.0f; }
};

struct Theme {
    ColorRGBA background      = { 0x11, 0x11, 0x11, 255 }; // #111111
    ColorRGBA panel           = { 0x18, 0x18, 0x18, 255 }; // #181818
    ColorRGBA secondary       = { 0x20, 0x20, 0x20, 255 }; // #202020
    ColorRGBA border          = { 0x2A, 0x2A, 0x2A, 255 }; // #2A2A2A
    ColorRGBA borderHover     = { 0x3D, 0x3D, 0x3D, 255 }; // #3D3D3D
    ColorRGBA text            = { 0xEA, 0xEA, 0xEA, 255 }; // #EAEAEA
    ColorRGBA textSecondary   = { 0x90, 0x90, 0x90, 255 }; // #909090
    ColorRGBA textMuted       = { 0x5A, 0x5A, 0x5A, 255 }; // #5A5A5A
    ColorRGBA accent          = { 0x5C, 0x8D, 0xFF, 255 }; // #5C8DFF
    ColorRGBA accentHover     = { 0x76, 0xA2, 0xFF, 255 }; // #76A2FF
    ColorRGBA itemHover       = { 0x22, 0x25, 0x2C, 255 }; // #22252C
    ColorRGBA itemSelected    = { 0x1E, 0x28, 0x3D, 255 }; // #1E283D
    ColorRGBA pinnedBadge     = { 0xE5, 0xA1, 0x24, 255 }; // #E5A124

    static const Theme& dark();
};

} // namespace cliphub
