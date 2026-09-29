# ClipHub

> **Ultra-lightweight, high-performance clipboard manager tailored for Windows 7 SP1 and low-end hardware.**

[![Build & Test](https://github.com/cliphub/cliphub/actions/workflows/build.yml/badge.svg)](https://github.com/cliphub/cliphub/actions/workflows/build.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows%207%20SP1%20x64-0078D7.svg)](docs/win7-compatibility.md)
[![RAM Footprint](https://img.shields.io/badge/RAM%20Footprint-%3C15%20MB-brightgreen.svg)](docs/performance.md)

---

```
╭──────────────────────────────────────────────────╮
│ 🔍  Search clipboard...                          │
├──────────────────────────────────────────────────┤
│ [T]   https://github.com/cliphub/cliphub         │
│       34 chars • 16:42                           │
│                                                  │
│ [T]   git checkout -b feature/direct2d-renderer  │
│       41 chars • 16:38                           │
│                                                  │
│ [IMG] [Image 1920x1080]                          │
│       PNG • 16:31                                │
│                                                  │
│ [T]   Zażółć gęślą jaźń (Unicode Polish test)    │
│       ★ pinned • 16:25                           │
├──────────────────────────────────────────────────┤
│ ↑ ↓ navigate   Enter paste   Esc close   Del del │
╰──────────────────────────────────────────────────╯
```

---

## 1. What is ClipHub?

Modern versions of Windows have built-in clipboard history (`Win+V`), but Windows 7 does not. Unfortunately, most third-party clipboard managers are built on Electron, WebView2, Qt, Python, or heavy .NET runtimes—consuming 200MB to 500MB of RAM and bogging down older CPUs.

**ClipHub** is engineered with a strict **"Performance First, without Ugly UI"** design:
- **Native Win32 foundation** with a custom **Direct2D 1.0** hardware-accelerated dark renderer.
- **Zero WebViews, zero Electron, zero .NET, zero external DLL dependencies.**
- **Event-driven Win32 clipboard hooks** (`AddClipboardFormatListener` / `WM_CLIPBOARDUPDATE`) ensuring **0.0% CPU usage while idling**.
- **Under 15 MB RAM footprint** (working set) on a fresh boot.
- **Non-blocking disk persistence** using an in-process SQLite amalgamation operating in WAL mode, offloading I/O to a background thread to prevent spindle lag on slow 5400/7200 RPM mechanical hard drives.

---

## 2. Hardware Target & Constraints

The reference hardware baseline for ClipHub is the classic **Dell Latitude E5500**:
- **CPU**: Intel Core 2 Duo (2 cores / 2 threads, 2.0 - 2.4 GHz)
- **RAM**: 2 GB DDR2 (Strict memory conservation: total app footprint < 15 MB)
- **Storage**: 160 GB 7200 RPM SATA HDD (Non-blocking I/O, sequential WAL commits)
- **Graphics**: Intel GMA 4500MHD (Direct2D 1.0 hardware with automatic WARP software fallback)
- **Operating System**: Windows 7 Professional 64-bit Service Pack 1

---

## 3. Key Features

- **Global Hotkey (`Alt+V`)**: Instant pop-up near cursor or active window in under 15ms.
- **Instant Paste Injection**: Automatically sets current clipboard, dismisses popup, restores target window, and synthesizes `Ctrl+V` (with Alt-release compensation to prevent `Ctrl+Alt+V` shortcuts).
- **Text & Image Support**: Captures full Unicode UTF-8/UTF-16 text and bitmap snapshots with generated thumbnails.
- **Intelligent Deduplication**: Copying the same content multiple times updates its timestamp and moves it to the top of the history list without cluttering database records.
- **Hot RAM Cache + Cold SQLite Storage**: $N$ most recent items (default 20) are kept in memory via `LRUCache`. Full history is persisted on disk in `cliphub.db`.
- **Real-Time Substring Search**: Type-to-filter instantly queries cached clips and falls back to SQLite.
- **Pinned Clips**: Pin frequently used templates, commands, or keys (★) so they never get pruned.
- **System Tray Residency**: Lives in the notification area with a right-click menu to pause monitoring, clear history, or open settings.

---

## 4. Keyboard Shortcuts

| Shortcut | Context | Action |
| :--- | :--- | :--- |
| **`Alt + V`** | Global (Any App) | Opens the ClipHub clipboard picker popup |
| **`Up` / `Down`** | Inside Popup | Navigates the history item list (with wrapping) |
| **`Enter`** | Inside Popup | Pastes selected item into previously focused window |
| **`Escape`** | Inside Popup | Closes the picker without action |
| **`Delete`** | Inside Popup | Deletes selected item from history |
| **`Any Character`** | Inside Popup | Filters items in real-time by search query |
| **`Backspace`** | Inside Popup | Deletes last character from search query |

---

## 5. Architecture Overview

```text
ClipHub/
├── .github/workflows/build.yml   # Multi-platform CI (MSVC x64 + MinGW cross-build)
├── cmake/
│   └── toolchain-mingw64.cmake   # MinGW cross-compilation toolchain
├── docs/
│   ├── architecture.md           # Module boundaries, threading, lifecycle
│   ├── build.md                  # Comprehensive build & toolchain guide
│   ├── manual-testing.md         # 20-point manual QA checklist
│   ├── performance.md            # Hardware latency & RAM budget benchmarks
│   └── win7-compatibility.md     # Win7 SP1 API audit & fallback matrix
├── include/
│   ├── core/                     # Types, ClipboardItem, LRUCache, Config, Search
│   ├── gui/                      # Direct2D context, Theme, PopupPicker, MainWindow
│   ├── platform/                 # Win32 clipboard, hotkey, paste injection, tray
│   └── storage/                  # SQLite wrapper, WAL schema, migrations
├── src/                          # Implementations
├── tests/                        # 18 unit tests (100% passing)
├── third_party/sqlite/           # Official SQLite 3.45 amalgamation
├── CMakeLists.txt                # Unified CMake build script
├── CMakePresets.json             # Visual Studio & CLI presets
├── LICENSE                       # MIT License
└── README.md
```

Detailed architectural diagrams and component descriptions can be found in [`docs/architecture.md`](docs/architecture.md).

---

## 6. Building & Installation

### Option A: Build with Visual Studio (MSVC on Windows)
```cmd
git clone https://github.com/cliphub/cliphub.git
cd cliphub
# Automatic compiler detection (Visual Studio / Ninja / MSVC):
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
Output executable: `build\Release\ClipHub.exe` (or `build\ClipHub.exe`)

*(Note: If explicitly specifying `-G "Visual Studio 17 2022"`, ensure you are running on Windows with VS 2022 installed).*

### Option B: Cross-Compile from Linux (MinGW-w64)
```bash
sudo apt-get install -y g++-mingw-w64-x86-64 cmake make
rm -rf build-win64
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw64.cmake -B build-win64 -DCMAKE_BUILD_TYPE=Release
cmake --build build-win64 -j$(nproc)
```
Output executable: `build-win64/ClipHub.exe` (statically linked, zero runtime DLL dependencies).

### Option C: Run Native Core Test Suite (Linux / macOS)
```bash
rm -rf build-tests
cmake -B build-tests -DCMAKE_BUILD_TYPE=Release
cmake --build build-tests -j$(nproc)
./build-tests/cliphub_tests
```
*(If you ever see a CMakeCache.txt directory mismatch error, simply `rm -rf build-tests` and re-run).*

---

## 7. Windows 7 Compatibility Audit

Every Windows API call in ClipHub was audited specifically for Windows 7 SP1:
- `AddClipboardFormatListener` / `WM_CLIPBOARDUPDATE`: Vista+ (avoids `SetClipboardViewer` chain breakages).
- `Direct2D 1.0` & `DirectWrite 1.0`: Native to Windows 7 RTM (avoids Windows 8+ Direct2D 1.1/1.2 APIs).
- `GetDeviceCaps(hdc, LOGPIXELSY)`: Native Win32 (avoids Windows 10 `GetDpiForWindow`).
- `SendInput`: Win2000+ (avoids deprecated `keybd_event`).

Read full verification details in [`docs/win7-compatibility.md`](docs/win7-compatibility.md).

---

## 8. License

This project is licensed under the [MIT License](LICENSE).
Embedded SQLite code is in the public domain.
