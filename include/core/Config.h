#pragma once

#include <string>
#include <cstdint>

namespace cliphub {

struct Config {
    int historyLimit = 20;              // Default 20 entries
    bool startWithWindows = false;      // Auto-start on boot
    bool monitoringPaused = false;      // Pause clipboard capture
    bool darkTheme = true;              // Dark minimal modern theme
    size_t maxImageMemoryBytes = 4 * 1024 * 1024; // 4MB RAM cache limit for images
    std::string dbPath = "cliphub.db";  // SQLite DB filename
    
    // Global hotkey: ALT + V
    uint32_t hotkeyModifier = 0x0001;   // MOD_ALT on Win32
    uint32_t hotkeyKey = 0x56;          // 'V' virtual key code

    // Persistence
    bool loadFromFile(const std::string& filePath = "cliphub.ini");
    bool saveToFile(const std::string& filePath = "cliphub.ini") const;
};

} // namespace cliphub
