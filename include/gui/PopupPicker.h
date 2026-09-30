#pragma once

#include "core/ClipboardItem.h"
#include "core/HistoryRepository.h"
#include "gui/Theme.h"
#include <vector>
#include <string>
#include <memory>
#include <functional>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#endif

namespace cliphub {

using PasteCallback = std::function<void(const ClipboardItem&, void* targetWindow)>;

class PopupPicker {
public:
    PopupPicker(std::shared_ptr<HistoryRepository> repo, PasteCallback pasteCb);
    ~PopupPicker();

    bool create(void* parentHwnd = nullptr);
    void showNearCursor(void* targetActiveHwnd);
    void hide();
    bool isVisible() const { return m_visible; }

private:
#ifdef _WIN32
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void render();
    void updateFilteredList();
    void onKeyNavigation(WPARAM key);
    void onConfirmSelection();

    void createDeviceResources();
    void discardDeviceResources();

private:
    HWND m_hwnd = nullptr;
    HWND m_targetHwnd = nullptr; // Target window that will receive paste
    ID2D1HwndRenderTarget* m_renderTarget = nullptr;
    ID2D1SolidColorBrush* m_brushBg = nullptr;
    ID2D1SolidColorBrush* m_brushPanel = nullptr;
    ID2D1SolidColorBrush* m_brushSecondary = nullptr;
    ID2D1SolidColorBrush* m_brushBorder = nullptr;
    ID2D1SolidColorBrush* m_brushText = nullptr;
    ID2D1SolidColorBrush* m_brushTextSecondary = nullptr;
    ID2D1SolidColorBrush* m_brushAccent = nullptr;
    ID2D1SolidColorBrush* m_brushSelected = nullptr;
    ID2D1SolidColorBrush* m_brushHover = nullptr;
    ID2D1SolidColorBrush* m_brushPinned = nullptr;

    IDWriteTextFormat* m_fontRegular = nullptr;
    IDWriteTextFormat* m_fontBold = nullptr;
    IDWriteTextFormat* m_fontSmall = nullptr;
    IDWriteTextFormat* m_fontMono = nullptr;
#endif

private:
    std::shared_ptr<HistoryRepository> m_repo;
    PasteCallback m_pasteCallback;
    bool m_visible = false;
    std::string m_searchQuery;
    std::vector<ClipboardItem> m_items;
    int m_selectedIndex = 0;
    int m_hoverIndex = -1;
    float m_dpiScale = 1.0f;
};

} // namespace cliphub
