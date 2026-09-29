import React, { useState } from 'react';
import { 
  Layers, 
  Cpu, 
  Database, 
  Eye, 
  Keyboard, 
  HardDrive, 
  ShieldCheck, 
  ArrowRight,
  Code2,
  FileCode,
  CheckCircle2
} from 'lucide-react';

interface ModuleDetail {
  id: string;
  title: string;
  icon: any;
  tag: string;
  files: string[];
  summary: string;
  keyDecisions: string[];
  win7Notes: string;
}

const MODULES: ModuleDetail[] = [
  {
    id: 'clipboard',
    title: 'Win32 Clipboard Engine',
    icon: Keyboard,
    tag: 'Platform / Event-Driven',
    files: ['include/platform/Win32Clipboard.h', 'src/platform/Win32Clipboard.cpp'],
    summary: 'Hooks into the Windows clipboard via AddClipboardFormatListener. When content changes, Windows sends WM_CLIPBOARDUPDATE to our message-only window.',
    keyDecisions: [
      'No polling loops: 0.0% CPU when idle.',
      'Supports CF_UNICODETEXT (UTF-16 converted to UTF-8) and CF_DIB (Bitmaps).',
      'Self-paste echo suppression flag prevents infinite feedback loops during paste injection.'
    ],
    win7Notes: 'AddClipboardFormatListener was introduced in Windows Vista. Unlike legacy SetClipboardViewer, it never breaks the system clipboard chain when other apps terminate.'
  },
  {
    id: 'hotkey',
    title: 'Global Alt+V Hotkey & Paste Injection',
    icon: Cpu,
    tag: 'Platform / Win32',
    files: ['include/platform/Win32Hotkey.h', 'include/platform/Win32PasteInjector.h', 'src/platform/Win32PasteInjector.cpp'],
    summary: 'Registers the system-wide Alt+V hotkey and executes safe simulated paste injection into previously focused windows.',
    keyDecisions: [
      'Captures foreground window handle before displaying picker popup.',
      'Explicitly releases VK_MENU (Alt key) before dispatching Ctrl+V to avoid triggering accidental Ctrl+Alt+V IDE shortcuts.',
      'Uses SendInput rather than deprecated keybd_event.'
    ],
    win7Notes: 'RegisterHotKey and SendInput are native to Win32 and compatible across all Windows versions including Win7 SP1.'
  },
  {
    id: 'cache',
    title: 'Hot LRU Cache & Deduplication',
    icon: Layers,
    tag: 'Core / Domain Logic',
    files: ['include/core/LRUCache.h', 'include/core/ClipboardItem.h', 'src/core/ClipboardItem.cpp'],
    summary: 'In-memory cache keeping the hottest N items (default 20) ready for 0ms lookup. Combines std::list with std::unordered_map for O(1) performance.',
    keyDecisions: [
      '64-bit FNV-1a hashing provides instant duplicate detection.',
      'Copying existing content bumps the item to the front and refreshes its timestamp without duplicate storage.',
      'Pinned items (★) are shielded from LRU eviction.'
    ],
    win7Notes: 'Memory capped to under 2 MB heap footprint. Leaves 99.5% of the 2GB DDR2 RAM free for user tasks.'
  },
  {
    id: 'storage',
    title: 'SQLite WAL Persistent Storage',
    icon: Database,
    tag: 'Storage / HDD Optimized',
    files: ['include/storage/Database.h', 'src/storage/Database.cpp', 'include/core/HistoryRepository.h'],
    summary: 'Embedded SQLite 3.45 amalgamation with WAL journaling and async background worker persistence.',
    keyDecisions: [
      'PRAGMA journal_mode = WAL eliminates disk seek conflicts between reads and writes.',
      'PRAGMA synchronous = NORMAL prevents synchronous spindle flushes.',
      'All database writes are queued to a single dedicated worker thread so UI never blocks on 7200 RPM mechanical HDD seeks.'
    ],
    win7Notes: 'Single-file cliphub.db database. No external SQLite installation or driver required.'
  },
  {
    id: 'renderer',
    title: 'Direct2D 1.0 Custom GUI Renderer',
    icon: Eye,
    tag: 'GUI / Direct2D & DirectWrite',
    files: ['include/gui/D2DCommon.h', 'include/gui/Theme.h', 'src/gui/PopupPicker.cpp', 'src/gui/MainWindow.cpp'],
    summary: 'Modern dark aesthetic rendered directly through Direct2D 1.0 and DirectWrite 1.0 without heavy UI frameworks.',
    keyDecisions: [
      'Zero WebViews, zero Electron, zero Qt, zero WPF.',
      'Automatic fallback to Direct2D WARP software rasterizer if integrated Intel GMA 4500MHD lacks full D2D hardware support.',
      'Palette: #111111 background, #181818 panels, #2A2A2A borders, #5C8DFF accent.'
    ],
    win7Notes: 'Direct2D 1.0 and DirectWrite 1.0 are native to Windows 7. Avoids Windows 8+ Direct2D 1.1/1.2/1.3 APIs.'
  }
];

