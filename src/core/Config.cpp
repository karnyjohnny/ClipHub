#include "core/Config.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace cliphub {

static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

bool Config::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;

        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;

        std::string key = trim(line.substr(0, eqPos));
        std::string val = trim(line.substr(eqPos + 1));

        if (key == "historyLimit") {
            try { historyLimit = std::clamp(std::stoi(val), 5, 500); } catch (...) {}
        } else if (key == "startWithWindows") {
            startWithWindows = (val == "1" || val == "true" || val == "TRUE");
        } else if (key == "monitoringPaused") {
            monitoringPaused = (val == "1" || val == "true" || val == "TRUE");
        } else if (key == "darkTheme") {
            darkTheme = (val == "1" || val == "true" || val == "TRUE");
        } else if (key == "dbPath") {
            dbPath = val;
        } else if (key == "hotkeyModifier") {
            try { hotkeyModifier = static_cast<uint32_t>(std::stoul(val, nullptr, 0)); } catch (...) {}
        } else if (key == "hotkeyKey") {
            try { hotkeyKey = static_cast<uint32_t>(std::stoul(val, nullptr, 0)); } catch (...) {}
        }
    }

    return true;
}

bool Config::saveToFile(const std::string& filePath) const {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    file << "; ClipHub Configuration File\n";
    file << "; Optimized for Windows 7 & Dell Latitude E5500\n\n";
    file << "historyLimit=" << historyLimit << "\n";
    file << "startWithWindows=" << (startWithWindows ? "1" : "0") << "\n";
    file << "monitoringPaused=" << (monitoringPaused ? "1" : "0") << "\n";
    file << "darkTheme=" << (darkTheme ? "1" : "0") << "\n";
    file << "dbPath=" << dbPath << "\n";
    file << "hotkeyModifier=" << hotkeyModifier << "\n";
    file << "hotkeyKey=" << hotkeyKey << "\n";

    return true;
}

} // namespace cliphub
