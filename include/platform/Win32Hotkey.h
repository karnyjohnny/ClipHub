#pragma once

#include <cstdint>
#include <functional>

namespace cliphub {

using HotkeyCallback = std::function<void()>;

class Win32Hotkey {
public:
    Win32Hotkey();
    ~Win32Hotkey();

    bool registerHotkey(void* hwndHandle, int hotkeyId, uint32_t modifiers, uint32_t vkKey, HotkeyCallback callback);
    void unregisterHotkey();

    void onHotkeyPressed(int hotkeyId);

private:
#ifdef _WIN32
    void* m_hwnd = nullptr;
#else
    void* m_hwnd = nullptr;
#endif
    int m_hotkeyId = 1001;
    HotkeyCallback m_callback;
    bool m_registered = false;
};

} // namespace cliphub
