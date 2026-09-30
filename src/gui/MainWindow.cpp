#include "gui/MainWindow.h"
#include "gui/D2DCommon.h"
#include "platform/Logger.h"

#ifdef _WIN32
#include <windows.h>
#include <algorithm>

namespace cliphub {

static const wchar_t* MAIN_CLASS_NAME = L"ClipHubMainWindowClass";

MainWindow::MainWindow(std::shared_ptr<HistoryRepository> repo, Config& config)
    : m_repo(std::move(repo)), m_config(config) {
}

MainWindow::~MainWindow() {
    discardDeviceResources();
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

bool MainWindow::create() {
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWindow::WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hbrBackground = nullptr;
    wc.lpszClassName = MAIN_CLASS_NAME;

    RegisterClassExW(&wc);

    int w = 540;
    int h = 640;
    int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

    m_hwnd = CreateWindowExW(
        0,
        MAIN_CLASS_NAME,
        L"ClipHub",
        WS_OVERLAPPEDWINDOW,
        x, y, w, h,
        nullptr,
        nullptr,
        hInstance,
        this
    );

    if (!m_hwnd) {
        LOG_ERROR("MainWindow: Failed to create window");
        return false;
    }

    m_dpiScale = D2DContext::getDpiScaleForHwnd(m_hwnd);
    refreshList();
    return true;
}

void MainWindow::createDeviceResources() {
    if (m_renderTarget) return;

    D2DContext& ctx = D2DContext::instance();
    if (!ctx.d2dFactory() || !m_hwnd) return;

    RECT rc;
    GetClientRect(m_hwnd, &rc);
    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

    D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
    rtProps.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
    D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps = D2D1::HwndRenderTargetProperties(m_hwnd, size);

    HRESULT hr = ctx.d2dFactory()->CreateHwndRenderTarget(rtProps, hwndProps, &m_renderTarget);
    if (FAILED(hr) || !m_renderTarget) {
        rtProps.type = D2D1_RENDER_TARGET_TYPE_SOFTWARE;
        ctx.d2dFactory()->CreateHwndRenderTarget(rtProps, hwndProps, &m_renderTarget);
    }

    if (!m_renderTarget) return;

    const Theme& th = Theme::dark();
    m_renderTarget->CreateSolidColorBrush(D2DContext::toD2DColor(th.background), &m_brushBg);
    m_renderTarget->CreateSolidColorBrush(D2DContext::toD2DColor(th.panel), &m_brushPanel);
    m_renderTarget->CreateSolidColorBrush(D2DContext::toD2DColor(th.secondary), &m_brushSecondary);
    m_renderTarget->CreateSolidColorBrush(D2DContext::toD2DColor(th.border), &m_brushBorder);
    m_renderTarget->CreateSolidColorBrush(D2DContext::toD2DColor(th.text), &m_brushText);
    m_renderTarget->CreateSolidColorBrush(D2DContext::toD2DColor(th.textSecondary), &m_brushTextSecondary);
    m_renderTarget->CreateSolidColorBrush(D2DContext::toD2DColor(th.accent), &m_brushAccent);
    m_renderTarget->CreateSolidColorBrush(D2DContext::toD2DColor(th.itemSelected), &m_brushSelected);
    m_renderTarget->CreateSolidColorBrush(D2DContext::toD2DColor(th.itemHover), &m_brushHover);
    m_renderTarget->CreateSolidColorBrush(D2DContext::toD2DColor(th.pinnedBadge), &m_brushPinned);

    if (ctx.dwriteFactory()) {
        ctx.dwriteFactory()->CreateTextFormat(
            L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL, 17.0f * m_dpiScale, L"en-us", &m_fontTitle
        );
        ctx.dwriteFactory()->CreateTextFormat(
            L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL, 13.0f * m_dpiScale, L"en-us", &m_fontRegular
        );
        ctx.dwriteFactory()->CreateTextFormat(
            L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL, 13.0f * m_dpiScale, L"en-us", &m_fontBold
        );
        ctx.dwriteFactory()->CreateTextFormat(
            L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL, 11.0f * m_dpiScale, L"en-us", &m_fontSmall
        );

        if (m_fontRegular) {
            m_fontRegular->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
            IDWriteInlineObject* trimmingSign = nullptr;
            if (SUCCEEDED(ctx.dwriteFactory()->CreateEllipsisTrimmingSign(m_fontRegular, &trimmingSign))) {
                DWRITE_TRIMMING trimming = { DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0 };
                m_fontRegular->SetTrimming(&trimming, trimmingSign);
                trimmingSign->Release();
            }
        }
        if (m_fontBold) {
            m_fontBold->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        }
        if (m_fontSmall) {
            m_fontSmall->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
            IDWriteInlineObject* trimmingSign = nullptr;
            if (SUCCEEDED(ctx.dwriteFactory()->CreateEllipsisTrimmingSign(m_fontSmall, &trimmingSign))) {
                DWRITE_TRIMMING trimming = { DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0 };
                m_fontSmall->SetTrimming(&trimming, trimmingSign);
                trimmingSign->Release();
            }
        }
    }
}

void MainWindow::discardDeviceResources() {
    if (m_fontSmall) { m_fontSmall->Release(); m_fontSmall = nullptr; }
    if (m_fontBold) { m_fontBold->Release(); m_fontBold = nullptr; }
    if (m_fontRegular) { m_fontRegular->Release(); m_fontRegular = nullptr; }
    if (m_fontTitle) { m_fontTitle->Release(); m_fontTitle = nullptr; }

    if (m_brushPinned) { m_brushPinned->Release(); m_brushPinned = nullptr; }
    if (m_brushHover) { m_brushHover->Release(); m_brushHover = nullptr; }
    if (m_brushSelected) { m_brushSelected->Release(); m_brushSelected = nullptr; }
    if (m_brushAccent) { m_brushAccent->Release(); m_brushAccent = nullptr; }
    if (m_brushTextSecondary) { m_brushTextSecondary->Release(); m_brushTextSecondary = nullptr; }
    if (m_brushText) { m_brushText->Release(); m_brushText = nullptr; }
    if (m_brushBorder) { m_brushBorder->Release(); m_brushBorder = nullptr; }
    if (m_brushSecondary) { m_brushSecondary->Release(); m_brushSecondary = nullptr; }
    if (m_brushPanel) { m_brushPanel->Release(); m_brushPanel = nullptr; }
    if (m_brushBg) { m_brushBg->Release(); m_brushBg = nullptr; }

    if (m_renderTarget) { m_renderTarget->Release(); m_renderTarget = nullptr; }
}

void MainWindow::refreshList() {
    if (m_repo) {
        m_items = m_repo->search(m_searchQuery);
        if (m_hwnd) InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void MainWindow::show() {
    if (m_hwnd) {
        refreshList();
        ShowWindow(m_hwnd, SW_SHOW);
        SetForegroundWindow(m_hwnd);
        m_visible = true;
    }
}

void MainWindow::hide() {
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_HIDE);
        m_visible = false;
    }
}

void MainWindow::render() {
    createDeviceResources();
    if (!m_renderTarget) return;

    m_renderTarget->BeginDraw();
    m_renderTarget->Clear(D2DContext::toD2DColor(Theme::dark().background));

    RECT clientRc;
    GetClientRect(m_hwnd, &clientRc);
    float w = static_cast<float>(clientRc.right);
    float h = static_cast<float>(clientRc.bottom);

    // 1. Top Title Bar Header
    float headerH = 48.0f * m_dpiScale;
    D2D1_RECT_F headerRect = D2D1::RectF(0, 0, w, headerH);
    m_renderTarget->FillRectangle(headerRect, m_brushPanel);
    m_renderTarget->DrawLine(D2D1::Point2F(0, headerH), D2D1::Point2F(w, headerH), m_brushBorder, 1.0f);

    std::wstring title = L"ClipHub";
    if (m_fontTitle && m_brushText) {
        m_renderTarget->DrawText(title.c_str(), static_cast<UINT32>(title.length()),
                                 m_fontTitle, D2D1::RectF(16.0f, 12.0f, 200.0f, headerH), m_brushText);
    }

    std::wstring sub = L"Windows 7 Ultra-Lightweight Edition";
    if (m_fontSmall && m_brushTextSecondary) {
        m_renderTarget->DrawText(sub.c_str(), static_cast<UINT32>(sub.length()),
                                 m_fontSmall, D2D1::RectF(w - 240.0f, 16.0f, w - 16.0f, headerH), m_brushTextSecondary);
    }

    // 2. Search Input Field
    float searchY = headerH + 12.0f;
    float searchH = 38.0f * m_dpiScale;
    D2D1_RECT_F searchBg = D2D1::RectF(16.0f, searchY, w - 16.0f, searchY + searchH);
    m_renderTarget->FillRectangle(searchBg, m_brushPanel);
    m_renderTarget->DrawRectangle(searchBg, m_brushBorder, 1.0f);

    std::wstring searchDisp = m_searchQuery.empty()
        ? L"🔍  Search clipboard history..."
        : L"🔍  " + std::wstring(m_searchQuery.begin(), m_searchQuery.end());
    if (m_fontRegular && m_brushTextSecondary) {
        m_renderTarget->DrawText(searchDisp.c_str(), static_cast<UINT32>(searchDisp.length()),
                                 m_fontRegular, D2D1::RectF(26.0f, searchY + 9.0f, w - 26.0f, searchY + searchH),
                                 m_searchQuery.empty() ? m_brushTextSecondary : m_brushText);
    }

    // 3. Section Title
    float secY = searchY + searchH + 14.0f;
    std::wstring secTitle = L"RECENT CLIPBOARD HISTORY";
    if (m_fontSmall && m_brushTextSecondary) {
        m_renderTarget->DrawText(secTitle.c_str(), static_cast<UINT32>(secTitle.length()),
                                 m_fontSmall, D2D1::RectF(18.0f, secY, w - 18.0f, secY + 18.0f), m_brushTextSecondary);
    }

    // 4. List Items
    float listY = secY + 22.0f;
    float itemH = 50.0f * m_dpiScale;
    float footerH = 32.0f * m_dpiScale;
    int maxItems = static_cast<int>((h - listY - footerH) / itemH);

    for (int i = 0; i < std::min<int>(static_cast<int>(m_items.size()), maxItems); ++i) {
        const auto& item = m_items[i];
        float itTop = listY + i * itemH;
        float itBottom = itTop + itemH - 3.0f;
        D2D1_RECT_F itemRect = D2D1::RectF(16.0f, itTop, w - 16.0f, itBottom);

        m_renderTarget->PushAxisAlignedClip(itemRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

        if (i == m_selectedIndex) {
            m_renderTarget->FillRectangle(itemRect, m_brushSelected);
            m_renderTarget->FillRectangle(D2D1::RectF(16.0f, itTop, 19.0f, itBottom), m_brushAccent);
        } else if (i == m_hoverIndex) {
            m_renderTarget->FillRectangle(itemRect, m_brushHover);
        }

        m_renderTarget->DrawRectangle(itemRect, m_brushBorder, 1.0f);

        // Icon badge
        std::wstring badge = (item.type == ItemType::Text) ? L"T" : L"IMG";
        D2D1_RECT_F bRect = D2D1::RectF(26.0f, itTop + 6.0f, 62.0f, itBottom - 6.0f);
        m_renderTarget->FillRectangle(bRect, m_brushSecondary);
        if (m_fontBold) {
            m_renderTarget->DrawText(badge.c_str(), static_cast<UINT32>(badge.length()),
                                     m_fontBold, D2D1::RectF(32.0f, itTop + 10.0f, 62.0f, itBottom),
                                     (item.type == ItemType::Text) ? m_brushAccent : m_brushPinned,
                                     D2D1_DRAW_TEXT_OPTIONS_CLIP);
        }

        // Preview text
        std::wstring prevW = D2DContext::utf8ToWide(item.previewText);
        D2D1_RECT_F textBounds = D2D1::RectF(72.0f, itTop + 6.0f, w - 80.0f, itTop + 25.0f);
        if (m_fontRegular && m_brushText) {
            m_renderTarget->DrawText(prevW.c_str(), static_cast<UINT32>(prevW.length()),
                                     m_fontRegular, textBounds, m_brushText,
                                     D2D1_DRAW_TEXT_OPTIONS_CLIP);
        }

        // Subtitle metadata
        std::wstring metaW = (item.type == ItemType::Text)
            ? std::to_wstring(item.charCount) + L" characters"
            : std::to_wstring(item.imageMeta.width) + L"x" + std::to_wstring(item.imageMeta.height) + L" Image";
        if (item.pinned) metaW += L"  ★ Pinned";

        D2D1_RECT_F metaBounds = D2D1::RectF(72.0f, itTop + 26.0f, w - 80.0f, itBottom - 2.0f);
        if (m_fontSmall && m_brushTextSecondary) {
            m_renderTarget->DrawText(metaW.c_str(), static_cast<UINT32>(metaW.length()),
                                     m_fontSmall, metaBounds, 
                                     item.pinned ? m_brushPinned : m_brushTextSecondary,
                                     D2D1_DRAW_TEXT_OPTIONS_CLIP);
        }

        m_renderTarget->PopAxisAlignedClip();
    }

    // 5. Footer Status Bar
    float footerY = h - footerH;
    m_renderTarget->FillRectangle(D2D1::RectF(0, footerY, w, h), m_brushPanel);
    m_renderTarget->DrawLine(D2D1::Point2F(0, footerY), D2D1::Point2F(w, footerY), m_brushBorder, 1.0f);

    std::wstring statusStr = std::to_wstring(m_items.size()) + L" items stored  •  Press Alt+V anywhere";
    if (m_fontSmall && m_brushTextSecondary) {
        m_renderTarget->DrawText(statusStr.c_str(), static_cast<UINT32>(statusStr.length()),
                                 m_fontSmall, D2D1::RectF(16.0f, footerY + 6.0f, 320.0f, h), m_brushTextSecondary);
    }

    std::wstring hintRight = L"RAM: <12MB  |  Core 2 Duo Ready";
    if (m_fontSmall && m_brushTextSecondary) {
        m_renderTarget->DrawText(hintRight.c_str(), static_cast<UINT32>(hintRight.length()),
                                 m_fontSmall, D2D1::RectF(w - 240.0f, footerY + 6.0f, w - 16.0f, h), m_brushTextSecondary);
    }

    HRESULT hr = m_renderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        discardDeviceResources();
    }
}

LRESULT CALLBACK MainWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    MainWindow* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        self = reinterpret_cast<MainWindow*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (self) {
        return self->handleMessage(hwnd, msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT MainWindow::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            render();
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_SIZE: {
            if (m_renderTarget) {
                RECT rc;
                GetClientRect(hwnd, &rc);
                m_renderTarget->Resize(D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top));
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }
        case WM_CLOSE: {
            // Minimize to system tray instead of terminating
            hide();
            return 0;
        }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace cliphub

#else

namespace cliphub {
MainWindow::MainWindow(std::shared_ptr<HistoryRepository> repo, Config& config)
    : m_repo(std::move(repo)), m_config(config) {}
MainWindow::~MainWindow() = default;
bool MainWindow::create() { return true; }
void MainWindow::show() { m_visible = true; }
void MainWindow::hide() { m_visible = false; }
void MainWindow::refreshList() {}
} // namespace cliphub

#endif
