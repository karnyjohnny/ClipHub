import React, { useState } from 'react';
import { REPO_FILES, RepoFile } from '../data/repoFiles';
import { FileCode, Search, Copy, Check, X, Folder, Download } from 'lucide-react';

interface FileViewerModalProps {
  isOpen: boolean;
  onClose: () => void;
  onDownloadZip: () => void;
}

export const FileViewerModal: React.FC<FileViewerModalProps> = ({ isOpen, onClose, onDownloadZip }) => {
  const [selectedFile, setSelectedFile] = useState<RepoFile>(REPO_FILES[0]);
  const [fileContent, setFileContent] = useState<string>('Loading file content...');
  const [copied, setCopied] = useState(false);
  const [search, setSearch] = useState('');
  const [activeCategory, setActiveCategory] = useState<string>('ALL');

  React.useEffect(() => {
    if (!isOpen) return;

    // Fetch file content via fetch or standard text loading
    fetch(`/${selectedFile.path}`)
      .then(res => {
        if (!res.ok) throw new Error('File not accessible via HTTP');
        return res.text();
      })
      .then(text => setFileContent(text))
      .catch(() => {
        setFileContent(`// Content of ${selectedFile.path}\n// Ready in repository workspace.`);
      });
  }, [selectedFile, isOpen]);

  if (!isOpen) return null;

  const categories = ['ALL', 'core', 'storage', 'platform', 'gui', 'tests', 'docs', 'build'];

  const filteredFiles = REPO_FILES.filter(f => {
    const matchesCat = activeCategory === 'ALL' || f.category === activeCategory;
    const matchesSearch = !search || f.path.toLowerCase().includes(search.toLowerCase()) || f.description.toLowerCase().includes(search.toLowerCase());
    return matchesCat && matchesSearch;
  });

  const handleCopy = () => {
    navigator.clipboard.writeText(fileContent);
    setCopied(true);
    setTimeout(() => setCopied(false), 2000);
  };

  return (
    <div className="fixed inset-0 bg-black/80 backdrop-blur-sm z-50 flex items-center justify-center p-4 sm:p-6">
      <div className="bg-[#111111] border border-[#2A2A2A] rounded-xl w-full max-w-5xl h-[85vh] flex flex-col shadow-2xl overflow-hidden">
        {/* Modal Header */}
        <div className="px-5 py-3.5 bg-[#181818] border-b border-[#2A2A2A] flex items-center justify-between">
          <div className="flex items-center gap-2">
            <FileCode className="w-4 h-4 text-blue-400" />
            <span className="font-semibold text-sm text-white">ClipHub C++ Source Repository Explorer</span>
            <span className="text-xs font-mono text-gray-500">({REPO_FILES.length} files)</span>
          </div>

          <div className="flex items-center gap-2">
            <button
              onClick={onDownloadZip}
              className="px-3 py-1 rounded-md text-xs font-mono font-medium bg-blue-600 hover:bg-blue-500 text-white flex items-center gap-1.5 transition-colors shadow-sm"
            >
              <Download className="w-3.5 h-3.5" />
              <span>Download ZIP</span>
            </button>
            <button
              onClick={onClose}
              className="p-1.5 rounded-md text-gray-400 hover:text-white hover:bg-[#2A2A2A] transition-colors"
            >
              <X className="w-4 h-4" />
            </button>
          </div>
        </div>

        {/* Modal Body */}
        <div className="flex-1 flex flex-col md:flex-row overflow-hidden">
          {/* Left: File Tree & Search */}
          <div className="w-full md:w-80 bg-[#141414] border-r border-[#2A2A2A] flex flex-col">
            <div className="p-3 border-b border-[#2A2A2A]">
              <div className="relative">
                <Search className="w-3.5 h-3.5 text-gray-500 absolute left-2.5 top-2.5" />
                <input
                  type="text"
                  placeholder="Filter files..."
                  value={search}
                  onChange={e => setSearch(e.target.value)}
                  className="w-full bg-[#1A1A1A] border border-[#2A2A2A] rounded px-8 py-1.5 text-xs text-gray-200 placeholder-gray-500 focus:outline-none focus:border-blue-500"
                />
              </div>

              {/* Category Pills */}
              <div className="flex flex-wrap gap-1 mt-2">
                {categories.map(c => (
                  <button
                    key={c}
                    onClick={() => setActiveCategory(c)}
                    className={`px-1.5 py-0.5 rounded text-[10px] font-mono transition-colors ${
                      activeCategory === c
                        ? 'bg-blue-600 text-white font-medium'
                        : 'bg-[#1E1E1E] text-gray-400 hover:text-gray-200'
                    }`}
                  >
                    {c}
                  </button>
                ))}
              </div>
            </div>

            {/* File List */}
            <div className="flex-1 overflow-y-auto divide-y divide-[#1D1D1D] p-1.5">
              {filteredFiles.map((file, idx) => {
                const isSelected = selectedFile.path === file.path;
                return (
                  <div
                    key={idx}
                    onClick={() => setSelectedFile(file)}
                    className={`p-2 rounded cursor-pointer transition-colors text-xs ${
                      isSelected 
                        ? 'bg-[#1E283D] text-white border-l-2 border-blue-500' 
                        : 'text-gray-400 hover:bg-[#1A1A1A] hover:text-gray-200'
                    }`}
                  >
                    <div className="font-mono truncate font-medium text-[11px] text-gray-200">
                      {file.path}
                    </div>
                    <div className="text-[10px] text-gray-500 truncate mt-0.5">
                      {file.description}
                    </div>
                  </div>
                );
              })}
            </div>
          </div>

          {/* Right: Code Viewer */}
          <div className="flex-1 flex flex-col bg-[#111111] overflow-hidden">
            <div className="px-4 py-2 bg-[#161616] border-b border-[#2A2A2A] flex items-center justify-between text-xs">
              <span className="font-mono text-gray-300 font-semibold">{selectedFile.path}</span>
              <button
                onClick={handleCopy}
                className="flex items-center gap-1 text-[11px] font-mono text-gray-400 hover:text-white px-2 py-0.5 rounded bg-[#202020] hover:bg-[#2A2A2A] transition-colors border border-[#2A2A2A]"
              >
                {copied ? <Check className="w-3 h-3 text-emerald-400" /> : <Copy className="w-3 h-3" />}
                <span>{copied ? 'Copied' : 'Copy Content'}</span>
              </button>
            </div>

            <div className="flex-1 overflow-auto p-4 font-mono text-xs text-gray-300 bg-[#0E0E0E] leading-relaxed whitespace-pre selection:bg-blue-900">
              {fileContent}
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};
