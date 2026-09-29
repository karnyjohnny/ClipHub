#include "gui/App.h"
#include "gui/D2DCommon.h"
#include "platform/Logger.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace cliphub {

#ifdef _WIN32
static const wchar_t* MSG_WINDOW_CLASS = L"ClipHubMessageWindowClass";
static const UINT WM_TRAYNOTIFY = WM_APP + 101;
static const int HOTKEY_ALTV_ID = 1001;

LRESULT CALLBACK App::MsgWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    App* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        self = reinterpret_cast<App*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<App*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (self) {
        return self->handleMessage(hwnd, msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
#endif

App::App() = default;

App::~App() {
    shutdown();
}

bool App::initialize() {
    LOG_INFO("=== Starting ClipHub (Windows 7 E5500 Ultra-Lightweight Edition) ===");

    // 1. Load config
    m_config.loadFromFile("cliphub.ini");

    // 2. Open SQLite Database
    m_db = std::make_shared<Database>();
    if (!m_db->open(m_config.dbPath)) {
        LOG_ERROR("Failed to open SQLite database: " + m_db->getLastError());
        return false;
    }
    LOG_INFO("SQLite database initialized successfully in WAL mode");

    // 3. Initialize History Repository & Warm Cache
    m_repo = std::make_shared<HistoryRepository>(m_db, m_config);
    if (!m_repo->initialize()) {
        LOG_ERROR("Failed to initialize HistoryRepository");
        return false;
    }
    LOG_INFO("HistoryRepository initialized with " + std::to_string(m_repo->getCachedItems().size()) + " cached items");

#ifdef _WIN32
    // 4. Initialize Direct2D Context
    if (!D2DContext::instance().initialize()) {
        LOG_ERROR("Failed to initialize Direct2D / DirectWrite context");
        return false;
    }

    // 5. Create Message-Only Window for OS Notifications
    if (!createMessageOnlyWindow()) {
        LOG_ERROR("Failed to create message-only window");
        return false;
    }

    // 6. Hook clipboard events (AddClipboardFormatListener)
    if (!m_clipboard.startListening(m_msgHwnd, [this](ClipboardItem item) {
        onClipboardItemCaptured(std::move(item));
    })) {
        LOG_ERROR("Failed to register clipboard format listener");
        return false;
    }

    // 7. Register Global Hotkey (ALT+V)
    if (!m_hotkey.registerHotkey(m_msgHwnd, HOTKEY_ALTV_ID, m_config.hotkeyModifier, m_config.hotkeyKey, [this]() {
        onHotkeyPressed();
    })) {
        LOG_WARN("Could not register default ALT+V hotkey (might be occupied by another app)");
    }

    // 8. Create Popup Picker and Main Window
    m_picker = std::make_unique<PopupPicker>(m_repo, [this](const ClipboardItem& item, void* target) {
        onPasteRequested(item, target);
    });
    m_picker->create(m_msgHwnd);

    m_mainWindow = std::make_unique<MainWindow>(m_repo, m_config);
    m_mainWindow->create();

    // 9. Install System Tray Icon
    m_tray.init(m_msgHwnd, WM_TRAYNOTIFY, [this](TrayAction action) {
        onTrayAction(action);
    });
#endif

    m_running = true;
    LOG_INFO("ClipHub initialization complete. Ready and idling at 0% CPU.");
    return true;
}

#ifdef _WIN32
bool App::createMessageOnlyWindow() {
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = App::MsgWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = MSG_WINDOW_CLASS;

    RegisterClassExW(&wc);

    m_msgHwnd = CreateWindowExW(
        0,
        MSG_WINDOW_CLASS,
        L"ClipHubMsgWindow",
        0,
        0, 0, 0, 0,
        HWND_MESSAGE,
        nullptr,
        hInstance,
        this
    );

    return m_msgHwnd != nullptr;
}

LRESULT App::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLIPBOARDUPDATE: {
            m_clipboard.onClipboardUpdate();
            return 0;
        }
        case WM_HOTKEY: {
            m_hotkey.onHotkeyPressed(static_cast<int>(wParam));
            return 0;
        }
        case WM_TRAYNOTIFY: {
            if (lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU) {
                m_tray.showContextMenu();
            } else if (lParam == WM_LBUTTONDBLCLK) {
                if (m_mainWindow) m_mainWindow->show();
            }
            return 0;
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
#endif

void App::onClipboardItemCaptured(ClipboardItem item) {
    if (m_config.monitoringPaused) return;

    bool isNew = m_repo->addItem(std::move(item));
    if (m_mainWindow && m_mainWindow->isVisible()) {
        m_mainWindow->refreshList();
    }
    LOG_DEBUG("Clipboard item handled (isNew: " + std::to_string(isNew) + ")");
}

void App::onHotkeyPressed() {
#ifdef _WIN32
    void* activeHwnd = Win32PasteInjector::getForegroundWindow();
    if (m_picker) {
        m_picker->showNearCursor(activeHwnd);
    }
#endif
}

void App::onPasteRequested(const ClipboardItem& item, void* targetWindow) {
#ifdef _WIN32
    // 1. Set item onto Windows clipboard
    m_clipboard.setClipboardItem(item);

    // 2. Inject paste into target application window
    if (targetWindow) {
        Win32PasteInjector::pasteIntoWindow(targetWindow);
    }

    // 3. Bump item usage timestamp in history
    m_repo->addItem(item);
#endif
}

void App::onTrayAction(TrayAction action) {
    switch (action) {
        case TrayAction::OpenMain:
            if (m_mainWindow) m_mainWindow->show();
            break;
        case TrayAction::ShowPicker:
            onHotkeyPressed();
            break;
        case TrayAction::TogglePause:
            m_config.monitoringPaused = !m_config.monitoringPaused;
            m_clipboard.setPaused(m_config.monitoringPaused);
            m_tray.setMonitoringPaused(m_config.monitoringPaused);
            LOG_INFO("Monitoring " + std::string(m_config.monitoringPaused ? "paused" : "resumed"));
            break;
        case TrayAction::ClearHistory:
            if (m_repo) m_repo->clearAll(true);
            if (m_mainWindow) m_mainWindow->refreshList();
            LOG_INFO("History cleared (pinned items preserved)");
            break;
        case TrayAction::Exit:
            shutdown();
#ifdef _WIN32
            PostQuitMessage(0);
#endif
            break;
    }
}

int App::run() {
#ifdef _WIN32
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
#else
    return 0;
#endif
}

void App::shutdown() {
    if (!m_running) return;
    m_running = false;

    LOG_INFO("Shutting down ClipHub...");

    m_clipboard.stopListening();
    m_hotkey.unregisterHotkey();
    m_tray.remove();

    if (m_repo) {
        m_repo->shutdown();
    }
    if (m_db) {
        m_db->close();
    }

    m_config.saveToFile("cliphub.ini");

#ifdef _WIN32
    D2DContext::instance().shutdown();
    if (m_msgHwnd) {
        DestroyWindow(m_msgHwnd);
        m_msgHwnd = nullptr;
    }
#endif

    LOG_INFO("ClipHub successfully terminated.");
}

} // namespace cliphub
