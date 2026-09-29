# ClipHub - Build & Compilation Guide

ClipHub is designed to be built in two ways:
1. **On Windows** using MSVC (Visual Studio 2019/2022 or Build Tools) or MinGW-w64.
2. **On Linux** using MinGW-w64 (`x86_64-w64-mingw32-g++-posix`) for static Windows 7 cross-compilation.
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
- `g++-mingw-w64-x86-64-posix`
- `cmake`
- `make`

---

## 2. Windows Native Compilation (Visual Studio / MSVC)

On Windows, run:
```cmd
git clone https://github.com/cliphub/cliphub.git
cd cliphub

# Automatic generator detection (detects installed Visual Studio or Ninja):
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

If you have Visual Studio 2022 explicitly installed on Windows and wish to generate a `.sln` solution:
```cmd
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
*(Note: Do not pass `-G "Visual Studio 17 2022"` when running on Linux or inside Linux CI runners; Visual Studio generators are exclusively available on Windows).*

The resulting binary will be placed in:
`build\Release\ClipHub.exe` (or `build\ClipHub.exe`)

---

## 3. MinGW Cross-Compilation (from Linux)

To cross-compile a standalone static Windows executable from Linux:

```bash
# 1. Ensure clean build folder
rm -rf build-win64

# 2. Configure with the MinGW toolchain
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw64.cmake -B build-win64 -DCMAKE_BUILD_TYPE=Release

# 3. Build ClipHub.exe
cmake --build build-win64 -j$(nproc)
```

The output executable is statically linked with runtime libraries:
`build-win64/ClipHub.exe`

Because it is statically linked (`-static -static-libgcc -static-libstdc++`), it does not require `libgcc_s_seh-1.dll` or `libstdc++-6.dll` and runs on a fresh Windows 7 SP1 installation out of the box.

---

## 4. Running Native Unit Tests (Linux / macOS / Windows)

```bash
# Clean any previous cache if source location changed
rm -rf build-tests

# Configure and compile
cmake -B build-tests -DCMAKE_BUILD_TYPE=Release
cmake --build build-tests -j$(nproc)

# Run test suite
./build-tests/cliphub_tests
# Or via CTest:
ctest --test-dir build-tests --output-on-failure
```

---

## 5. Troubleshooting Common CMake Errors

### Error: `could not find any instance of Visual Studio`
- **Cause**: You ran `cmake -G "Visual Studio 17 2022"` on a Linux environment (or a Linux GitHub Actions runner like `ubuntu-latest`), or on a Windows machine where Visual Studio 2022 is not installed in the standard path.
- **Fix**:
  - If on Linux: Do not specify a Visual Studio generator. Use standard `cmake -B build-tests` (for native tests) or `cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw64.cmake -B build-win64` (for Windows binary).
  - If on Windows: Simply omit `-G` by running `cmake -B build -DCMAKE_BUILD_TYPE=Release`. CMake will automatically detect your installed compiler.

### Error: `The current CMakeCache.txt directory ... is different than the directory where CMakeCache.txt was created`
- **Cause**: An existing `build` or `build-tests` directory was copied or committed from another machine, or the repository was moved to a new path.
- **Fix**: Remove the build directory and reconfigure:
  ```bash
  rm -rf build-tests build-win64 build
  cmake -B build-tests -DCMAKE_BUILD_TYPE=Release
  ```
