#include "gui/PopupPicker.h"
#include "gui/D2DCommon.h"
#include "platform/Logger.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <algorithm>

namespace cliphub {

static const wchar_t* POPUP_CLASS_NAME = L"ClipHubPopupPickerClass";

PopupPicker::PopupPicker(std::shared_ptr<HistoryRepository> repo, PasteCallback pasteCb)
    : m_repo(std::move(repo)), m_pasteCallback(std::move(pasteCb)) {
}

PopupPicker::~PopupPicker() {
    discardDeviceResources();
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

bool PopupPicker::create(void* parentHwnd) {
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW;
    wc.lpfnWndProc = PopupPicker::WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(101));
    wc.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCEW(101));
    if (!wc.hIcon) wc.hIcon = LoadIconW(nullptr, (LPCWSTR)IDI_APPLICATION);
    if (!wc.hIconSm) wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr; // Handled by Direct2D
    wc.lpszClassName = POPUP_CLASS_NAME;

    RegisterClassExW(&wc);

    // Borderless popup tool window
    m_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        POPUP_CLASS_NAME,
        L"ClipHub Picker",
        WS_POPUP | WS_BORDER,
        100, 100, 440, 360,
        static_cast<HWND>(parentHwnd),
        nullptr,
        hInstance,
        this
    );

    if (!m_hwnd) {
        DWORD err = GetLastError();
        LOG_ERROR("PopupPicker: CreateWindowEx failed with error " + std::to_string(err));
        return false;
    }

    m_dpiScale = D2DContext::getDpiScaleForHwnd(m_hwnd);
    LOG_INFO("PopupPicker: Window created successfully");
    return true;
}

void PopupPicker::createDeviceResources() {
    if (m_renderTarget) return;

    D2DContext& ctx = D2DContext::instance();
    if (!ctx.d2dFactory() || !m_hwnd) return;

    RECT rc;
    GetClientRect(m_hwnd, &rc);
    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

    D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
    // Default to hardware rendering, Direct2D automatically falls back if needed
    rtProps.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
    rtProps.usage = D2D1_RENDER_TARGET_USAGE_NONE;

    D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps = D2D1::HwndRenderTargetProperties(m_hwnd, size);

    HRESULT hr = ctx.d2dFactory()->CreateHwndRenderTarget(rtProps, hwndProps, &m_renderTarget);
    if (FAILED(hr) || !m_renderTarget) {
        // Fallback to software WARP rasterizer if low-end GPU lacks Direct2D hardware features
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

    // DirectWrite Typography
    if (ctx.dwriteFactory()) {
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
        ctx.dwriteFactory()->CreateTextFormat(
            L"Consolas", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL, 12.0f * m_dpiScale, L"en-us", &m_fontMono
        );

        // Strict single-line display: prevent text wrapping and enable ellipsis trimming
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
        if (m_fontMono) {
            m_fontMono->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        }
    }
}

void PopupPicker::discardDeviceResources() {
    if (m_fontMono) { m_fontMono->Release(); m_fontMono = nullptr; }
    if (m_fontSmall) { m_fontSmall->Release(); m_fontSmall = nullptr; }
    if (m_fontBold) { m_fontBold->Release(); m_fontBold = nullptr; }
    if (m_fontRegular) { m_fontRegular->Release(); m_fontRegular = nullptr; }

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

void PopupPicker::updateFilteredList() {
    if (!m_repo) return;
    m_items = m_repo->search(m_searchQuery);
    if (m_selectedIndex >= static_cast<int>(m_items.size())) {
        m_selectedIndex = (std::max)(0, static_cast<int>(m_items.size()) - 1);
    }
}

void PopupPicker::showNearCursor(void* targetActiveHwnd) {
    if (!m_hwnd) return;

    m_targetHwnd = static_cast<HWND>(targetActiveHwnd);
    m_searchQuery.clear();
    m_selectedIndex = 0;
    m_hoverIndex = -1;
    updateFilteredList();

    // Position near mouse cursor, clamped to work area
    POINT pt;
    GetCursorPos(&pt);

    HMONITOR hMon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(MONITORINFO) };
    GetMonitorInfo(hMon, &mi);

    int width = static_cast<int>(420 * m_dpiScale);
    int height = static_cast<int>(360 * m_dpiScale);

    int x = pt.x - 20;
    int y = pt.y + 10;

    // Clamp to screen bounds
    if (x + width > mi.rcWork.right) x = mi.rcWork.right - width - 10;
    if (y + height > mi.rcWork.bottom) y = pt.y - height - 10;
    if (x < mi.rcWork.left) x = mi.rcWork.left + 10;
    if (y < mi.rcWork.top) y = mi.rcWork.top + 10;

    SetWindowPos(m_hwnd, HWND_TOPMOST, x, y, width, height, SWP_SHOWWINDOW);
    SetForegroundWindow(m_hwnd);
    SetActiveWindow(m_hwnd);
    SetFocus(m_hwnd);

    m_visible = true;
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void PopupPicker::hide() {
    if (m_hwnd && m_visible) {
        ShowWindow(m_hwnd, SW_HIDE);
        m_visible = false;
    }
}

void PopupPicker::onConfirmSelection() {
    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_items.size())) {
        ClipboardItem item = m_items[m_selectedIndex];
        HWND target = m_targetHwnd;
        hide();
        if (m_pasteCallback) {
            m_pasteCallback(item, target);
        }
    } else {
        hide();
    }
}

