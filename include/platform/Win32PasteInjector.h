#pragma once

#include <cstdint>

namespace cliphub {

class Win32PasteInjector {
public:
    // Pastes into target window handle: restores foreground, releases Alt modifier, then sends Ctrl+V
    static bool pasteIntoWindow(void* targetHwnd);

    // Get handle of currently active foreground window before opening popup
    static void* getForegroundWindow();
};

} // namespace cliphub
