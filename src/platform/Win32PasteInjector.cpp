#include "platform/Win32PasteInjector.h"
#include "platform/Logger.h"

#ifdef _WIN32
#include <windows.h>
#include <thread>
#include <chrono>

namespace cliphub {

void* Win32PasteInjector::getForegroundWindow() {
    return static_cast<void*>(GetForegroundWindow());
}

bool Win32PasteInjector::pasteIntoWindow(void* targetHwnd) {
    HWND hwnd = static_cast<HWND>(targetHwnd);
    if (!hwnd || !IsWindow(hwnd)) {
        LOG_WARN("Win32PasteInjector: Target window handle is invalid");
        return false;
    }

    // 1. Restore focus to the target window
    SetForegroundWindow(hwnd);
    SetActiveWindow(hwnd);

    // 2. Ensure Alt key is released if user held it from Alt+V hotkey
    if (GetAsyncKeyState(VK_MENU) & 0x8000) {
        INPUT altUp{};
        altUp.type = INPUT_KEYBOARD;
        altUp.ki.wVk = VK_MENU;
        altUp.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &altUp, sizeof(INPUT));
    }

    // 3. Short wait for focus to settle
    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    // 4. Inject Ctrl + V
    INPUT inputs[4] = {};

    // Ctrl Down
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;

    // V Down
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'V';

    // V Up
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'V';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    // Ctrl Up
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    UINT sent = SendInput(4, inputs, sizeof(INPUT));
    if (sent != 4) {
        LOG_ERROR("Win32PasteInjector: SendInput failed to inject all 4 key events");
        return false;
    }

    LOG_INFO("Win32PasteInjector: Successfully injected Ctrl+V into target window");
    return true;
}

} // namespace cliphub

#else

namespace cliphub {
void* Win32PasteInjector::getForegroundWindow() { return nullptr; }
bool Win32PasteInjector::pasteIntoWindow(void*) { return true; }
} // namespace cliphub

#endif
