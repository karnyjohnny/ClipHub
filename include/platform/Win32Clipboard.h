#pragma once

#include "core/ClipboardItem.h"
#include <functional>
#include <memory>
#include <string>

#ifdef _WIN32
#include "platform/PlatformDefs.h"
#endif

namespace cliphub {

using ClipboardCallback = std::function<void(ClipboardItem)>;

class Win32Clipboard {
public:
    Win32Clipboard();
    ~Win32Clipboard();

    // Attach to message-only window for WM_CLIPBOARDUPDATE
    bool startListening(void* hwndHandle, ClipboardCallback onNewItem);
    void stopListening();

    // Called when WM_CLIPBOARDUPDATE is received
    void onClipboardUpdate();

    // Place content onto clipboard
    bool setClipboardText(const std::string& utf8Text);
    bool setClipboardItem(const ClipboardItem& item);

    // Pause/Resume monitoring
    void setPaused(bool paused) { m_paused = paused; }
    bool isPaused() const { return m_paused; }

    // Flag to avoid re-capturing our own paste injection
    void markNextUpdateAsSelfPaste() { m_ignoreNextUpdate = true; }

private:
    std::string getClipboardTextInternal();
    bool getClipboardImageInternal(std::vector<uint8_t>& outThumb, uint32_t& outW, uint32_t& outH);

private:
#ifdef _WIN32
    HWND m_hwnd = nullptr;
#else
    void* m_hwnd = nullptr;
#endif
    ClipboardCallback m_callback;
    bool m_isListening = false;
    bool m_paused = false;
    bool m_ignoreNextUpdate = false;
};

} // namespace cliphub
