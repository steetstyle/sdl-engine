#!/bin/bash
# macOS native build script

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDL_DIR="$SCRIPT_DIR/../../../third_party/sdl"
SDL_TARGET_DIR="$SCRIPT_DIR/SDL"
SDL_INCLUDE_DIR="$SDL_DIR/include"

echo "=== Building SDL3 Render Test for macOS (Native) ==="

echo "Copying SDL headers..."
mkdir -p "$SDL_TARGET_DIR"
rm -rf "$SDL_TARGET_DIR"/*
cp -r "$SDL_INCLUDE_DIR"/* "$SDL_TARGET_DIR/"

BUILD_DIR="$SCRIPT_DIR/TestRender/build"
mkdir -p "$BUILD_DIR"

cmake "$SCRIPT_DIR/TestRender" \
    -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release

cmake --build "$BUILD_DIR" --parallel

echo "Build complete: $BUILD_DIR/test_render.app"