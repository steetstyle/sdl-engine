# SDL3 Android Example

This example demonstrates how to build an SDL3 application for Android using CMake with Gradle.

## Project Structure

```
examples/android/
├── app/
│   ├── build.gradle              # Gradle app configuration
│   ├── jni/
│   │   ├── CMakeLists.txt        # CMake entry point (uses SDL source via symlink)
│   │   ├── Application.mk        # NDK ABI configuration  
│   │   ├── SDL -> ../../../third_party/sdl/src  # Symlink to SDL3 source
│   │   └── src/
│   │       ├── CMakeLists.txt    # App CMake config
│   │       └── YourSourceHere.c  # Render test source
│   └── src/main/
│       ├── AndroidManifest.xml
│       └── java/org/libsdl/app/ # SDL3 Java bindings (SDLActivity.java, etc.)
├── build.gradle                  # Root Gradle config
└── gradle.properties             # Gradle settings (Java 17)
```

## Prerequisites

- Android SDK with NDK 28.2.13676358
- Java 17 (NOT Java 21 - AGP 8.7.3 has issues with Java 21's compiler)
- Android NDK

## Building

```bash
cd examples/android

# Set NDK and SDK paths
export ANDROID_NDK_HOME=/path/to/ndk/28.2.13676358
export ANDROID_SDK_ROOT=/path/to/sdk

# Build with CMake (recommended - better path handling)
./gradlew assembleDebug -PBUILD_WITH_CMAKE=1

# Or build with ndk-build (deprecated)
./gradlew assembleDebug
```

## Output

The APK will be at:
```
app/build/outputs/apk/debug/app-debug.apk
```

The APK contains:
- `lib/arm64-v8a/libSDL3.so` - SDL3 library
- `lib/arm64-v8a/libmain.so` - Your app's native library

## Test App

The default app (`YourSourceHere.c`) creates a blue window that stays open until you press back. It demonstrates:
- SDL3 initialization
- Window creation
- Renderer creation  
- Drawing a blue background

## Using with Your SDL Project

1. Replace `app/jni/src/YourSourceHere.c` with your source files
2. Update `app/jni/src/CMakeLists.txt` to include your sources
3. Rebuild with `./gradlew assembleDebug -PBUILD_WITH_CMAKE=1`

================================================================================
## TROUBLESHOOTING GUIDE

### 1. "Module main depends on undefined modules: SDL3" (ndk-build)

**Problem:**
```
Android NDK: Module main depends on undefined modules: SDL3
```

**Cause:** The ndk-build couldn't find the SDL3 source files.

**Solution (used CMake instead):**
```bash
# Use CMake instead of ndk-build
./gradlew assembleDebug -PBUILD_WITH_CMAKE=1
```

CMake handles paths better via the `add_subdirectory()` directive.

---

### 2. "dlopen failed: library 'libmain.so' not found"

**Problem:**
```
dlopen failed: library "libmain.so" not found
```

**Cause:** ndk-build was looking for the native library in the wrong location.

**Solution:**
- Use CMake instead of ndk-build (see above)
- The CMakeLists.txt properly builds and packages the shared library

---

### 3. "The source directory does not appear to contain CMakeLists.txt"

**Problem:**
```
CMake Error: The source directory "/home/.../tests" does not appear 
to contain CMakeLists.txt.
```

**Cause:** CMake was invoked with wrong working directory.

**Solution:** Use correct CMake invocation:
```bash
# Wrong - runs cmake in build dir
cd build && cmake .. 

# Correct - specifies source and build dirs
cmake /path/to/source -B /path/to/build
```

---

### 4. Java 21 "does not provide the required capabilities: [JAVA_COMPILER]"

**Problem:**
```
Toolchain installation '/usr/lib/jvm/java-21-openjdk-amd64' does not 
provide the required capabilities: [JAVA_COMPILER]
```

**Cause:** AGP 8.7.3 requires Java 17, not Java 21.

**Solution:** Add to `gradle.properties`:
```properties
org.gradle.java.home=/usr/lib/jvm/java-17-openjdk-amd64
```

---

### 5. "use of undeclared identifier" - SDL3 API Changes

**Problem:**
```
error: use of undeclared identifier 'SDL_WINDOW_WIDTH_CENTERED'
error: use of undeclared identifier 'SDL_WINDOW_HEIGHT_CENTERED'
error: too many arguments to function call, expected 4, have 6
```

**Cause:** SDL3 changed the API from SDL2. In SDL3:
- `SDL_CreateWindow()` takes 4 arguments (no x,y position)
- No more `SDL_WINDOW_WIDTH_CENTERED` / `SDL_WINDOW_HEIGHT_CENTERED`

**Solution:** Use new SDL3 API:
```c
// SDL2 (OLD):
SDL_CreateWindow("Title", 100, 100, 640, 480, SDL_WINDOW_RESIZABLE);

// SDL3 (NEW):
SDL_CreateWindow("Title", 640, 480, SDL_WINDOW_RESIZABLE);
```

---

### 6. "undefined symbol: main" - SDL3 Main Entry Point

**Problem:**
```
ld.lld: error: undefined symbol: main
```

**Cause:** SDL3 on Android requires special main handling. When using 
`SDL_RunApp()` (callback-based), there's no traditional main(). But when
using `main()` directly with `SDL_SetMainReady()`, you need special linking.

**Solution:** Don't use `SDL_MAIN_NO_MACROS`. Instead:
```c
// Include SDL_main.h - this provides the main() wrapper
#include <SDL3/SDL_main.h>

// Use normal main() - SDL_main.h handles the JNI bridging
int main(int argc, char *argv[]) {
    SDL_SetMainReady();
    // ... your code
}
```

The SDL_main.h macro transforms your main() to work with Android's JNI.

---

### 7. APP_ALLOW_MISSING_DEPS=true for ndk-build

**Problem:** If using ndk-build with external SDL3 module

**Solution:** Add to `app/jni/Application.mk`:
```makefile
APP_ALLOW_MISSING_DEPS=true
```

---

### 8. NDK Not Found

**Problem:**
```
Could not find NDK
```

**Solution:** Set environment variables:
```bash
export ANDROID_NDK_HOME=/path/to/ndk/28.2.13676358
export ANDROID_SDK_ROOT=/path/to/sdk
```

Or set in `gradle.properties`:
```properties
android.ndkPath=/path/to/ndk/28.2.13676358
```

---

### 9. Rebuilding After Changes

```bash
# Clean rebuild
./gradlew clean

# Or just rebuild native code
./gradlew externalNativeBuildCleanDebug
./gradlew assembleDebug -PBUILD_WITH_CMAKE=1
```

---

### 10. Debugging the APK

```bash
# Check APK contents
unzip -l app/build/outputs/apk/debug/app-debug.apk

# Install on device
adb install -r app/build/outputs/apk/debug/app-debug.apk

# View logs
adb logcat | grep -E "SDL|main\("

# Launch app
adb shell am start -n org.libsdl.app/org.libsdl.app.SDLActivity
```

================================================================================

## Key Takeaways

1. **Use CMake, not ndk-build** - Better path resolution and integration
2. **Java 17 required** - Not Java 21
3. **SDL3 API changed** - No x,y in CreateWindow, no CENTERED constants
4. **SDL_main.h is required** - For Android main() to work
5. **Use symlink for SDL source** - `app/jni/SDL -> third_party/sdl/src`
