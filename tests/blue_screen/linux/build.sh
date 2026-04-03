#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDL_DIR="/home/roy/github-projects/sdl-engine/third_party/sdl"

echo "=== Building SDL3 Render Test for Linux ==="

gcc -o "$SCRIPT_DIR/test_render_linux" \
    "$SCRIPT_DIR/../test_render.c" \
    -I "$SDL_DIR/src/include" \
    -L "$SDL_DIR/linux/lib/x86_64" \
    -lSDL3 \
    -Wl,-rpath,"$SDL_DIR/linux/lib/x86_64"

echo "Build complete: $SCRIPT_DIR/test_render_linux"
echo ""
echo "=== Running test ==="
timeout 3 "$SCRIPT_DIR/test_render_linux" 2>&1 || echo "Test exited (timeout is expected without display)"
