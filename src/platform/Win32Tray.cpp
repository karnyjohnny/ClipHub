#include "platform/Win32Tray.h"
#include "platform/Logger.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>

namespace cliphub {

enum TrayMenuCmd {
    CMD_TRAY_OPEN = 2001,
    CMD_TRAY_PICKER,
    CMD_TRAY_PAUSE,
    CMD_TRAY_CLEAR,
    CMD_TRAY_EXIT
};

Win32Tray::Win32Tray() = default;

Win32Tray::~Win32Tray() {
    remove();
}

bool Win32Tray::init(void* hwndHandle, uint32_t messageId, TrayCallback callback) {
    remove();

    m_hwnd = hwndHandle;
    m_msgId = messageId;
    m_callback = std::move(callback);

    HWND hwnd = static_cast<HWND>(m_hwnd);

    HINSTANCE hInst = GetModuleHandleW(nullptr);
    HICON hAppIcon = LoadIconW(hInst, MAKEINTRESOURCEW(101));
    if (!hAppIcon) {
        hAppIcon = LoadIconW(nullptr, (LPCWSTR)IDI_APPLICATION);
    }

    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = m_msgId;
    nid.hIcon = hAppIcon;
    wcscpy_s(nid.szTip, L"ClipHub - Ultra-Lightweight Clipboard Manager");

    if (!Shell_NotifyIconW(NIM_ADD, &nid)) {
        LOG_ERROR("Win32Tray: Failed to install notification tray icon");
        return false;
    }

    m_installed = true;
    LOG_INFO("Win32Tray: Tray icon installed successfully");
    return true;
}

void Win32Tray::remove() {
    if (m_installed && m_hwnd) {
        NOTIFYICONDATAW nid = {};
        nid.cbSize = sizeof(NOTIFYICONDATAW);
        nid.hWnd = static_cast<HWND>(m_hwnd);
        nid.uID = 1;
        Shell_NotifyIconW(NIM_DELETE, &nid);
        m_installed = false;
        LOG_INFO("Win32Tray: Tray icon removed");
    }
}

void Win32Tray::updateTooltip(const wchar_t* tooltip) {
    if (!m_installed || !m_hwnd) return;

    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = static_cast<HWND>(m_hwnd);
    nid.uID = 1;
    nid.uFlags = NIF_TIP;
    wcscpy_s(nid.szTip, tooltip);
    Shell_NotifyIconW(NIM_MODIFY, &nid);
}

void Win32Tray::setMonitoringPaused(bool paused) {
    m_monitoringPaused = paused;
    if (paused) {
        updateTooltip(L"ClipHub - [PAUSED]");
    } else {
        updateTooltip(L"ClipHub - Ultra-Lightweight Clipboard Manager");
    }
}

void Win32Tray::showContextMenu() {
    if (!m_hwnd) return;
    HWND hwnd = static_cast<HWND>(m_hwnd);

    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    InsertMenuW(hMenu, 0, MF_BYPOSITION | MF_STRING, CMD_TRAY_OPEN, L"Open ClipHub");
    InsertMenuW(hMenu, 1, MF_BYPOSITION | MF_STRING, CMD_TRAY_PICKER, L"Clipboard History (Alt+V)");
    InsertMenuW(hMenu, 2, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
    
    UINT pauseFlags = MF_BYPOSITION | MF_STRING | (m_monitoringPaused ? MF_CHECKED : MF_UNCHECKED);
    InsertMenuW(hMenu, 3, pauseFlags, CMD_TRAY_PAUSE, m_monitoringPaused ? L"Resume Monitoring" : L"Pause Monitoring");
    
    InsertMenuW(hMenu, 4, MF_BYPOSITION | MF_STRING, CMD_TRAY_CLEAR, L"Clear History");
    InsertMenuW(hMenu, 5, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
    InsertMenuW(hMenu, 6, MF_BYPOSITION | MF_STRING, CMD_TRAY_EXIT, L"Exit");

    // Make Open the bold default item
    SetMenuDefaultItem(hMenu, CMD_TRAY_OPEN, FALSE);

    SetForegroundWindow(hwnd);
    int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, nullptr);
    DestroyMenu(hMenu);

    if (!m_callback) return;

    switch (cmd) {
        case CMD_TRAY_OPEN:   m_callback(TrayAction::OpenMain); break;
        case CMD_TRAY_PICKER: m_callback(TrayAction::ShowPicker); break;
        case CMD_TRAY_PAUSE:  m_callback(TrayAction::TogglePause); break;
        case CMD_TRAY_CLEAR:  m_callback(TrayAction::ClearHistory); break;
        case CMD_TRAY_EXIT:   m_callback(TrayAction::Exit); break;
    }
}

} // namespace cliphub

#else

namespace cliphub {
Win32Tray::Win32Tray() = default;
Win32Tray::~Win32Tray() = default;
bool Win32Tray::init(void*, uint32_t, TrayCallback cb) { m_callback = std::move(cb); return true; }
void Win32Tray::remove() {}
void Win32Tray::showContextMenu() {}
void Win32Tray::updateTooltip(const wchar_t*) {}
void Win32Tray::setMonitoringPaused(bool) {}
} // namespace cliphub

#endif
