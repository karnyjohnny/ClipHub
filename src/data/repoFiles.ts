export interface RepoFile {
  path: string;
  category: 'core' | 'storage' | 'platform' | 'gui' | 'tests' | 'docs' | 'build';
  description: string;
  language: string;
}

export const REPO_FILES: RepoFile[] = [
  // Core
  { path: 'include/core/Types.h', category: 'core', description: 'ItemType enum, ImageMetadata, ClipboardData structures', language: 'cpp' },
  { path: 'include/core/ClipboardItem.h', category: 'core', description: 'Core clipboard item, FNV-1a hash calculation, preview generator', language: 'cpp' },
  { path: 'src/core/ClipboardItem.cpp', category: 'core', description: 'Clipboard item factory methods, Unicode search matching', language: 'cpp' },
  { path: 'include/core/Config.h', category: 'core', description: 'Application configuration, history limits, hotkey definitions', language: 'cpp' },
  { path: 'src/core/Config.cpp', category: 'core', description: 'Ultra-lightweight zero-dependency INI parser & serializer', language: 'cpp' },
  { path: 'include/core/LRUCache.h', category: 'core', description: 'O(1) in-memory hot cache with duplicate refresh & pinned protection', language: 'cpp' },
  { path: 'include/core/HistoryRepository.h', category: 'core', description: 'Coordinates RAM hot cache with async SQLite background worker', language: 'cpp' },
  { path: 'src/core/HistoryRepository.cpp', category: 'core', description: 'Non-blocking disk persistence queue for 7200 RPM HDDs', language: 'cpp' },
  { path: 'include/core/SearchEngine.h', category: 'core', description: 'Fast case-insensitive substring search engine', language: 'cpp' },
  { path: 'src/core/SearchEngine.cpp', category: 'core', description: 'Search filtering implementation', language: 'cpp' },

  // Storage
  { path: 'include/storage/Database.h', category: 'storage', description: 'SQLite persistent storage manager with prepared statements', language: 'cpp' },
  { path: 'src/storage/Database.cpp', category: 'storage', description: 'SQLite WAL mode initialization, indexing, pruning, and queries', language: 'cpp' },
  { path: 'third_party/sqlite/sqlite3.h', category: 'storage', description: 'Official SQLite 3.45.2 Amalgamation Header', language: 'c' },
  { path: 'third_party/sqlite/sqlite3.c', category: 'storage', description: 'Official SQLite 3.45.2 Amalgamation Source', language: 'c' },

  // Platform
  { path: 'include/platform/PlatformDefs.h', category: 'platform', description: 'Windows 7 SP1 target macros (_WIN32_WINNT 0x0601)', language: 'cpp' },
  { path: 'include/platform/Logger.h', category: 'platform', description: 'Lightweight privacy-focused logging (ClipHub.log)', language: 'cpp' },
  { path: 'src/platform/Logger.cpp', category: 'platform', description: 'Thread-safe formatted log output', language: 'cpp' },
  { path: 'include/platform/Win32Clipboard.h', category: 'platform', description: 'AddClipboardFormatListener & format extractor header', language: 'cpp' },
  { path: 'src/platform/Win32Clipboard.cpp', category: 'platform', description: 'Unicode text & DIB image clipboard listener and injector', language: 'cpp' },
  { path: 'include/platform/Win32Hotkey.h', category: 'platform', description: 'RegisterHotKey Alt+V wrapper header', language: 'cpp' },
  { path: 'src/platform/Win32Hotkey.cpp', category: 'platform', description: 'Global Alt+V hotkey dispatching', language: 'cpp' },
  { path: 'include/platform/Win32PasteInjector.h', category: 'platform', description: 'SendInput Ctrl+V injection with Alt release compensation', language: 'cpp' },
  { path: 'src/platform/Win32PasteInjector.cpp', category: 'platform', description: 'Focus restoration and paste key event synthesis', language: 'cpp' },
  { path: 'include/platform/Win32Tray.h', category: 'platform', description: 'Shell_NotifyIcon resident system tray icon header', language: 'cpp' },
  { path: 'src/platform/Win32Tray.cpp', category: 'platform', description: 'Tray context menu & notification handling', language: 'cpp' },

  // GUI
  { path: 'include/gui/Theme.h', category: 'gui', description: 'Dark minimal technical theme palette (#111111, #5C8DFF)', language: 'cpp' },
  { path: 'src/gui/Theme.cpp', category: 'gui', description: 'Theme color definitions', language: 'cpp' },
  { path: 'include/gui/D2DCommon.h', category: 'gui', description: 'Direct2D 1.0, DirectWrite, WIC, and DPI scaling helpers', language: 'cpp' },
  { path: 'src/gui/D2DCommon.cpp', category: 'gui', description: 'Direct2D initialization with WARP software fallback', language: 'cpp' },
  { path: 'include/gui/PopupPicker.h', category: 'gui', description: 'Alt+V quick popup picker window header', language: 'cpp' },
  { path: 'src/gui/PopupPicker.cpp', category: 'gui', description: 'Direct2D hardware-accelerated popup renderer & input handler', language: 'cpp' },
  { path: 'include/gui/MainWindow.h', category: 'gui', description: 'ClipHub main history explorer window header', language: 'cpp' },
  { path: 'src/gui/MainWindow.cpp', category: 'gui', description: 'Direct2D main window interface', language: 'cpp' },
  { path: 'include/gui/App.h', category: 'gui', description: 'Master application coordinator & message-only window loop', language: 'cpp' },
  { path: 'src/gui/App.cpp', category: 'gui', description: 'Event dispatching & resident lifecycle', language: 'cpp' },
  { path: 'src/main.cpp', category: 'gui', description: 'WinMain entry point with single-instance mutex check', language: 'cpp' },

  // Tests
  { path: 'tests/TestHarness.h', category: 'tests', description: 'Microsecond test runner framework', language: 'cpp' },
  { path: 'tests/test_clipboard.cpp', category: 'tests', description: 'Text, Unicode, long string, and FNV-1a hashing tests', language: 'cpp' },
  { path: 'tests/test_storage.cpp', category: 'tests', description: 'SQLite persistence, usage update, delete, and prune tests', language: 'cpp' },
  { path: 'tests/test_search.cpp', category: 'tests', description: 'Substring, case-insensitivity, and query filter tests', language: 'cpp' },
  { path: 'tests/test_history.cpp', category: 'tests', description: 'LRU capacity, duplicate refresh, and repository tests', language: 'cpp' },
  { path: 'tests/test_image.cpp', category: 'tests', description: 'Image metadata and dimension query tests', language: 'cpp' },
  { path: 'tests/test_main.cpp', category: 'tests', description: 'Test suite runner entry point', language: 'cpp' },

  // Build & CI
  { path: 'CMakeLists.txt', category: 'build', description: 'Unified CMake build configuration for MSVC, MinGW, and native tests', language: 'cmake' },
  { path: 'CMakePresets.json', category: 'build', description: 'Presets for Visual Studio 2022, MinGW, and Linux testing', language: 'json' },
  { path: 'cmake/toolchain-mingw64.cmake', category: 'build', description: 'MinGW-w64 x64 cross-compilation toolchain script', language: 'cmake' },
  { path: '.github/workflows/build.yml', category: 'build', description: 'GitHub Actions automated build, test & packaging workflow', language: 'yaml' },
  { path: 'resources/resource.h', category: 'build', description: 'Win32 resource identifiers', language: 'cpp' },
  { path: 'resources/cliphub.ico', category: 'build', description: 'Multi-resolution Windows application & tray icon (16x16 to 256x256)', language: 'binary' },
  { path: 'resources/ClipHub.rc', category: 'build', description: 'Windows 7 version info and resource manifest', language: 'rc' },
  { path: 'LICENSE', category: 'build', description: 'MIT Open Source License', language: 'text' },
  { path: 'README.md', category: 'docs', description: 'Technical documentation & quick start guide', language: 'markdown' },

  // Documentation
  { path: 'docs/architecture.md', category: 'docs', description: 'Comprehensive architectural diagrams & module specifications', language: 'markdown' },
  { path: 'docs/build.md', category: 'docs', description: 'MSVC and MinGW compilation procedures', language: 'markdown' },
  { path: 'docs/manual-testing.md', category: 'docs', description: '20-point manual QA checklist for Dell Latitude E5500', language: 'markdown' },
  { path: 'docs/performance.md', category: 'docs', description: 'Hardware benchmarks, RAM budget, and HDD optimization analysis', language: 'markdown' },
  { path: 'docs/win7-compatibility.md', category: 'docs', description: 'Windows 7 SP1 API audit & fallback matrix', language: 'markdown' }
];

