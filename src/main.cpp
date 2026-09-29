#include "gui/App.h"
#include "platform/Logger.h"

#ifdef _WIN32
#include <windows.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    (void)hInstance; (void)hPrevInstance; (void)pCmdLine; (void)nCmdShow;

    // Single instance mutex guard so multiple instances are not started
    HANDLE hMutex = CreateMutexW(nullptr, TRUE, L"ClipHub_SingleInstance_Mutex_E5500");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // Bring existing instance popup or main window to front
        HWND existingWnd = FindWindowW(L"ClipHubMainWindowClass", nullptr);
        if (existingWnd) {
            ShowWindow(existingWnd, SW_SHOW);
            SetForegroundWindow(existingWnd);
        }
        if (hMutex) CloseHandle(hMutex);
        return 0;
    }

    cliphub::Logger::instance().init("ClipHub.log", cliphub::LogLevel::Info);

    cliphub::App app;
    if (!app.initialize()) {
        LOG_ERROR("ClipHub failed to initialize properly");
        if (hMutex) CloseHandle(hMutex);
        return 1;
    }

    int exitCode = app.run();

    if (hMutex) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }
    return exitCode;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)lpCmdLine;
    return wWinMain(hInstance, hPrevInstance, GetCommandLineW(), nCmdShow);
}

#else

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    cliphub::Logger::instance().init("ClipHub.log", cliphub::LogLevel::Info);
    cliphub::App app;
    if (!app.initialize()) {
        return 1;
    }
    return app.run();
}

#endif
