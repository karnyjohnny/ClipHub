#pragma once

#ifdef _WIN32
    #ifndef WINVER
        #define WINVER 0x0601         // Windows 7
    #endif
    #ifndef _WIN32_WINNT
        #define _WIN32_WINNT 0x0601   // Windows 7
    #endif
    #ifndef NTDDI_VERSION
        #define NTDDI_VERSION 0x06010000 // NTDDI_WIN7
    #endif

    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    #include <windows.h>
    #include <shellapi.h>
    #include <d2d1.h>
    #include <dwrite.h>
    #include <wincodec.h>
#endif
