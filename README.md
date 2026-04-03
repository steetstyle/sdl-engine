# SDL3 Cross-Platform Engine

A cross-platform SDL3 game engine supporting Linux, Android, macOS, and iOS from a single codebase.

## Project Structure

```
sdl-engine/
├── third_party/
│   └── sdl/              # SDL3 git submodule (https://github.com/libsdl-org/SDL)
│       ├── src/          # SDL3 source code
│       └── android/     # Pre-built Android libraries (armeabi-v7a, arm64-v8a, x86, x86_64)
│
├── examples/
│   └── android/          # Android example app with Gradle
│
├── tests/
│   └── blue_screen/      # Test applications for each platform
│       ├── test_render.c    # Shared test source
│       ├── linux/           # Linux build
│       ├── android/         # Android build (CMake + NDK)
│       ├── macos/           # macOS build
│       └── ios/             # iOS build
│
└── README.md
```

## Quick Start

### Clone with SDL Submodule

```bash
# Clone with submodules
git clone --recursive <repo-url> sdl-engine

# Or if already cloned:
git clone <repo-url> sdl-engine
cd sdl-engine
git submodule update --init --recursive
```

### Prerequisites

| Platform | Requirements |
|----------|-------------|
| Linux | GCC, SDL3 source |
| Android | Android NDK 28.2.13676358, Java 17, Gradle |
| macOS | Xcode, osxcross (for cross-compilation) |
| iOS | macOS with Xcode |

## Building Tests

### Linux
```bash
cd tests/blue_screen/linux
./build.sh
```
Output: `test_render_linux`

### Android
```bash
cd tests/blue_screen/android
./build.sh
```
Output: `build/<ABI>/test_render` for each ABI

### Running on Android (APK)

```bash
cd examples/android

# Copy test source
cp ../../tests/blue_screen/test_render.c app/jni/src/YourSourceHere.c

# Build APK
export ANDROID_NDK_HOME=/path/to/ndk/28.2.13676358
export ANDROID_SDK_ROOT=/path/to/sdk
./gradlew assembleDebug -PBUILD_WITH_CMAKE=1

# Install & run
adb install app/build/outputs/apk/debug/app-debug.apk
adb shell am start -n org.libsdl.app/org.libsdl.app.SDLActivity
```

## Pre-built Libraries

This project includes pre-built SDL3 libraries:

- **Linux**: `third_party/sdl/linux/lib/x86_64/libSDL3.so`
- **Android**: `third_party/sdl/android/lib/{armeabi-v7a,arm64-v8a,x86,x86_64}/libSDL3.so`

## Rebuilding SDL3

### Linux
```bash
cd third_party/sdl
mkdir -p build/linux
cd build/linux
cmake ../.. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Android
```bash
cd third_party/sdl
mkdir -p build/android
cd build/android
cmake ../.. \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-21 \
    -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## SDL3 API Notes

SDL3 has API changes from SDL2:

```c
// SDL2 (OLD)
SDL_CreateWindow("Title", x, y, w, h, flags);

// SDL3 (NEW)
SDL_CreateWindow("Title", w, h, flags);  // No x,y position

// No more SDL_WINDOW_WIDTH_CENTERED / SDL_WINDOW_HEIGHT_CENTERED
```

## License

SDL3 is under zlib license. See third_party/sdl/LICENSE for details.
