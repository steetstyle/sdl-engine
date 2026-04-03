#!/bin/bash
# Build SDL3 for macOS first, then build test

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR/../../.."
OSXCROSS_DIR="$PROJECT_ROOT/third_party/osxcross"
SDL_DIR="$PROJECT_ROOT/third_party/sdl"

export OSXCROSS_PATH="$OSXCROSS_DIR"
export PATH="$OSXCROSS_DIR/target/bin:$PATH"

CC="${OSXCROSS_DIR}/target/bin/x86_64-apple-darwin24.5-clang"
CXX="${OSXCROSS_DIR}/target/bin/x86_64-apple-darwin24.5-clang++"

SDL_BUILD_DIR="$SDL_DIR/build/macos"
if [ ! -f "$SDL_BUILD_DIR/libSDL3.so" ]; then
    echo "=== Building SDL3 for macOS ==="
    mkdir -p "$SDL_BUILD_DIR"
    cd "$SDL_BUILD_DIR"
    cmake "$SDL_DIR" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_COMPILER="$CC" \
        -DCMAKE_CXX_COMPILER="$CXX" \
        -DCMAKE_SYSROOT="$OSXCROSS_DIR/target/SDK/MacOSX15.5.sdk" \
        -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13 \
        -DCMAKE_EXE_LINKER_FLAGS="-target x86_64-apple-darwin24.5" \
        -DSDL_COCOA=OFF \
        -DSDL_VIDEO=OFF \
        -DSDL_AUDIO=OFF \
        -DSDL_JOYSTICK=OFF \
        -DSDL_HIDAPI=OFF \
        -DSDL_POWER=OFF \
        -DSDL_SENSOR=OFF \
        -DBUILD_SHARED_LIBS=OFF \
        2>&1 | tail -30
    make -j$(nproc) 2>&1 | tail -20
fi

echo "=== Building test_render_macos ==="
mkdir -p "$SCRIPT_DIR/build"
cd "$SCRIPT_DIR/build"

$CXX -o test_render_macos \
    "$SCRIPT_DIR/../mac/TestRender/test_render.cpp" \
    -I"$SDL_DIR/include" \
    -I"$SCRIPT_DIR/SDL" \
    -L"$SDL_BUILD_DIR" \
    -lSDL3 \
    -target x86_64-apple-darwin24.5 \
    -isysroot "$OSXCROSS_DIR/target/SDK/MacOSX15.5.sdk" \
    -mmacosx_version_min=10.13 \
    -Wl,-rpath,"@executable_path/../Frameworks"

echo ""
echo "Build complete: $SCRIPT_DIR/build/test_render_macos"
file "$SCRIPT_DIR/build/test_render_macos"
