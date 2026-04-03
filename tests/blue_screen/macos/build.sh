#!/bin/bash
# macOS build script - requires osxcross and macOS SDK
# Run this on Linux after installing osxcross

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDL_DIR="$SCRIPT_DIR/../../../third_party/sdl"
TOOLCHAIN_DIR="$SCRIPT_DIR/../../../scripts/toolchain"
SDL_TARGET_DIR="$SCRIPT_DIR/SDL"
SDL_INCLUDE_DIR="$SDL_DIR/include"

echo "=== Building SDL3 Render Test for macOS ==="

echo "Copying SDL headers..."
mkdir -p "$SDL_TARGET_DIR"
rm -rf "$SDL_TARGET_DIR"/*
cp -r "$SDL_INCLUDE_DIR"/* "$SDL_TARGET_DIR/"

if [ -z "$OSXCROSS_PATH" ]; then
    echo "Error: OSXCROSS_PATH not set"
    echo "Please install osxcross and set OSXCROSS_PATH"
    exit 1
fi

BUILD_DIR="$SCRIPT_DIR/build"
mkdir -p "$BUILD_DIR"

export PATH="$OSXCROSS_PATH/bin:$PATH"

cmake "$SCRIPT_DIR" \
    -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/osxcross.cmake" \
    -DCMAKE_BUILD_TYPE=Release \
    -DSDL_DIR="$SDL_DIR"

cmake --build "$BUILD_DIR" --parallel

echo "Build complete: $BUILD_DIR/test_render_macos"
