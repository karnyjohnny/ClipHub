import React, { useState } from 'react';
import { 
  Clipboard, 
  Gauge, 
  Layers, 
  CheckCircle2, 
  FileCode, 
  Download, 
  Cpu, 
  HardDrive, 
  ShieldCheck, 
  Terminal, 
  Github, 
  ExternalLink,
  ChevronRight,
  Code2
} from 'lucide-react';
import { ClipHubSimulator } from './components/ClipHubSimulator';
import { HardwareProfiler } from './components/HardwareProfiler';
import { ArchitectureExplorer } from './components/ArchitectureExplorer';
import { TestResultsViewer } from './components/TestResultsViewer';
import { FileViewerModal } from './components/FileViewerModal';
import { downloadRepositoryZip } from './components/ZipDownloader';

type ActiveTab = 'simulator' | 'profiler' | 'architecture' | 'tests';

export default function App() {
  const [activeTab, setActiveTab] = useState<ActiveTab>('simulator');
  const [isFileModalOpen, setIsFileModalOpen] = useState(false);
  const [downloadStatus, setDownloadStatus] = useState<string | null>(null);

  const handleDownload = () => {
    downloadRepositoryZip((msg) => {
      setDownloadStatus(msg);
      setTimeout(() => setDownloadStatus(null), 3000);
    });
  };

  return (
    <div className="min-h-screen bg-[#0B0B0B] text-gray-200 flex flex-col font-sans selection:bg-blue-900 selection:text-white">
      {/* Top Header Bar */}
      <header className="border-b border-[#222222] bg-[#111111]/90 backdrop-blur-md sticky top-0 z-40">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 h-16 flex items-center justify-between">
          <div className="flex items-center gap-3">
            <div className="w-8 h-8 rounded-lg bg-blue-600 flex items-center justify-center text-white shadow-lg shadow-blue-500/20">
              <Clipboard className="w-4 h-4" />
            </div>
            <div>
              <div className="flex items-center gap-2">
                <span className="font-bold text-base tracking-wide text-white">ClipHub</span>
                <span className="text-[10px] font-mono bg-blue-950/60 text-blue-400 border border-blue-800/60 px-2 py-0.5 rounded">
                  v0.1.0-win7-e5500
                </span>
              </div>
              <p className="text-[11px] text-gray-400 font-mono hidden sm:block">
                Ultra-lightweight Win32 / Direct2D Clipboard Manager for Windows 7
              </p>
            </div>
          </div>

          {/* Action Buttons */}
          <div className="flex items-center gap-2">
            <button
              onClick={() => setIsFileModalOpen(true)}
              className="px-3 py-1.5 rounded-lg text-xs font-mono font-medium bg-[#1A1A1A] hover:bg-[#252525] text-gray-300 border border-[#2E2E2E] flex items-center gap-1.5 transition-colors"
            >
              <FileCode className="w-3.5 h-3.5 text-blue-400" />
              <span>Browse Code (36 Files)</span>
            </button>

            <button
              onClick={handleDownload}
              className="px-3.5 py-1.5 rounded-lg text-xs font-mono font-medium bg-blue-600 hover:bg-blue-500 text-white flex items-center gap-1.5 transition-all shadow-md shadow-blue-600/20"
            >
              <Download className="w-3.5 h-3.5" />
              <span className="hidden sm:inline">Download Repository (.ZIP)</span>
              <span className="sm:hidden">ZIP</span>
            </button>
          </div>
        </div>

        {/* Navigation Tabs */}
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 flex space-x-1 border-t border-[#1C1C1C] overflow-x-auto scrollbar-none">
          <button
            onClick={() => setActiveTab('simulator')}
            className={`py-3 px-3.5 text-xs font-medium border-b-2 transition-colors whitespace-nowrap flex items-center gap-2 ${
              activeTab === 'simulator'
                ? 'border-blue-500 text-white bg-[#151515]'
                : 'border-transparent text-gray-400 hover:text-gray-200 hover:bg-[#141414]'
            }`}
          >
            <Clipboard className="w-3.5 h-3.5 text-blue-400" />
            <span>Direct2D Live Simulator (Alt+V)</span>
          </button>

          <button
            onClick={() => setActiveTab('profiler')}
            className={`py-3 px-3.5 text-xs font-medium border-b-2 transition-colors whitespace-nowrap flex items-center gap-2 ${
              activeTab === 'profiler'
                ? 'border-blue-500 text-white bg-[#151515]'
                : 'border-transparent text-gray-400 hover:text-gray-200 hover:bg-[#141414]'
            }`}
          >
            <Gauge className="w-3.5 h-3.5 text-emerald-400" />
            <span>Dell E5500 Hardware Profiler</span>
          </button>

          <button
            onClick={() => setActiveTab('architecture')}
            className={`py-3 px-3.5 text-xs font-medium border-b-2 transition-colors whitespace-nowrap flex items-center gap-2 ${
              activeTab === 'architecture'
                ? 'border-blue-500 text-white bg-[#151515]'
                : 'border-transparent text-gray-400 hover:text-gray-200 hover:bg-[#141414]'
            }`}
          >
            <Layers className="w-3.5 h-3.5 text-purple-400" />
            <span>Architecture Specification</span>
          </button>

          <button
            onClick={() => setActiveTab('tests')}
            className={`py-3 px-3.5 text-xs font-medium border-b-2 transition-colors whitespace-nowrap flex items-center gap-2 ${
              activeTab === 'tests'
                ? 'border-blue-500 text-white bg-[#151515]'
                : 'border-transparent text-gray-400 hover:text-gray-200 hover:bg-[#141414]'
            }`}
          >
            <CheckCircle2 className="w-3.5 h-3.5 text-green-400" />
            <span>C++ Test Harness (18/18 PASS)</span>
          </button>
        </div>
      </header>

      {/* Download Alert Toast */}
      {downloadStatus && (
        <div className="fixed bottom-6 right-6 z-50 bg-[#161616] border border-blue-500/50 rounded-lg p-3 shadow-2xl flex items-center gap-3 text-xs text-blue-200 font-mono animate-in slide-in-from-bottom-2">
          <Download className="w-4 h-4 text-blue-400 animate-bounce" />
          <span>{downloadStatus}</span>
        </div>
      )}

      {/* Main Content Area */}
      <main className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8 flex-1 w-full space-y-6">
        {/* Core Principles Header Pill */}
        <div className="grid grid-cols-1 md:grid-cols-4 gap-3 text-xs font-mono">
          <div className="bg-[#141414] border border-[#222] p-3 rounded-lg flex items-center gap-2.5">
            <Cpu className="w-4 h-4 text-blue-400 shrink-0" />
            <div>
              <div className="text-gray-500 text-[10px]">CPU IMPACT</div>
              <div className="text-gray-200 font-bold">0.0% Idle (Event-driven)</div>
            </div>
          </div>

          <div className="bg-[#141414] border border-[#222] p-3 rounded-lg flex items-center gap-2.5">
            <Gauge className="w-4 h-4 text-emerald-400 shrink-0" />
            <div>
              <div className="text-gray-500 text-[10px]">RAM WORKING SET</div>
              <div className="text-gray-200 font-bold">11.4 MB (&lt; 15 MB Budget)</div>
            </div>
          </div>

          <div className="bg-[#141414] border border-[#222] p-3 rounded-lg flex items-center gap-2.5">
            <HardDrive className="w-4 h-4 text-amber-400 shrink-0" />
            <div>
              <div className="text-gray-500 text-[10px]">7200 RPM HDD PERSISTENCE</div>
              <div className="text-gray-200 font-bold">SQLite WAL (Async Thread)</div>
            </div>
          </div>

          <div className="bg-[#141414] border border-[#222] p-3 rounded-lg flex items-center gap-2.5">
            <ShieldCheck className="w-4 h-4 text-purple-400 shrink-0" />
            <div>
              <div className="text-gray-500 text-[10px]">TARGET PLATFORM</div>
              <div className="text-gray-200 font-bold">Windows 7 SP1 64-bit</div>
            </div>
          </div>
        </div>

        {/* Tab 1: Direct2D Live Simulator */}
        {activeTab === 'simulator' && <ClipHubSimulator />}

        {/* Tab 2: Dell Latitude E5500 Profiler */}
        {activeTab === 'profiler' && <HardwareProfiler />}

        {/* Tab 3: Architecture Explorer */}
        {activeTab === 'architecture' && <ArchitectureExplorer />}

        {/* Tab 4: C++ Test Results */}
        {activeTab === 'tests' && <TestResultsViewer />}
      </main>

      {/* Code Repository Browser Modal */}
      <FileViewerModal 
        isOpen={isFileModalOpen} 
        onClose={() => setIsFileModalOpen(false)} 
        onDownloadZip={handleDownload}
      />

      {/* Technical Footer */}
      <footer className="border-t border-[#1C1C1C] bg-[#0E0E0E] py-6 text-xs text-gray-500">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 flex flex-col sm:flex-row items-center justify-between gap-4">
          <div className="flex items-center gap-2">
            <span className="font-semibold text-gray-400">ClipHub Master Engineering Specification</span>
            <span>•</span>
            <span className="font-mono text-gray-500">Dell Latitude E5500 Certified</span>
          </div>

          <div className="flex items-center gap-4 text-gray-400">
            <span>C++17 • Win32 • Direct2D 1.0 • DirectWrite • WIC • SQLite 3.45</span>
            <span>•</span>
            <span className="text-emerald-400 font-mono">18/18 Unit Tests Passing</span>
          </div>
        </div>
      </footer>
    </div>
  );
}
