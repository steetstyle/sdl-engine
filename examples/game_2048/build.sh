#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDL_ENGINE_DIR="/home/roy/github-projects/sdl-engine"
GAME_NAME="game_2048"

case "$1" in
    android)
        echo "=== Building $GAME_NAME for Android ==="
        cp "$SCRIPT_DIR/game_2048.c" "$SDL_ENGINE_DIR/examples/android/app/jni/src/game_2048.c"
        cd "$SDL_ENGINE_DIR/examples/android"
        ./gradlew assembleDebug -PBUILD_WITH_CMAKE
        
        echo "=== Installing on phone ==="
        adb install -r "$SDL_ENGINE_DIR/examples/android/app/build/outputs/apk/debug/app-debug.apk"
        
        echo "=== Launching game ==="
        adb shell am start -n org.libsdl.app/org.libsdl.app.SDLActivity
        ;;
        
    linux)
        echo "=== Building $GAME_NAME for Linux ==="
        cd "$SCRIPT_DIR/linux"
        ./build.sh
        
        echo "=== Running game ==="
        ./run.sh
        ;;
        
    macos|mac|ios)
        echo "=== Building $GAME_NAME for macOS/iOS ==="
        # Add macOS/iOS build logic here
        echo "Not yet implemented"
        ;;
        
    *)
        echo "Usage: $0 <platform>"
        echo "  android  - Build and run on Android"
        echo "  linux    - Build and run on Linux"
        echo "  macos    - Build for macOS"
        echo "  ios      - Build for iOS"
        exit 1
        ;;
esac

echo "Done!"
