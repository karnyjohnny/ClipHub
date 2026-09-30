import React, { useState, useEffect, useRef } from 'react';
import { 
  Clipboard, 
  Search, 
  CornerDownLeft, 
  Pin, 
  Trash2, 
  Image as ImageIcon, 
  FileText, 
  Monitor, 
  Check, 
  Copy, 
  Terminal, 
  Code, 
  Globe, 
  Sparkles,
  ArrowDown,
  ArrowUp,
  X
} from 'lucide-react';

export interface SimItem {
  id: number;
  type: 'text' | 'image';
  content: string;
  preview: string;
  charCount: number;
  timestamp: string;
  pinned: boolean;
  hash: string;
  width?: number;
  height?: number;
}

const INITIAL_ITEMS: SimItem[] = [
  {
    id: 1,
    type: 'text',
    content: 'https://github.com/cliphub/cliphub',
    preview: 'https://github.com/cliphub/cliphub',
    charCount: 34,
    timestamp: '16:42:15',
    pinned: true,
    hash: '0x9a8b1c2d'
  },
  {
    id: 2,
    type: 'text',
    content: 'git checkout -b feature/direct2d-renderer',
    preview: 'git checkout -b feature/direct2d-renderer',
    charCount: 41,
    timestamp: '16:38:02',
    pinned: false,
    hash: '0x3f4e5a6b'
  },
  {
    id: 3,
    type: 'image',
    content: '[Bitmap DIB 1920x1080 Screenshot]',
    preview: '[Image 1920x1080]',
    charCount: 0,
    timestamp: '16:31:40',
    pinned: false,
    hash: '0x7c8d9e0f',
    width: 1920,
    height: 1080
  },
  {
    id: 4,
    type: 'text',
    content: 'Zażółć gęślą jaźń - test polskich znaków schowka (Unicode UTF-8)',
    preview: 'Zażółć gęślą jaźń - test polskich znaków schowka (Unicode UTF-8)',
    charCount: 63,
    timestamp: '16:25:11',
    pinned: true,
    hash: '0x1a2b3c4d'
  },
  {
    id: 5,
    type: 'text',
    content: 'SELECT id, content_hash, last_used_at FROM clipboard_history ORDER BY last_used_at DESC LIMIT 20;',
    preview: 'SELECT id, content_hash, last_used_at FROM clipboard_history ORDER BY...',
    charCount: 97,
    timestamp: '16:20:05',
    pinned: false,
    hash: '0x5e6f7a8b'
  }
];

