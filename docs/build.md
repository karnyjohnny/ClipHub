# ClipHub - Build & Compilation Guide

ClipHub is designed to be built in two ways:
1. **Directly on Windows** using MSVC (Visual Studio 2019/2022 or Build Tools) or MinGW-w64.
2. **Cross-compiled from Linux** using MinGW-w64 (`x86_64-w64-mingw32-g++`).
3. **Core unit test suite** compiled natively on any platform with a standard C++17 compiler.

---

## 1. Prerequisites

### Windows Native Build
- Windows 7 SP1 or newer (Windows 10/11 supported for development)
- Visual Studio 2019/2022 (with "Desktop development with C++" workload) OR MinGW-w64
- CMake 3.20+
- Git

### Linux Cross-Compilation Build
- Debian / Ubuntu / Fedora
- `g++-mingw-w64-x86-64`
- `cmake`
- `make`

---

## 2. Windows Native Compilation (Visual Studio / MSVC)

```cmd
git clone https://github.com/cliphub/cliphub.git
cd cliphub

cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

The resulting binaries will be placed in:
`build\Release\ClipHub.exe`

---

## 3. MinGW Cross-Compilation (from Linux)

ClipHub provides a dedicated CMake toolchain file `cmake/toolchain-mingw64.cmake`.

```bash
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw64.cmake -B build-win64 -DCMAKE_BUILD_TYPE=Release
cmake --build build-win64 -j$(nproc)
```

The output executable is statically linked with runtime libraries:
`build-win64/ClipHub.exe`

Because it is statically linked (`-static -static-libgcc -static-libstdc++`), it does not require `libgcc_s_seh-1.dll` or `libstdc++-6.dll` and runs on a fresh Windows 7 SP1 installation out of the box.

---

## 4. Running Unit Tests

```bash
cmake -B build-tests -DCMAKE_BUILD_TYPE=Release
cmake --build build-tests -j$(nproc)
ctest --test-dir build-tests --output-on-failure
# Or directly:
./build-tests/cliphub_tests
```
