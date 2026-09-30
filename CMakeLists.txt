cmake_minimum_required(VERSION 3.20)
project(ClipHub VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

# Optimization and warning flags
if(MSVC)
    add_compile_options(/W4 /WX- /O2 /Oi /Gy /GS- /D_CRT_SECURE_NO_WARNINGS /DUNICODE /D_UNICODE)
    add_compile_definitions(
        _WIN32_WINNT=0x0601
        WINVER=0x0601
        NTDDI_VERSION=0x06010000
        UNICODE
        _UNICODE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        _CRT_SECURE_NO_WARNINGS
    )
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O2 -pipe)
    if(WIN32)
        # Windows 7 target defines
        add_compile_definitions(
            _WIN32_WINNT=0x0601
            WINVER=0x0601
            NTDDI_VERSION=0x06010000
            UNICODE
            _UNICODE
            NOMINMAX
            WIN32_LEAN_AND_MEAN
        )
    endif()
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/resources
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlite
)

# 1. SQLite3 Amalgamation Static Library
add_library(sqlite3 STATIC
    third_party/sqlite/sqlite3.c
)
target_include_directories(sqlite3 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlite
)
target_compile_definitions(sqlite3 PRIVATE
    SQLITE_THREADSAFE=1
    SQLITE_ENABLE_FTS5=1
    SQLITE_DEFAULT_MEMSTATUS=0
    SQLITE_OMIT_DEPRECATED=1
    SQLITE_DQS=0
)
if(NOT MSVC)
    target_compile_options(sqlite3 PRIVATE -w) # suppress sqlite warnings
endif()

# 2. Core Domain Logic Library (Cross-Platform)
set(CLIPHUB_CORE_SOURCES
    src/core/ClipboardItem.cpp
    src/core/Config.cpp
    src/core/HistoryRepository.cpp
    src/core/SearchEngine.cpp
    src/storage/Database.cpp
    src/platform/Logger.cpp
)

add_library(cliphub_core STATIC ${CLIPHUB_CORE_SOURCES})
target_link_libraries(cliphub_core PUBLIC sqlite3)

# 3. Unit Test Suite (Platform-Agnostic Core Verification)
set(CLIPHUB_TEST_SOURCES
    tests/test_main.cpp
    tests/test_clipboard.cpp
    tests/test_storage.cpp
    tests/test_search.cpp
    tests/test_history.cpp
    tests/test_image.cpp
)

add_executable(cliphub_tests ${CLIPHUB_TEST_SOURCES})
target_include_directories(cliphub_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(cliphub_tests PRIVATE cliphub_core)

enable_testing()
add_test(NAME CoreTests COMMAND cliphub_tests)

# 4. Windows Native Target (ClipHub.exe)
if(WIN32)
    set(CLIPHUB_WIN32_SOURCES
        src/platform/Win32Clipboard.cpp
        src/platform/Win32Hotkey.cpp
        src/platform/Win32PasteInjector.cpp
        src/platform/Win32Tray.cpp
        src/gui/Theme.cpp
        src/gui/D2DCommon.cpp
        src/gui/PopupPicker.cpp
        src/gui/MainWindow.cpp
        src/gui/App.cpp
        src/main.cpp
        resources/ClipHub.rc
    )

    add_executable(ClipHub WIN32 ${CLIPHUB_WIN32_SOURCES})
    target_link_libraries(ClipHub PRIVATE
        cliphub_core
        d2d1
        dwrite
        windowscodecs
        ole32
        shell32
        user32
        gdi32
        advapi32
    )

    # --- POPRAWKA DLA WINDOWS 7 (MSVC) ---
    if(MSVC)
        # Wymuś wersję subsystemu Windows 7 (6.1) w linkerze
        target_link_options(ClipHub PRIVATE "/SUBSYSTEM:WINDOWS,6.01")
    elseif(MINGW)
        # Fully static runtime to guarantee 0-dependency portability on Windows 7
        target_link_options(ClipHub PRIVATE -static -static-libgcc -static-libstdc++)
    endif()
endif()