export const ClipHubSimulator: React.FC = () => {
  const [items, setItems] = useState<SimItem[]>(INITIAL_ITEMS);
  const [isPickerOpen, setIsPickerOpen] = useState(false);
  const [searchQuery, setSearchQuery] = useState('');
  const [selectedIndex, setSelectedIndex] = useState(0);
  const [hoverIndex, setHoverIndex] = useState<number | null>(null);
  const [recentAction, setRecentAction] = useState<string | null>(null);
  const [editorText, setEditorText] = useState('// Press Alt+V or click the button below to paste from ClipHub history:\n');
  const [targetApp, setTargetApp] = useState<'notepad' | 'terminal'>('notepad');
  const [nextId, setNextId] = useState(6);
  const [duplicatedItemId, setDuplicatedItemId] = useState<number | null>(null);

  const searchInputRef = useRef<HTMLInputElement>(null);

  // Global Alt+V keyboard shortcut listener
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      if (e.altKey && (e.key === 'v' || e.key === 'V')) {
        e.preventDefault();
        setIsPickerOpen(prev => !prev);
      } else if (isPickerOpen) {
        if (e.key === 'Escape') {
          setIsPickerOpen(false);
        } else if (e.key === 'ArrowDown') {
          e.preventDefault();
          setSelectedIndex(prev => (prev + 1) % filteredItems.length);
        } else if (e.key === 'ArrowUp') {
          e.preventDefault();
          setSelectedIndex(prev => (prev - 1 + filteredItems.length) % filteredItems.length);
        } else if (e.key === 'Enter') {
          e.preventDefault();
          if (filteredItems.length > 0) {
            handlePasteItem(filteredItems[selectedIndex]);
          }
        }
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [isPickerOpen, selectedIndex, items, searchQuery]);

  useEffect(() => {
    if (isPickerOpen && searchInputRef.current) {
      searchInputRef.current.focus();
    }
  }, [isPickerOpen]);

  const filteredItems = items.filter(item => {
    if (!searchQuery) return true;
    const q = searchQuery.toLowerCase();
    return item.content.toLowerCase().includes(q) || 
           item.preview.toLowerCase().includes(q) ||
           (item.width && `${item.width}x${item.height}`.includes(q));
  });

  const triggerCopyAction = (text: string, isImage = false, width = 1280, height = 720) => {
    const now = new Date().toTimeString().split(' ')[0];
    
    // Check for duplicate (Section 10: Duplicates - bump to top and update timestamp)
    const existingIndex = items.findIndex(i => i.content === text);
    if (existingIndex !== -1) {
      const existing = items[existingIndex];
      const updatedList = [
        { ...existing, timestamp: now },
        ...items.filter((_, idx) => idx !== existingIndex)
      ];
      setItems(updatedList);
      setDuplicatedItemId(existing.id);
      setTimeout(() => setDuplicatedItemId(null), 1800);
      setRecentAction(`Duplicate detected! Bumping "${existing.preview.slice(0, 30)}" to front & updating timestamp`);
      return;
    }

    const newItem: SimItem = {
      id: nextId,
      type: isImage ? 'image' : 'text',
      content: text,
      preview: isImage ? `[Image ${width}x${height}]` : (text.length > 70 ? text.slice(0, 70) + '...' : text),
      charCount: isImage ? 0 : text.length,
      timestamp: now,
      pinned: false,
      hash: '0x' + Math.random().toString(16).slice(2, 10),
      width: isImage ? width : undefined,
      height: isImage ? height : undefined
    };

    setNextId(nextId + 1);
    setItems([newItem, ...items]);
    setRecentAction(`Captured new clipboard item: ${newItem.preview.slice(0, 35)}`);
  };

  const handlePasteItem = (item: SimItem) => {
    setIsPickerOpen(false);
    // Simulate paste injection into target window
    setEditorText(prev => prev + '\n' + item.content);
    setRecentAction(`Injected "${item.preview.slice(0, 25)}" via simulated SendInput (Ctrl+V)`);

    // Bump timestamp in LRU cache
    const now = new Date().toTimeString().split(' ')[0];
    setItems(prev => [
      { ...item, timestamp: now },
      ...prev.filter(i => i.id !== item.id)
    ]);
  };

  const togglePin = (id: number, e: React.MouseEvent) => {
    e.stopPropagation();
    setItems(items.map(it => it.id === id ? { ...it, pinned: !it.pinned } : it));
  };

  const deleteItem = (id: number, e: React.MouseEvent) => {
    e.stopPropagation();
    setItems(items.filter(it => it.id !== id));
  };

  return (
    <div className="bg-[#111111] border border-[#2A2A2A] rounded-xl p-6 text-gray-200 shadow-2xl relative">
      {/* Top Banner */}
      <div className="flex flex-wrap items-center justify-between gap-4 pb-4 border-b border-[#2A2A2A]">
        <div>
          <div className="flex items-center gap-2">
            <span className="w-2.5 h-2.5 rounded-full bg-blue-500 animate-pulse" />
            <h3 className="text-lg font-semibold text-white tracking-wide">ClipHub Direct2D Live Simulator</h3>
            <span className="text-xs bg-[#202020] text-blue-400 px-2.5 py-0.5 rounded border border-[#2A2A2A] font-mono">
              Windows 7 SP1 • Core 2 Duo Ready
            </span>
          </div>
          <p className="text-xs text-gray-400 mt-1">
            Experience the real-time Alt+V popup picker, instant paste injection, intelligent deduplication, and non-blocking SQLite storage.
          </p>
        </div>

        {/* Global Hotkey Button */}
        <button
          onClick={() => setIsPickerOpen(prev => !prev)}
          className={`px-4 py-2 rounded-lg font-medium text-sm flex items-center gap-2 transition-all shadow-lg ${
            isPickerOpen 
              ? 'bg-blue-600 text-white ring-2 ring-blue-400' 
              : 'bg-[#202020] hover:bg-[#2A2A2A] text-blue-400 border border-[#2A2A2A]'
          }`}
        >
          <Clipboard className="w-4 h-4 text-blue-400" />
          <span>Press <strong>Alt + V</strong></span>
          <span className="text-xs bg-[#111111] px-1.5 py-0.5 rounded text-gray-400">or Click</span>
        </button>
      </div>

      {/* Simulator Workspace */}
      <div className="grid grid-cols-1 lg:grid-cols-12 gap-6 mt-6">
        {/* Left: Test Clipping Injectors & Telemetry */}
        <div className="lg:col-span-4 space-y-4">
          <div className="bg-[#181818] border border-[#2A2A2A] rounded-lg p-4">
            <h4 className="text-xs font-semibold uppercase tracking-wider text-gray-400 mb-3 flex items-center gap-1.5">
              <Copy className="w-3.5 h-3.5 text-blue-400" />
              Simulate Clipboard Copies
            </h4>
            <div className="space-y-2">
              <button
                onClick={() => triggerCopyAction('https://github.com/cliphub/cliphub')}
                className="w-full text-left text-xs bg-[#141414] hover:bg-[#202020] p-2.5 rounded border border-[#2A2A2A] flex items-center justify-between transition-colors group"
              >
                <div className="flex items-center gap-2 overflow-hidden">
                  <Globe className="w-3.5 h-3.5 text-blue-400 shrink-0" />
                  <span className="truncate text-gray-300 group-hover:text-white">GitHub Repository URL</span>
                </div>
                <span className="text-[10px] text-gray-500 font-mono">Test Deduplication</span>
              </button>

              <button
                onClick={() => triggerCopyAction('const d2dFactory = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED);')}
                className="w-full text-left text-xs bg-[#141414] hover:bg-[#202020] p-2.5 rounded border border-[#2A2A2A] flex items-center justify-between transition-colors group"
              >
                <div className="flex items-center gap-2 overflow-hidden">
                  <Code className="w-3.5 h-3.5 text-green-400 shrink-0" />
                  <span className="truncate text-gray-300 group-hover:text-white">Direct2D C++ Code Snippet</span>
                </div>
                <span className="text-[10px] text-gray-500 font-mono">C++17</span>
              </button>

              <button
                onClick={() => triggerCopyAction('Zażółć gęślą jaźń - chrabąszcz brzmi w trzcinie w Szczebrzeszynie (UTF-8)')}
                className="w-full text-left text-xs bg-[#141414] hover:bg-[#202020] p-2.5 rounded border border-[#2A2A2A] flex items-center justify-between transition-colors group"
              >
                <div className="flex items-center gap-2 overflow-hidden">
                  <FileText className="w-3.5 h-3.5 text-amber-400 shrink-0" />
                  <span className="truncate text-gray-300 group-hover:text-white">Unicode Polish Diacritics</span>
                </div>
                <span className="text-[10px] text-gray-500 font-mono">UTF-8/16</span>
              </button>

              <button
                onClick={() => triggerCopyAction('[Bitmap Screenshot 1920x1080]', true, 1920, 1080)}
                className="w-full text-left text-xs bg-[#141414] hover:bg-[#202020] p-2.5 rounded border border-[#2A2A2A] flex items-center justify-between transition-colors group"
              >
                <div className="flex items-center gap-2 overflow-hidden">
                  <ImageIcon className="w-3.5 h-3.5 text-purple-400 shrink-0" />
                  <span className="truncate text-gray-300 group-hover:text-white">Screenshot (CF_DIB Snapshot)</span>
                </div>
                <span className="text-[10px] text-gray-500 font-mono">WIC DIB</span>
              </button>
            </div>
          </div>

          {/* Real-time Status / Event Log */}
          <div className="bg-[#181818] border border-[#2A2A2A] rounded-lg p-3">
            <div className="text-[11px] font-mono text-gray-400 flex items-center justify-between mb-1.5">
              <span>EVENT STREAM</span>
              <span className="text-green-400 text-[10px]">WM_CLIPBOARDUPDATE</span>
            </div>
            <div className="text-xs text-blue-300 font-mono bg-[#111111] p-2 rounded border border-[#2A2A2A] min-h-[44px] flex items-center">
              {recentAction || 'ClipHub listening in background... (CPU: 0.0%)'}
            </div>
          </div>
        </div>

        {/* Right: Target Application (Notepad / Terminal) */}
        <div className="lg:col-span-8 flex flex-col">
          <div className="bg-[#181818] border border-[#2A2A2A] rounded-lg flex-1 flex flex-col overflow-hidden">
            {/* Target Window Title Bar */}
            <div className="bg-[#141414] px-4 py-2.5 border-b border-[#2A2A2A] flex items-center justify-between">
              <div className="flex items-center gap-2">
                <span className="w-3 h-3 rounded-full bg-red-500/80 inline-block" />
                <span className="w-3 h-3 rounded-full bg-yellow-500/80 inline-block" />
                <span className="w-3 h-3 rounded-full bg-green-500/80 inline-block" />
                <span className="text-xs font-mono text-gray-300 ml-2">
                  {targetApp === 'notepad' ? 'Untitled - Notepad (Active Foreground Window)' : 'C:\\Windows\\system32\\cmd.exe'}
                </span>
              </div>
              <div className="flex items-center gap-1.5 text-xs">
                <button
                  onClick={() => setTargetApp('notepad')}
                  className={`px-2 py-0.5 rounded text-[11px] font-mono transition-colors ${
                    targetApp === 'notepad' ? 'bg-[#2A2A2A] text-white' : 'text-gray-400 hover:text-white'
                  }`}
                >
                  Notepad
                </button>
                <button
                  onClick={() => setTargetApp('terminal')}
                  className={`px-2 py-0.5 rounded text-[11px] font-mono transition-colors ${
                    targetApp === 'terminal' ? 'bg-[#2A2A2A] text-white' : 'text-gray-400 hover:text-white'
                  }`}
                >
                  CMD
                </button>
                <button
                  onClick={() => setEditorText('')}
                  className="text-gray-400 hover:text-red-400 ml-2"
                  title="Clear text"
                >
                  <Trash2 className="w-3.5 h-3.5" />
                </button>
              </div>
            </div>

            {/* Target Window Content Area */}
            <div className="p-4 flex-1 min-h-[220px] bg-[#111111] font-mono text-xs text-gray-200 whitespace-pre-wrap overflow-y-auto">
              {editorText}
              <span className="inline-block w-2 h-3.5 bg-blue-500 ml-0.5 animate-pulse align-middle" />
            </div>

            <div className="bg-[#141414] px-4 py-2 border-t border-[#2A2A2A] flex items-center justify-between text-[11px] text-gray-500 font-mono">
              <span>Lines: {editorText.split('\n').length} | Chars: {editorText.length}</span>
              <span>Encoding: UTF-8 | Target: Win32 HWND</span>
            </div>
          </div>
        </div>
      </div>

      {/* Floating Alt+V Direct2D Dark Popup Picker Window */}
      {isPickerOpen && (
        <div className="fixed inset-0 bg-black/60 backdrop-blur-xs flex items-center justify-center z-50 p-4">
          <div 
            className="w-full max-w-[460px] bg-[#111111] border border-[#2A2A2A] rounded-lg shadow-2xl overflow-hidden animate-in fade-in zoom-in-95 duration-100"
            style={{
              boxShadow: '0 20px 40px -10px rgba(0,0,0,0.8), 0 0 0 1px #2A2A2A'
            }}
          >
            {/* Popup Header & Search Input */}
            <div className="p-2.5 bg-[#181818] border-b border-[#2A2A2A] flex items-center gap-2.5">
              <img src="/resources/cliphub.png" alt="ClipHub" className="w-4 h-4 rounded shrink-0 ml-1 shadow-sm" />
              <Search className="w-3.5 h-3.5 text-gray-400 shrink-0" />
              <input
                ref={searchInputRef}
                type="text"
                value={searchQuery}
                onChange={e => {
                  setSearchQuery(e.target.value);
                  setSelectedIndex(0);
                }}
                placeholder="Search clipboard..."
                className="w-full bg-transparent text-sm text-gray-200 placeholder-gray-500 focus:outline-none"
              />
              {searchQuery && (
                <button onClick={() => setSearchQuery('')} className="text-gray-500 hover:text-gray-300">
                  <X className="w-3.5 h-3.5" />
                </button>
              )}
              <span className="text-[10px] text-gray-500 font-mono bg-[#111111] px-1.5 py-0.5 rounded border border-[#2A2A2A]">
                Alt+V
              </span>
            </div>

            {/* Clips List */}
            <div className="max-h-[340px] overflow-y-auto divide-y divide-[#1D1D1D] p-1.5">
              {filteredItems.length === 0 ? (
                <div className="py-8 text-center text-xs text-gray-500">
                  No clipboard items match "{searchQuery}"
                </div>
              ) : (
                filteredItems.map((item, index) => {
                  const isSelected = index === selectedIndex;
                  const isHovered = hoverIndex === index;
                  const isDupBumping = duplicatedItemId === item.id;

                  return (
                    <div
                      key={item.id}
                      onClick={() => handlePasteItem(item)}
                      onMouseEnter={() => {
                        setHoverIndex(index);
                        setSelectedIndex(index);
                      }}
                      onMouseLeave={() => setHoverIndex(null)}
                      className={`relative px-3 py-2.5 rounded cursor-pointer transition-all flex items-center justify-between gap-3 text-xs ${
                        isSelected 
                          ? 'bg-[#1E283D] text-white' 
                          : isHovered 
                          ? 'bg-[#22252C] text-gray-200' 
                          : 'text-gray-300 hover:bg-[#181818]'
                      } ${isDupBumping ? 'ring-1 ring-blue-400 bg-blue-950/40' : ''}`}
                    >
                      {/* Left accent indicator */}
                      {isSelected && (
                        <div className="absolute left-0 top-1.5 bottom-1.5 w-1 bg-[#5C8DFF] rounded-r" />
                      )}

                      <div className="flex items-center gap-2.5 overflow-hidden pl-1 min-w-0 flex-1">
                        {/* Type Badge */}
                        <span className={`px-1.5 py-0.5 rounded font-mono font-bold text-[10px] shrink-0 ${
                          item.type === 'text' 
                            ? 'bg-[#202020] text-[#5C8DFF] border border-[#2A2A2A]' 
                            : 'bg-[#202020] text-[#E5A124] border border-[#2A2A2A]'
                        }`}>
                          {item.type === 'text' ? 'T' : 'IMG'}
                        </span>

                        <div className="overflow-hidden min-w-0 flex-1">
                          <p className="truncate whitespace-nowrap text-gray-200 font-medium leading-tight">
                            {item.preview}
                          </p>
                          <div className="flex items-center gap-2 text-[10px] text-gray-400 mt-1 whitespace-nowrap overflow-hidden">
                            <span>{item.timestamp}</span>
                            <span>•</span>
                            <span>{item.type === 'text' ? `${item.charCount} chars` : `${item.width}x${item.height} PNG`}</span>
                            {item.pinned && (
                              <span className="text-[#E5A124] flex items-center gap-0.5">
                                ★ pinned
                              </span>
                            )}
                          </div>
                        </div>
                      </div>

                      {/* Item Actions */}
                      <div className="flex items-center gap-1 shrink-0 opacity-80 group-hover:opacity-100">
                        <button
                          onClick={(e) => togglePin(item.id, e)}
                          title={item.pinned ? "Unpin item" : "Pin item"}
                          className={`p-1 rounded hover:bg-[#2A2A2A] ${item.pinned ? 'text-[#E5A124]' : 'text-gray-500'}`}
                        >
                          <Pin className="w-3 h-3" />
                        </button>
                        <button
                          onClick={(e) => deleteItem(item.id, e)}
                          title="Delete"
                          className="p-1 rounded text-gray-500 hover:text-red-400 hover:bg-[#2A2A2A]"
                        >
                          <Trash2 className="w-3 h-3" />
                        </button>
                      </div>
                    </div>
                  );
                })
              )}
            </div>

            {/* Popup Footer Hints */}
            <div className="px-3 py-2 bg-[#141414] border-t border-[#2A2A2A] flex items-center justify-between text-[11px] text-gray-500">
              <div className="flex items-center gap-3">
                <span><kbd className="bg-[#202020] px-1 py-0.5 rounded text-gray-300">↑</kbd> <kbd className="bg-[#202020] px-1 py-0.5 rounded text-gray-300">↓</kbd> navigate</span>
                <span><kbd className="bg-[#202020] px-1 py-0.5 rounded text-gray-300">Enter</kbd> paste</span>
                <span><kbd className="bg-[#202020] px-1 py-0.5 rounded text-gray-300">Esc</kbd> close</span>
              </div>
              <span className="font-mono text-[10px] text-blue-400">{filteredItems.length} items</span>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};
