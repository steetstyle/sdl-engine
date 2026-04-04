#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDL_DIR="/home/roy/github-projects/sdl-engine/third_party/sdl"
SDL_TTF_DIR="/home/roy/github-projects/sdl-engine/third_party/SDL_ttf"

echo "=== Building game_2048 for Linux ==="

mkdir -p "$SCRIPT_DIR/linux/build"
cd "$SCRIPT_DIR/linux/build"

cmake "$SCRIPT_DIR" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DSDL_SOURCE_DIR="$SDL_DIR" \
    -DSDL_TTF_SOURCE_DIR="$SDL_TTF_DIR"

cmake --build . --parallel

echo "Build complete: $SCRIPT_DIR/linux/build/game_2048"
