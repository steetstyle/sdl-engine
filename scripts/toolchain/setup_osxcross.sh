#!/bin/bash
# osxcross setup script for cross-compiling to macOS
# Run this on Linux to set up the cross-compilation toolchain

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OSXCROSS_DIR="$SCRIPT_DIR/../../third_party/osxcross"

echo "=== Setting up osxcross for macOS cross-compilation ==="

if [ -d "$OSXCROSS_DIR" ]; then
    echo "osxcross already exists at $OSXCROSS_DIR"
    read -p "Reinstall? (y/n): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 0
    fi
    rm -rf "$OSXCROSS_DIR"
fi

echo "Cloning osxcross..."
git clone --depth 1 https://github.com/tpoechtrager/osxcross.git "$OSXCROSS_DIR"

cd "$OSXCROSS_DIR"

echo "Downloading macOS SDK..."
echo "NOTE: You may need your Apple ID for SDK download"
echo "If prompted, enter your Apple Developer credentials"
echo ""
read -p "Continue? (y/n): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    exit 1
fi

./tools/getSdk.py

echo "Building osxcross..."
unset MACOSX_DEPLOYMENT_TARGET
OCPATH=$(pwd) ./build.sh

echo ""
echo "=== Setup complete! ==="
echo ""
echo "Add these to your ~/.bashrc or ~/.zshrc:"
echo ""
echo "  export OSXCROSS_PATH=\"$OSXCROSS_DIR\""
echo "  export PATH=\"\$OSXCROSS_PATH/bin:\$PATH\""
echo ""
echo "Then run:"
echo "  source ~/.bashrc  # or ~/.zshrc"
echo "  cd tests/blue_screen/macos"
echo "  ./build.sh"