export const ArchitectureExplorer: React.FC = () => {
  const [selectedModule, setSelectedModule] = useState<ModuleDetail>(MODULES[0]);

  return (
    <div className="bg-[#111111] border border-[#2A2A2A] rounded-xl p-6 text-gray-200 shadow-xl">
      <div className="pb-4 border-b border-[#2A2A2A]">
        <h3 className="text-lg font-semibold text-white flex items-center gap-2">
          <Layers className="w-5 h-5 text-blue-400" />
          ClipHub Modular Architecture Specification
        </h3>
        <p className="text-xs text-gray-400 mt-0.5">
          Decoupled, event-driven architecture designed to achieve sub-15ms response times on dual-core hardware.
        </p>
      </div>

      <div className="grid grid-cols-1 lg:grid-cols-12 gap-6 mt-6">
        {/* Module Selector Buttons */}
        <div className="lg:col-span-4 space-y-2">
          {MODULES.map((mod) => {
            const Icon = mod.icon;
            const isSelected = mod.id === selectedModule.id;

            return (
              <button
                key={mod.id}
                onClick={() => setSelectedModule(mod)}
                className={`w-full text-left p-3 rounded-lg border transition-all flex items-start gap-3 ${
                  isSelected 
                    ? 'bg-[#181818] border-[#5C8DFF] shadow-md ring-1 ring-[#5C8DFF]/40 text-white' 
                    : 'bg-[#141414] border-[#2A2A2A] hover:bg-[#181818] text-gray-400 hover:text-gray-200'
                }`}
              >
                <div className={`p-2 rounded-md shrink-0 ${isSelected ? 'bg-blue-600/20 text-blue-400' : 'bg-[#202020] text-gray-400'}`}>
                  <Icon className="w-4 h-4" />
                </div>
                <div className="overflow-hidden">
                  <div className="text-xs font-semibold truncate text-gray-200">
                    {mod.title}
                  </div>
                  <div className="text-[10px] text-gray-500 font-mono mt-0.5">
                    {mod.tag}
                  </div>
                </div>
              </button>
            );
          })}
        </div>

        {/* Selected Module Deep-Dive View */}
        <div className="lg:col-span-8 bg-[#181818] border border-[#2A2A2A] rounded-lg p-5 flex flex-col justify-between">
          <div>
            <div className="flex flex-wrap items-center justify-between gap-2 pb-3 border-b border-[#2A2A2A]">
              <div className="flex items-center gap-2">
                <span className="text-xs font-mono px-2 py-0.5 rounded bg-blue-950/60 text-blue-400 border border-blue-800/60">
                  {selectedModule.tag}
                </span>
                <h4 className="text-base font-bold text-white">{selectedModule.title}</h4>
              </div>
            </div>

            <p className="text-xs text-gray-300 mt-3 leading-relaxed">
              {selectedModule.summary}
            </p>

            {/* Key Decisions */}
            <div className="mt-4">
              <h5 className="text-[11px] font-semibold uppercase tracking-wider text-gray-400 mb-2 flex items-center gap-1.5">
                <CheckCircle2 className="w-3.5 h-3.5 text-blue-400" />
                Key Engineering Decisions
              </h5>
              <ul className="space-y-1.5 text-xs text-gray-300">
                {selectedModule.keyDecisions.map((dec, i) => (
                  <li key={i} className="flex items-start gap-2">
                    <span className="text-blue-400 mt-1">•</span>
                    <span>{dec}</span>
                  </li>
                ))}
              </ul>
            </div>

            {/* Windows 7 Specific Constraint Audit */}
            <div className="mt-4 p-3 bg-[#121212] rounded border border-[#2A2A2A]">
              <div className="text-[11px] font-semibold text-amber-400 flex items-center gap-1 mb-1">
                <ShieldCheck className="w-3.5 h-3.5" />
                Windows 7 SP1 Compatibility Audit
              </div>
              <p className="text-xs text-gray-400 leading-relaxed">
                {selectedModule.win7Notes}
              </p>
            </div>
          </div>

          {/* Associated Source Files */}
          <div className="mt-5 pt-3 border-t border-[#2A2A2A]">
            <span className="text-[11px] font-mono text-gray-500 uppercase tracking-wider block mb-1.5">
              C++ Source & Header Files
            </span>
            <div className="flex flex-wrap gap-2">
              {selectedModule.files.map((file, i) => (
                <span 
                  key={i} 
                  className="inline-flex items-center gap-1 text-[11px] font-mono bg-[#111111] px-2.5 py-1 rounded text-gray-300 border border-[#2A2A2A]"
                >
                  <FileCode className="w-3 h-3 text-blue-400" />
                  {file}
                </span>
              ))}
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};
