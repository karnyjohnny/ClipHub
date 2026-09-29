#include "platform/Win32Clipboard.h"
#include "platform/Logger.h"

#ifdef _WIN32
#include <windows.h>
#include <wincodec.h>
#include <vector>

namespace cliphub {

Win32Clipboard::Win32Clipboard() = default;

Win32Clipboard::~Win32Clipboard() {
    stopListening();
}

bool Win32Clipboard::startListening(void* hwndHandle, ClipboardCallback onNewItem) {
    m_hwnd = static_cast<HWND>(hwndHandle);
    m_callback = std::move(onNewItem);

    if (!m_hwnd) {
        LOG_ERROR("Win32Clipboard: Invalid window handle passed to startListening");
        return false;
    }

    if (!AddClipboardFormatListener(m_hwnd)) {
        DWORD err = GetLastError();
        LOG_ERROR("Win32Clipboard: AddClipboardFormatListener failed with error: " + std::to_string(err));
        return false;
    }

    m_isListening = true;
    LOG_INFO("Win32Clipboard: Successfully started listening via AddClipboardFormatListener");
    return true;
}

void Win32Clipboard::stopListening() {
    if (m_isListening && m_hwnd) {
        RemoveClipboardFormatListener(m_hwnd);
        m_isListening = false;
        LOG_INFO("Win32Clipboard: Stopped clipboard listener");
    }
}

void Win32Clipboard::onClipboardUpdate() {
    if (m_paused) {
        LOG_DEBUG("Win32Clipboard: Update ignored because monitoring is paused");
        return;
    }

    if (m_ignoreNextUpdate) {
        m_ignoreNextUpdate = false;
        LOG_DEBUG("Win32Clipboard: Ignored self-injected clipboard update");
        return;
    }

    // Try text first
    if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
        std::string text = getClipboardTextInternal();
        if (!text.empty()) {
            ClipboardItem item = ClipboardItem::createText(text);
            LOG_INFO("Captured text clipboard item (" + std::to_string(text.length()) + " chars)");
            if (m_callback) {
                m_callback(std::move(item));
            }
            return;
        }
    }

    // Try image next
    if (IsClipboardFormatAvailable(CF_DIB) || IsClipboardFormatAvailable(CF_DIBV5)) {
        std::vector<uint8_t> thumb;
        uint32_t w = 0, h = 0;
        if (getClipboardImageInternal(thumb, w, h)) {
            ClipboardItem item = ClipboardItem::createImage(thumb, w, h);
            LOG_INFO("Captured image clipboard item (" + std::to_string(w) + "x" + std::to_string(h) + ")");
            if (m_callback) {
                m_callback(std::move(item));
            }
            return;
        }
    }
}

std::string Win32Clipboard::getClipboardTextInternal() {
    if (!OpenClipboard(m_hwnd)) {
        return "";
    }

    std::string result;
    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData) {
        const wchar_t* wstr = static_cast<const wchar_t*>(GlobalLock(hData));
        if (wstr) {
            int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
            if (len > 1) {
                result.resize(len - 1);
                WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &result[0], len, nullptr, nullptr);
            }
            GlobalUnlock(hData);
        }
    }

    CloseClipboard();
    return result;
}

bool Win32Clipboard::getClipboardImageInternal(std::vector<uint8_t>& outThumb, uint32_t& outW, uint32_t& outH) {
    if (!OpenClipboard(m_hwnd)) {
        return false;
    }

    HANDLE hData = GetClipboardData(CF_DIB);
    if (!hData) {
        CloseClipboard();
        return false;
    }

    const void* pDib = GlobalLock(hData);
    if (!pDib) {
        CloseClipboard();
        return false;
    }

    const auto* pHeader = static_cast<const BITMAPINFOHEADER*>(pDib);
    outW = static_cast<uint32_t>(pHeader->biWidth);
    outH = static_cast<uint32_t>(std::abs(pHeader->biHeight));

    // Store a lightweight thumbnail representation (sample of header/bytes)
    size_t dataSize = GlobalSize(hData);
    size_t copySize = std::min<size_t>(dataSize, 8192); // Keep thumbnail byte buffer small
    outThumb.assign(static_cast<const uint8_t*>(pDib), static_cast<const uint8_t*>(pDib) + copySize);

    GlobalUnlock(hData);
    CloseClipboard();
    return true;
}

bool Win32Clipboard::setClipboardText(const std::string& utf8Text) {
    markNextUpdateAsSelfPaste();

    if (!OpenClipboard(m_hwnd)) {
        return false;
    }
    EmptyClipboard();

    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8Text.c_str(), -1, nullptr, 0);
    if (wlen <= 0) {
        CloseClipboard();
        return false;
    }

    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, wlen * sizeof(wchar_t));
    if (!hGlobal) {
        CloseClipboard();
        return false;
    }

    wchar_t* pBuf = static_cast<wchar_t*>(GlobalLock(hGlobal));
    MultiByteToWideChar(CP_UTF8, 0, utf8Text.c_str(), -1, pBuf, wlen);
    GlobalUnlock(hGlobal);

    SetClipboardData(CF_UNICODETEXT, hGlobal);
    CloseClipboard();
    return true;
}

bool Win32Clipboard::setClipboardItem(const ClipboardItem& item) {
    if (item.type == ItemType::Text) {
        return setClipboardText(item.textContent);
    }
    // Image restore
    if (!item.thumbnailData.empty()) {
        markNextUpdateAsSelfPaste();
        if (!OpenClipboard(m_hwnd)) return false;
        EmptyClipboard();

        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, item.thumbnailData.size());
        if (hMem) {
            void* p = GlobalLock(hMem);
            memcpy(p, item.thumbnailData.data(), item.thumbnailData.size());
            GlobalUnlock(hMem);
            SetClipboardData(CF_DIB, hMem);
        }
        CloseClipboard();
        return true;
    }
    return false;
}

} // namespace cliphub

#else

// Non-Windows stub for headless Linux unit tests
namespace cliphub {

Win32Clipboard::Win32Clipboard() = default;
Win32Clipboard::~Win32Clipboard() = default;
bool Win32Clipboard::startListening(void* hwnd, ClipboardCallback callback) {
    m_callback = std::move(callback);
    m_isListening = true;
    return true;
}
void Win32Clipboard::stopListening() { m_isListening = false; }
void Win32Clipboard::onClipboardUpdate() {}
std::string Win32Clipboard::getClipboardTextInternal() { return ""; }
bool Win32Clipboard::getClipboardImageInternal(std::vector<uint8_t>&, uint32_t&, uint32_t&) { return false; }
bool Win32Clipboard::setClipboardText(const std::string&) { return true; }
bool Win32Clipboard::setClipboardItem(const ClipboardItem&) { return true; }

} // namespace cliphub

#endif
