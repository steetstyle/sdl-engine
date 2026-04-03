#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDL_DIR="/home/roy/github-projects/sdl-engine/third_party/sdl"
WGPU_DIR="/home/roy/github-projects/sdl-engine/third_party/wgpu-native"

echo "=== Building SDL3 WebGPU Test for Linux ==="

gcc -o "$SCRIPT_DIR/build/test_webgpu" \
    "$SCRIPT_DIR/test_webgpu.c" \
    "$SDL_DIR/sdl3webgpu.c" \
    -I "$SDL_DIR/include" \
    -I "$WGPU_DIR/ffi/webgpu-headers" \
    -I "$WGPU_DIR/ffi" \
    -L "$SDL_DIR/build/linux" \
    -L "$WGPU_DIR/target/debug" \
    -lSDL3 \
    -lwgpu_native \
    -Wl,-rpath,"$SDL_DIR/build/linux" \
    -Wl,-rpath,"$WGPU_DIR/target/debug" \
    -lpthread -ldl -lEGL -lvulkan -lwayland-client -lwayland-cursor -lxkbcommon -lz -lm

echo "Build complete: $SCRIPT_DIR/build/test_webgpu"
echo ""
echo "=== Running test ==="
timeout 3 "$SCRIPT_DIR/build/test_webgpu" 2>&1 || echo "Test exited (timeout is expected without display)"
