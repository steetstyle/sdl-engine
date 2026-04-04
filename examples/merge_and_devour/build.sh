#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDL_ENGINE_DIR="/home/roy/github-projects/sdl-engine"

case "$1" in
    android)
        cp "$SCRIPT_DIR/merge_and_devour.c" "$SDL_ENGINE_DIR/examples/android/app/jni/src/merge_and_devour.c"
        cd "$SDL_ENGINE_DIR/examples/android"
        ./gradlew assembleDebug -PBUILD_WITH_CMAKE
        adb install -r app/build/outputs/apk/debug/app-debug.apk
        adb shell am start -n org.libsdl.app/org.libsdl.app.SDLActivity
        ;;
    linux)
        echo "Linux build not yet implemented"
        ;;
    *)
        echo "Usage: $0 <android|linux>"
        ;;
esac
