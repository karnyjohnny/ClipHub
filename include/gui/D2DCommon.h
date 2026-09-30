#pragma once

#include "gui/Theme.h"
#include <string>

#ifdef _WIN32
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>

namespace cliphub {

class D2DContext {
public:
    static D2DContext& instance();

    bool initialize();
    void shutdown();

    ID2D1Factory* d2dFactory() const { return m_d2dFactory; }
    IDWriteFactory* dwriteFactory() const { return m_dwriteFactory; }
    IWICImagingFactory* wicFactory() const { return m_wicFactory; }

    static float getDpiScaleForHwnd(HWND hwnd);
    static D2D1_COLOR_F toD2DColor(const ColorRGBA& c);
    static std::wstring utf8ToWide(const std::string& utf8);

private:
    D2DContext() = default;
    ~D2DContext() { shutdown(); }

private:
    ID2D1Factory* m_d2dFactory = nullptr;
    IDWriteFactory* m_dwriteFactory = nullptr;
    IWICImagingFactory* m_wicFactory = nullptr;
    bool m_initialized = false;
};

} // namespace cliphub

#else

namespace cliphub {
class D2DContext {
public:
    static D2DContext& instance() { static D2DContext ctx; return ctx; }
    bool initialize() { return true; }
    void shutdown() {}
    static float getDpiScaleForHwnd(void*) { return 1.0f; }
};
} // namespace cliphub

#endif
