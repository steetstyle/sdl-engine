#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDL_DIR="/home/roy/github-projects/sdl-engine/third_party/sdl"
SDL_TTF_DIR="/home/roy/github-projects/sdl-engine/third_party/SDL_ttf"

ANDROID_NDK_ROOT="${ANDROID_NDK_ROOT:-$HOME/Android/Sdk/ndk/28.2.13676358}"
CMAKE_TOOLCHAIN="$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake"

echo "=== Building 2048 Game for Android ==="
echo "SDL source: $SDL_DIR/src"
echo "SDL_ttf source: $SDL_TTF_DIR"

ABIS=("armeabi-v7a" "arm64-v8a" "x86" "x86_64")

for ABI in "${ABIS[@]}"; do
    echo "=== Building for ABI: $ABI ==="
    
    BUILD_DIR="$SCRIPT_DIR/build/$ABI"
    mkdir -p "$BUILD_DIR"
    
    cmake "$SCRIPT_DIR" \
        -B "$BUILD_DIR" \
        -DCMAKE_TOOLCHAIN_FILE="$CMAKE_TOOLCHAIN" \
        -DANDROID_ABI="$ABI" \
        -DANDROID_PLATFORM=android-21 \
        -DSDL_SOURCE_DIR="$SDL_DIR" \
        -DSDL_TTF_SOURCE_DIR="$SDL_TTF_DIR" \
        -DCMAKE_BUILD_TYPE=Release
    
    cmake --build "$BUILD_DIR" --parallel
    
    echo "Success: $BUILD_DIR/game_2048"
done

echo ""
echo "=== All builds complete ==="
