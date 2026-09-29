#pragma once

#include "core/Config.h"
#include "core/HistoryRepository.h"
#include "storage/Database.h"
#include "platform/Win32Clipboard.h"
#include "platform/Win32Hotkey.h"
#include "platform/Win32PasteInjector.h"
#include "platform/Win32Tray.h"
#include "gui/PopupPicker.h"
#include "gui/MainWindow.h"

#include <memory>

namespace cliphub {

class App {
public:
    App();
    ~App();

    bool initialize();
    int run();
    void shutdown();

private:
#ifdef _WIN32
    static LRESULT CALLBACK MsgWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool createMessageOnlyWindow();
#endif

    void onClipboardItemCaptured(ClipboardItem item);
    void onHotkeyPressed();
    void onPasteRequested(const ClipboardItem& item, void* targetWindow);
    void onTrayAction(TrayAction action);

private:
    Config m_config;
    std::shared_ptr<Database> m_db;
    std::shared_ptr<HistoryRepository> m_repo;

    Win32Clipboard m_clipboard;
    Win32Hotkey m_hotkey;
    Win32Tray m_tray;

    std::unique_ptr<PopupPicker> m_picker;
    std::unique_ptr<MainWindow> m_mainWindow;

#ifdef _WIN32
    HWND m_msgHwnd = nullptr;
#else
    void* m_msgHwnd = nullptr;
#endif
    bool m_running = false;
};

} // namespace cliphub