void PopupPicker::render() {
    createDeviceResources();
    if (!m_renderTarget) return;

    m_renderTarget->BeginDraw();
    m_renderTarget->Clear(D2DContext::toD2DColor(Theme::dark().background));

    RECT clientRc;
    GetClientRect(m_hwnd, &clientRc);
    float w = static_cast<float>(clientRc.right);
    float h = static_cast<float>(clientRc.bottom);

    // 1. Draw outer border
    D2D1_RECT_F borderRect = D2D1::RectF(0.5f, 0.5f, w - 0.5f, h - 0.5f);
    m_renderTarget->DrawRectangle(borderRect, m_brushBorder, 1.0f);

    // 2. Search Box Header (height: ~42px)
    float searchH = 42.0f * m_dpiScale;
    D2D1_RECT_F searchBg = D2D1::RectF(6.0f, 6.0f, w - 6.0f, searchH - 2.0f);
    m_renderTarget->FillRectangle(searchBg, m_brushPanel);
    m_renderTarget->DrawRectangle(searchBg, m_brushBorder, 1.0f);

    // Search text / placeholder
    std::wstring searchDisp = m_searchQuery.empty() 
        ? L"🔍  Type to search history..." 
        : L"🔍  " + D2DContext::utf8ToWide(m_searchQuery);

    D2D1_RECT_F textRect = D2D1::RectF(14.0f, 12.0f, w - 16.0f, searchH);
    IDWriteTextFormat* sFont = m_searchQuery.empty() ? m_fontRegular : m_fontBold;
    ID2D1SolidColorBrush* sBrush = m_searchQuery.empty() ? m_brushTextSecondary : m_brushText;
    if (sFont && sBrush) {
        m_renderTarget->DrawText(searchDisp.c_str(), static_cast<UINT32>(searchDisp.length()), 
                                 sFont, textRect, sBrush, D2D1_DRAW_TEXT_OPTIONS_CLIP);
    }

    // 3. Separator below search box
    m_renderTarget->DrawLine(D2D1::Point2F(0, searchH + 4), D2D1::Point2F(w, searchH + 4), m_brushBorder, 1.0f);

    // 4. List Items
    float itemH = 48.0f * m_dpiScale;
    float listY = searchH + 6.0f;
    float footerH = 26.0f * m_dpiScale;
    int maxVisible = static_cast<int>((h - listY - footerH) / itemH);

    if (m_items.empty()) {
        std::wstring emptyMsg = L"No clipboard items found";
        D2D1_RECT_F emptyRect = D2D1::RectF(20.0f, listY + 30.0f, w - 20.0f, listY + 70.0f);
        if (m_fontRegular && m_brushTextSecondary) {
            m_renderTarget->DrawText(emptyMsg.c_str(), static_cast<UINT32>(emptyMsg.length()),
                                     m_fontRegular, emptyRect, m_brushTextSecondary, D2D1_DRAW_TEXT_OPTIONS_CLIP);
        }
    } else {
        for (int i = 0; i < std::min<int>(static_cast<int>(m_items.size()), maxVisible); ++i) {
            const auto& item = m_items[i];
            float itemTop = listY + i * itemH;
            float itemBottom = itemTop + itemH - 2.0f;
            D2D1_RECT_F itemRect = D2D1::RectF(6.0f, itemTop, w - 6.0f, itemBottom);

            // Clip strictly to item bounds to prevent any bleed across items
            m_renderTarget->PushAxisAlignedClip(itemRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

            // Selection / Hover background
            if (i == m_selectedIndex) {
                m_renderTarget->FillRectangle(itemRect, m_brushSelected);
                // Active left accent indicator bar
                m_renderTarget->FillRectangle(D2D1::RectF(itemRect.left, itemTop, itemRect.left + 3.0f, itemBottom), m_brushAccent);
            } else if (i == m_hoverIndex) {
                m_renderTarget->FillRectangle(itemRect, m_brushHover);
            }

            // Type icon: [T] or [IMG]
            std::wstring typeBadge = (item.type == ItemType::Text) ? L"T" : L"IMG";
            float badgeW = (item.type == ItemType::Text) ? 26.0f : 34.0f;
            D2D1_RECT_F badgeRect = D2D1::RectF(itemRect.left + 8.0f, itemTop + 7.0f, itemRect.left + 8.0f + badgeW, itemBottom - 7.0f);
            m_renderTarget->FillRectangle(badgeRect, m_brushSecondary);
            if (m_fontBold) {
                D2D1_RECT_F badgeTextRect = D2D1::RectF(badgeRect.left, itemTop + 8.0f, badgeRect.right, itemBottom - 6.0f);
                m_renderTarget->DrawText(typeBadge.c_str(), static_cast<UINT32>(typeBadge.length()),
                                         m_fontBold, badgeTextRect,
                                         (item.type == ItemType::Text) ? m_brushAccent : m_brushPinned,
                                         D2D1_DRAW_TEXT_OPTIONS_CLIP);
            }

            float textLeft = badgeRect.right + 10.0f;
            float textRight = itemRect.right - 10.0f;

            // Content preview text (UTF-8 converted properly to wide string)
            std::wstring prevW = D2DContext::utf8ToWide(item.previewText);
            D2D1_RECT_F textBounds = D2D1::RectF(textLeft, itemTop + 5.0f, textRight, itemTop + 24.0f);
            if (m_fontRegular && m_brushText) {
                m_renderTarget->DrawText(prevW.c_str(), static_cast<UINT32>(prevW.length()),
                                         m_fontRegular, textBounds, m_brushText,
                                         D2D1_DRAW_TEXT_OPTIONS_CLIP);
            }

            // Metadata info (chars or dimensions)
            std::wstring metaW = (item.type == ItemType::Text)
                ? std::to_wstring(item.charCount) + L" chars"
                : std::to_wstring(item.imageMeta.width) + L"x" + std::to_wstring(item.imageMeta.height);
            if (item.pinned) metaW += L"  ★ pinned";

            D2D1_RECT_F metaBounds = D2D1::RectF(textLeft, itemTop + 25.0f, textRight, itemBottom - 3.0f);
            if (m_fontSmall && m_brushTextSecondary) {
                m_renderTarget->DrawText(metaW.c_str(), static_cast<UINT32>(metaW.length()),
                                         m_fontSmall, metaBounds, 
                                         item.pinned ? m_brushPinned : m_brushTextSecondary,
                                         D2D1_DRAW_TEXT_OPTIONS_CLIP);
            }

            m_renderTarget->PopAxisAlignedClip();
        }
    }

    // 5. Footer Hint Bar
    float footerY = h - footerH;
    m_renderTarget->DrawLine(D2D1::Point2F(0, footerY), D2D1::Point2F(w, footerY), m_brushBorder, 1.0f);
    std::wstring hint = L"↑ ↓ navigate   Enter paste   Esc close   Del remove";
    D2D1_RECT_F hintBounds = D2D1::RectF(12.0f, footerY + 5.0f, w - 12.0f, h);
    if (m_fontSmall && m_brushTextSecondary) {
        m_renderTarget->DrawText(hint.c_str(), static_cast<UINT32>(hint.length()),
                                 m_fontSmall, hintBounds, m_brushTextSecondary);
    }

    HRESULT hr = m_renderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        discardDeviceResources();
    }
}

