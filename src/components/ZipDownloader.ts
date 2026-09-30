import JSZip from 'jszip';
import { REPO_FILES } from '../data/repoFiles';

export async function downloadRepositoryZip(onProgress?: (msg: string) => void): Promise<void> {
  if (onProgress) onProgress('Preparing ClipHub repository files...');

  const zip = new JSZip();

  // Load all repository files
  for (const file of REPO_FILES) {
    try {
      if (onProgress) onProgress(`Adding ${file.path}...`);
      const res = await fetch(`/${file.path}`);
      if (res.ok) {
        if (file.path.endsWith('.ico') || file.path.endsWith('.png') || file.path.endsWith('.exe')) {
          const buffer = await res.arrayBuffer();
          zip.file(file.path, buffer);
        } else {
          const text = await res.text();
          zip.file(file.path, text);
        }
      }
    } catch (e) {
      console.warn(`Could not add ${file.path} to zip`, e);
    }
  }

  if (onProgress) onProgress('Compressing archive...');
  const blob = await zip.generateAsync({ 
    type: 'blob',
    compression: 'DEFLATE',
    compressionOptions: { level: 9 }
  });

  if (onProgress) onProgress('Downloading ClipHub-master.zip...');
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = 'ClipHub-Windows7-E5500.zip';
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  URL.revokeObjectURL(url);
}
