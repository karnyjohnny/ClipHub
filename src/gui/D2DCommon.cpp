#include "gui/D2DCommon.h"
#include "platform/Logger.h"

#ifdef _WIN32

namespace cliphub {

D2DContext& D2DContext::instance() {
    static D2DContext ctx;
    return ctx;
}

bool D2DContext::initialize() {
    if (m_initialized) return true;

    // 1. Initialize COM for WIC and Direct2D
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    // 2. Direct2D 1.0 Factory (Native Windows 7 RTM / SP1)
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_d2dFactory);
    if (FAILED(hr) || !m_d2dFactory) {
        LOG_ERROR("D2DContext: D2D1CreateFactory failed: " + std::to_string(hr));
        return false;
    }

    // 3. DirectWrite Factory
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), 
                             reinterpret_cast<IUnknown**>(&m_dwriteFactory));
    if (FAILED(hr) || !m_dwriteFactory) {
        LOG_ERROR("D2DContext: DWriteCreateFactory failed: " + std::to_string(hr));
        return false;
    }

    // 4. Windows Imaging Component (WIC) Factory
    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IWICImagingFactory, reinterpret_cast<void**>(&m_wicFactory));
    if (FAILED(hr) || !m_wicFactory) {
        LOG_WARN("D2DContext: WICImagingFactory initialization failed (image thumbnails will be limited)");
    }

    m_initialized = true;
    LOG_INFO("D2DContext: Direct2D and DirectWrite initialized successfully");
    return true;
}

void D2DContext::shutdown() {
    if (m_wicFactory) {
        m_wicFactory->Release();
        m_wicFactory = nullptr;
    }
    if (m_dwriteFactory) {
        m_dwriteFactory->Release();
        m_dwriteFactory = nullptr;
    }
    if (m_d2dFactory) {
        m_d2dFactory->Release();
        m_d2dFactory = nullptr;
    }
    if (m_initialized) {
        CoUninitialize();
        m_initialized = false;
    }
}

float D2DContext::getDpiScaleForHwnd(HWND hwnd) {
    HDC hdc = GetDC(hwnd);
    int dpi = hdc ? GetDeviceCaps(hdc, LOGPIXELSY) : 96;
    if (hdc) ReleaseDC(hwnd, hdc);
    return (dpi > 0) ? (static_cast<float>(dpi) / 96.0f) : 1.0f;
}

D2D1_COLOR_F D2DContext::toD2DColor(const ColorRGBA& c) {
    return D2D1::ColorF(c.rF(), c.gF(), c.bF(), c.aF());
}

} // namespace cliphub

#endif
