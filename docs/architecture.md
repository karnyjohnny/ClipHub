# ClipHub - Architectural Specification

## 1. System Overview

ClipHub is an ultra-lightweight, high-performance clipboard manager designed specifically for Windows 7 SP1 and low-specification hardware (target: Dell Latitude E5500 with Intel Core 2 Duo, 2 GB DDR2 RAM, 7200 RPM HDD).

The core design philosophy is:
> **Native Win32 foundation + custom Direct2D lightweight renderer.**

No webviews, no Chromium, no .NET runtime, no Electron, and no polling loops.

```
+-------------------------------------------------------------------------+
|                               Win32 OS Layer                            |
|  - AddClipboardFormatListener (Vista+ / Win7 SP1 compatible)            |
|  - RegisterHotKey (Alt+V)                                               |
|  - SendInput (Paste injection with Alt-key release compensation)        |
|  - Shell_NotifyIcon (System tray resident icon)                         |
+------------------------------------+------------------------------------+
                                     |
                         [WM_CLIPBOARDUPDATE]
                                     |
                                     v
+------------------------------------+------------------------------------+
|                         cliphub::ClipboardManager                       |
|  - Format detection (CF_UNICODETEXT, CF_DIB / CF_DIBV5)                 |
|  - Content sanitization & FNV-1a 64-bit hashing                         |
|  - Self-paste echo suppression                                          |
+------------------------------------+------------------------------------+
                                     |
                                     v
+------------------------------------+------------------------------------+
|                        cliphub::HistoryRepository                       |
|                                                                         |
|   +----------------------------+      +-------------------------------+ |
|   |         HOT CACHE          |      |      PERSISTENT STORAGE       | |
|   |  cliphub::LRUCache (RAM)   |      |   cliphub::Database (SQLite)  | |
|   |  - O(1) hash map lookup    |      |   - WAL journal mode          | |
|   |  - O(1) list splice        |      |   - PRAGMA synchronous=NORMAL | |
|   |  - Zero disk I/O on UI     |      |   - 2MB RAM page cache        | |
|   |  - Configurable capacity   |      |   - Async background worker   | |
|   +----------------------------+      +-------------------------------+ |
+------------------------------------+------------------------------------+
                                     |
                                     v
+------------------------------------+------------------------------------+
|                              GUI Layer                                  |
|                                                                         |
|   +----------------------------+      +-------------------------------+ |
|   |    PopupPicker (Alt+V)     |      |          MainWindow           | |
|   |  - Frameless Win32 HWND    |      |  - Win32 Overlapped HWND      | |
|   |  - Direct2D 1.0 GPU/WARP   |      |  - Direct2D 1.0 rendering     | |
|   |  - DirectWrite typography  |      |  - History explorer           | |
|   |  - Keyboard navigation     |      |  - Pinned item management     | |
|   |  - Instant fuzzy search    |      |  - Settings & system status   | |
|   +----------------------------+      +-------------------------------+ |
+-------------------------------------------------------------------------+
```

---

## 2. Key Modules & Responsibilities

### 2.1 `core/ClipboardItem`
- Represents a single clipboard payload (text or image).
- Calculates a 64-bit FNV-1a hash of the content for fast $O(1)$ duplicate checking.
- Maintains creation and last-used millisecond timestamps.
- Pre-truncates preview strings to avoid string processing during Direct2D render loops.

### 2.2 `core/LRUCache`
- Holds the $N$ hottest items in memory (default 20).
- Combines `std::list` (ordering) with `std::unordered_map` (hash lookup).
- When an identical item is copied, it updates `lastUsedAt` and moves the item to the front without inserting duplicate rows.

### 2.3 `storage/Database`
- Single-file SQLite storage (`cliphub.db`).
- Embedded via official SQLite amalgamation (`sqlite3.c` & `sqlite3.h`).
- PRAGMAs configured for mechanical 7200 RPM HDDs:
  - `journal_mode = WAL`: Eliminates read/write lock contention.
  - `synchronous = NORMAL`: Flushes only at checkpoints, preventing spindle stalls.
  - `cache_size = -2000`: Caps SQLite RAM usage at ~2MB.

### 2.4 `core/HistoryRepository`
- Connects the hot LRU cache to SQLite.
- Background worker thread with task queue handles SQLite writes asynchronously. Alt+V and UI actions never block on disk I/O.

### 2.5 `platform/Win32Clipboard`
- Calls `AddClipboardFormatListener` to receive `WM_CLIPBOARDUPDATE`.
- Extracts UTF-16 text (`CF_UNICODETEXT`) and converts to UTF-8.
- Extracts Device Independent Bitmaps (`CF_DIB`) and downscales them to thumbnails.
- Prevents re-capturing its own injected pastes.

### 2.6 `platform/Win32PasteInjector`
- Restores focus to the target window.
- Explicitly tests and releases `VK_MENU` (Alt) before dispatching `Ctrl+V` to avoid accidental `Ctrl+Alt+V` shortcuts in IDEs or terminals.
- Uses `SendInput` to inject keyboard events safely.

### 2.7 `gui/PopupPicker`
- Created as a `WS_POPUP | WS_BORDER` window with `WS_EX_TOPMOST`.
- Uses Direct2D 1.0 (built into Windows 7 SP1) with automatic WARP software fallback on older integrated Intel IGPs.
- DirectWrite provides subpixel antialiased text.
- Full keyboard support (Up/Down, Enter, Esc, Del, Type-to-search).
- Auto-hides on `WM_ACTIVATE` loss.
