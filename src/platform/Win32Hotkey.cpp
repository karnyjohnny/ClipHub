#include "platform/Win32Hotkey.h"
#include "platform/Logger.h"

#ifdef _WIN32
#include <windows.h>

namespace cliphub {

Win32Hotkey::Win32Hotkey() = default;

Win32Hotkey::~Win32Hotkey() {
    unregisterHotkey();
}

bool Win32Hotkey::registerHotkey(void* hwndHandle, int hotkeyId, uint32_t modifiers, uint32_t vkKey, HotkeyCallback callback) {
    unregisterHotkey();

    m_hwnd = hwndHandle;
    m_hotkeyId = hotkeyId;
    m_callback = std::move(callback);

    HWND hwnd = static_cast<HWND>(m_hwnd);
    if (!RegisterHotKey(hwnd, m_hotkeyId, modifiers, vkKey)) {
        DWORD err = GetLastError();
        LOG_ERROR("Win32Hotkey: RegisterHotKey failed with error " + std::to_string(err));
        return false;
    }

    m_registered = true;
    LOG_INFO("Win32Hotkey: Global hotkey registered successfully (ID: " + std::to_string(m_hotkeyId) + ")");
    return true;
}

void Win32Hotkey::unregisterHotkey() {
    if (m_registered && m_hwnd) {
        UnregisterHotKey(static_cast<HWND>(m_hwnd), m_hotkeyId);
        m_registered = false;
        LOG_INFO("Win32Hotkey: Unregistered global hotkey");
    }
}

void Win32Hotkey::onHotkeyPressed(int hotkeyId) {
    if (m_registered && hotkeyId == m_hotkeyId && m_callback) {
        LOG_DEBUG("Win32Hotkey: Alt+V hotkey triggered");
        m_callback();
    }
}

} // namespace cliphub

#else

namespace cliphub {
Win32Hotkey::Win32Hotkey() = default;
Win32Hotkey::~Win32Hotkey() = default;
bool Win32Hotkey::registerHotkey(void*, int id, uint32_t, uint32_t, HotkeyCallback cb) {
    m_hotkeyId = id;
    m_callback = std::move(cb);
    m_registered = true;
    return true;
}
void Win32Hotkey::unregisterHotkey() { m_registered = false; }
void Win32Hotkey::onHotkeyPressed(int id) {
    if (m_registered && id == m_hotkeyId && m_callback) m_callback();
}
} // namespace cliphub

#endif
