# ClipHub - Manual Testing Checklist & Verification Guide

This checklist is used for manual QA on the reference target:
**Dell Latitude E5500 (Intel Core 2 Duo, 2 GB DDR2, Windows 7 SP1 64-bit)**.

---

## 1. Startup & Residency

- [ ] Application starts without UAC elevation prompt (standard user rights).
- [ ] System tray icon appears with ClipHub icon and tooltip.
- [ ] Log file `ClipHub.log` is created with level `INFO`.
- [ ] Task Manager confirms memory footprint under 15 MB working set.
- [ ] Process CPU usage settles to 0.0% when idle.
- [ ] Launching a second instance focuses the existing instance instead of spawning duplicate processes.

---

## 2. Clipboard Capture

- [ ] Copying plain text in Notepad triggers `WM_CLIPBOARDUPDATE`.
- [ ] Item appears in ClipHub history with character count.
- [ ] Copying multi-line text preserves formatting and displays sanitized preview.
- [ ] Copying Unicode text (Polish diacritics: `ąęśćłóżź`, Cyrillic, CJK, Emoji) preserves all characters accurately.
- [ ] Copying an image (Snipping Tool, Paint, or browser copy) creates an `[IMG]` history entry with dimensions.
- [ ] Copying duplicate text does NOT create a second duplicate row; it moves the existing item to the top and updates its timestamp.
- [ ] Pausing monitoring via tray menu disables clipboard capture.
- [ ] Resuming monitoring re-enables capture.

---

## 3. Alt+V Popup Picker UX

- [ ] Pressing `Alt+V` in any application (browser, terminal, Notepad) instantly pops up the picker.
- [ ] Popup appears near mouse cursor or active window, clamped to visible monitor boundaries.
- [ ] Background blur or dark aesthetic (#111111) renders smoothly without lag.
- [ ] Keyboard navigation:
  - [ ] `Down Arrow` moves selection downward with wrapping.
  - [ ] `Up Arrow` moves selection upward with wrapping.
  - [ ] `Enter` confirms selection, closes popup, and pastes into the previously focused application.
  - [ ] `Escape` dismisses popup without action.
  - [ ] `Delete` removes selected entry from history.
  - [ ] Typing characters filters the list in real-time.
  - [ ] `Backspace` removes search characters and refreshes list.
- [ ] Clicking an item with the mouse pastes it immediately.
- [ ] Clicking outside the popup dismisses it cleanly.

---

## 4. Paste Injection Mechanics

- [ ] In Notepad: `Alt+V` -> select item -> `Enter` pastes the text at caret.
- [ ] In Command Prompt / PowerShell: item pastes correctly.
- [ ] In Web Browser (Chrome/Firefox): item pastes into input fields.
- [ ] No `Ctrl+Alt+V` phantom shortcuts triggered (Alt release compensation verified).

---

## 5. Storage & Persistence

- [ ] SQLite database `cliphub.db` operates in WAL mode (`cliphub.db-wal` present).
- [ ] Exiting and restarting ClipHub retains previous history.
- [ ] Clearing history preserves pinned items.
- [ ] History retention limit prunes oldest unpinned entries without exceeding configured limit.