LRESULT CALLBACK PopupPicker::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    PopupPicker* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        self = reinterpret_cast<PopupPicker*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<PopupPicker*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (self) {
        return self->handleMessage(hwnd, msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT PopupPicker::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            render();
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_ACTIVATE: {
            if (LOWORD(wParam) == WA_INACTIVE) {
                hide();
            }
            return 0;
        }
        case WM_KEYDOWN: {
            if (wParam == VK_ESCAPE) {
                hide();
                return 0;
            } else if (wParam == VK_DOWN) {
                if (!m_items.empty()) {
                    m_selectedIndex = static_cast<int>((m_selectedIndex + 1) % m_items.size());
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
                return 0;
            } else if (wParam == VK_UP) {
                if (!m_items.empty()) {
                    m_selectedIndex = static_cast<int>((m_selectedIndex - 1 + static_cast<int>(m_items.size())) % m_items.size());
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
                return 0;
            } else if (wParam == VK_RETURN) {
                onConfirmSelection();
                return 0;
            } else if (wParam == VK_BACK) {
                if (!m_searchQuery.empty()) {
                    m_searchQuery.pop_back();
                    updateFilteredList();
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
                return 0;
            } else if (wParam == VK_DELETE) {
                if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_items.size())) {
                    m_repo->deleteItem(m_items[m_selectedIndex].id);
                    updateFilteredList();
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
                return 0;
            }
            break;
        }
        case WM_CHAR: {
            if (wParam >= 32 && wParam < 127) {
                m_searchQuery.push_back(static_cast<char>(wParam));
                m_selectedIndex = 0;
                updateFilteredList();
                InvalidateRect(hwnd, nullptr, FALSE);
                return 0;
            }
            break;
        }
        case WM_MOUSEMOVE: {
            int y = HIWORD(lParam);
            float searchH = 42.0f * m_dpiScale;
            float itemH = 44.0f * m_dpiScale;
            float listY = searchH + 6.0f;
            if (y >= listY) {
                int idx = static_cast<int>((y - listY) / itemH);
                if (idx < static_cast<int>(m_items.size()) && idx != m_hoverIndex) {
                    m_hoverIndex = idx;
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
            }
            return 0;
        }
        case WM_LBUTTONDOWN: {
            int y = HIWORD(lParam);
            float searchH = 42.0f * m_dpiScale;
            float itemH = 44.0f * m_dpiScale;
            float listY = searchH + 6.0f;
            if (y >= listY) {
                int idx = static_cast<int>((y - listY) / itemH);
                if (idx >= 0 && idx < static_cast<int>(m_items.size())) {
                    m_selectedIndex = idx;
                    onConfirmSelection();
                }
            }
            return 0;
        }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace cliphub

#else

namespace cliphub {
PopupPicker::PopupPicker(std::shared_ptr<HistoryRepository> repo, PasteCallback cb)
    : m_repo(std::move(repo)), m_pasteCallback(std::move(cb)) {}
PopupPicker::~PopupPicker() = default;
bool PopupPicker::create(void*) { return true; }
void PopupPicker::showNearCursor(void*) { m_visible = true; }
void PopupPicker::hide() { m_visible = false; }
} // namespace cliphub

#endif
