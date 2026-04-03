# SDL3 Android Test Build

CMake-based build system for testing SDL3 on Android without Gradle.

## Project Structure

```
tests/android/
├── CMakeLists.txt    # Main CMake config (adds SDL3 as subdirectory)
├── build.sh         # Build script for all 4 ABIs
├── README.md        # This file
└── build/           # Build output (created by build.sh)
    ├── arm64-v8a/
    ├── armeabi-v7a/
    ├── x86/
    └── x86_64/
```

## Prerequisites

- Android NDK 28.2.13676358
- CMake 3.18+

## Building

```bash
cd tests/android
export ANDROID_NDK_ROOT="$HOME/Android/Sdk/ndk/28.2.13676358"
./build.sh
```

## Output

```
tests/android/build/<ABI>/
├── test_render           # Test executable
└── sdl-build/
    └── libSDL3.so       # SDL3 shared library
```

## Testing on Device

```bash
adb push build/arm64-v8a/test_render /data/local/tmp/
adb push build/arm64-v8a/sdl-build/libSDL3.so /data/local/tmp/
adb shell "LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/test_render"
```

================================================================================
## TROUBLESHOOTING GUIDE

### 1. CMake Cannot Find NDK

**Problem:**
```
CMake Error: Could not find NDK
```

**Solution:**
```bash
export ANDROID_NDK_ROOT="$HOME/Android/Sdk/ndk/28.2.13676358"
export ANDROID_SDK_ROOT="$HOME/Android/Sdk"
```

Or edit `build.sh` to set correct path.

---

### 2. "undefined symbol: main"

**Problem:**
```
ld.lld: error: undefined symbol: main
```

**Cause:** Missing SDL_main.h include.

**Solution:**
```c
#include <SDL3/SDL_main.h>

int main(int argc, char *argv[]) {
    // Your code
}
```

---

### 3. CMake Source Directory Error

**Problem:**
```
CMake Error: The source directory does not appear to contain CMakeLists.txt
```

**Cause:** Wrong working directory.

**Solution:** Use correct cmake invocation:
```bash
cmake "$SCRIPT_DIR" -B "$BUILD_DIR" [options]
```

NOT `cd build && cmake ..`

---

### 4. Link Errors (undefined SDL_*)

**Problem:**
```
error: undefined reference to 'SDL_Init'
```

**Solution:** Ensure CMakeLists.txt has:
```cmake
add_subdirectory(${SDL_SOURCE_DIR} sdl-build EXCLUDE_FROM_ALL)
target_link_libraries(test_render PRIVATE SDL3::SDL3)
```

---

### 5. Multiple Build Failures

**Problem:** Each ABI rebuilds SDL3.

**Solution:** This is expected. For single ABI, modify build.sh:
```bash
ABIS=("arm64-v8a")
```

---

### 6. Runtime "library not found"

**Problem:**
```
dlopen failed: library "libSDL3.so" not found
```

**Solution:** Use LD_LIBRARY_PATH:
```bash
adb shell "LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/test_render"
```

---

### 7. SDL3 Configuration Output is Normal

You will see:
```
SDL3 was configured with the following options:
Platform: Android-1
Subsystems: Audio Video GPU Render...
```

This is **normal** - just SDL3 printing its configuration. Not an error.

---

## Key Configuration

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.18)
project(TestRenderAndroid)

set(SDL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/sdl/src")
set(TEST_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." CACHE PATH "Path to test sources")

add_executable(test_render ${TEST_SOURCE_DIR}/test_render.c)
target_include_directories(test_render PRIVATE ${SDL_SOURCE_DIR}/include)

add_subdirectory(${SDL_SOURCE_DIR} sdl-build EXCLUDE_FROM_ALL)
target_link_libraries(test_render PRIVATE SDL3::SDL3)
```

### build.sh Key Variables
```bash
ANDROID_NDK_ROOT      # Path to NDK
CMAKE_TOOLCHAIN      # NDK's cmake toolchain
DANDROID_ABI         # Target ABI
DANDROID_PLATFORM    # Android API level
```

---

## See Also

- `examples/android/` - Full Gradle app example with SDL3
- `third_party/sdl/src/` - SDL3 source code
