#pragma once

#include "core/ClipboardItem.h"
#include "core/HistoryRepository.h"
#include "core/Config.h"
#include "gui/Theme.h"
#include <memory>
#include <vector>
#include <string>

#ifdef _WIN32
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#endif

namespace cliphub {

class MainWindow {
public:
    MainWindow(std::shared_ptr<HistoryRepository> repo, Config& config);
    ~MainWindow();

    bool create();
    void show();
    void hide();
    bool isVisible() const { return m_visible; }

    void refreshList();

private:
#ifdef _WIN32
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void render();
    void createDeviceResources();
    void discardDeviceResources();

private:
    HWND m_hwnd = nullptr;
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

    IDWriteTextFormat* m_fontTitle = nullptr;
    IDWriteTextFormat* m_fontRegular = nullptr;
    IDWriteTextFormat* m_fontBold = nullptr;
    IDWriteTextFormat* m_fontSmall = nullptr;
#endif

private:
    std::shared_ptr<HistoryRepository> m_repo;
    Config& m_config;
    bool m_visible = false;
    std::string m_searchQuery;
    std::vector<ClipboardItem> m_items;
    int m_selectedIndex = -1;
    int m_hoverIndex = -1;
    float m_dpiScale = 1.0f;
};

} // namespace cliphub