export const TEST_BENCHMARK_RESULTS = [
  { suite: 'Clipboard', test: 'CreateText', status: 'PASS', timeUs: 9, description: 'Text item creation and FNV-1a hash calculation' },
  { suite: 'Clipboard', test: 'EmptyText', status: 'PASS', timeUs: 0, description: 'Handling of zero-length clipboard payloads' },
  { suite: 'Clipboard', test: 'UnicodeSupport', status: 'PASS', timeUs: 1, description: 'Polish diacritics (Zażółć gęślą jaźń) preservation' },
  { suite: 'Clipboard', test: 'LongTextTruncation', status: 'PASS', timeUs: 4, description: '1000-char string sanitized preview truncation' },
  { suite: 'Clipboard', test: 'DuplicateHashCollisionResistance', status: 'PASS', timeUs: 1, description: 'O(1) collision-resistant FNV-1a verification' },
  { suite: 'Storage', test: 'OpenAndClose', status: 'PASS', timeUs: 1979, description: 'SQLite open in WAL mode and safe schema init' },
  { suite: 'Storage', test: 'InsertAndRetrieve', status: 'PASS', timeUs: 1361, description: 'Inserting item and reading back from disk' },
  { suite: 'Storage', test: 'UpdateUsageByHash', status: 'PASS', timeUs: 1603, description: 'Bumping last_used_at on duplicate copy' },
  { suite: 'Storage', test: 'DeleteAndClear', status: 'PASS', timeUs: 1482, description: 'Deleting single item and clearing non-pinned' },
  { suite: 'Storage', test: 'PinnedPreservedOnPrune', status: 'PASS', timeUs: 2409, description: 'Pinned items retained when pruning past limit' },
  { suite: 'Search', test: 'SubstringAndCaseInsensitive', status: 'PASS', timeUs: 16, description: 'Fast case-insensitive substring matching' },
  { suite: 'Search', test: 'EmptyQueryReturnsAll', status: 'PASS', timeUs: 2, description: 'Empty search returns unfiltered hot cache' },
  { suite: 'Search', test: 'NoMatchReturnsEmpty', status: 'PASS', timeUs: 1, description: 'Zero matches handled gracefully' },
  { suite: 'History', test: 'LRUCapacityLimit', status: 'PASS', timeUs: 21, description: 'Hot RAM cache enforces capacity eviction' },
  { suite: 'History', test: 'DuplicateRefreshesTimestampAndPosition', status: 'PASS', timeUs: 9, description: 'Duplicate moves to front in O(1) time' },
  { suite: 'History', test: 'RepositoryEndToEnd', status: 'PASS', timeUs: 51942, description: 'Full async background worker disk sync' },
  { suite: 'Image', test: 'CreateImageMetadata', status: 'PASS', timeUs: 38, description: 'Image DIB thumbnail & dimension tracking' },
  { suite: 'Image', test: 'QueryMatching', status: 'PASS', timeUs: 9, description: 'Searching by image dimensions and format tags' }
];
