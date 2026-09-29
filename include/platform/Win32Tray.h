#pragma once

#include <cstdint>
#include <functional>

namespace cliphub {

enum class TrayAction {
    OpenMain,
    ShowPicker,
    TogglePause,
    ClearHistory,
    Exit
};

using TrayCallback = std::function<void(TrayAction)>;

class Win32Tray {
public:
    Win32Tray();
    ~Win32Tray();

    bool init(void* hwndHandle, uint32_t messageId, TrayCallback callback);
    void remove();
    void showContextMenu();
    void updateTooltip(const wchar_t* tooltip);
    void setMonitoringPaused(bool paused);

private:
#ifdef _WIN32
    void* m_hwnd = nullptr;
    uint32_t m_msgId = 0;
#else
    void* m_hwnd = nullptr;
    uint32_t m_msgId = 0;
#endif
    TrayCallback m_callback;
    bool m_installed = false;
    bool m_monitoringPaused = false;
};

} // namespace cliphub
