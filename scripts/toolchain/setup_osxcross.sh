#!/bin/bash
# osxcross setup script for cross-compiling to macOS
# Run this on Linux to set up the cross-compilation toolchain

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OSXCROSS_DIR="$SCRIPT_DIR/../../third_party/osxcross"

echo "=== Setting up osxcross for macOS cross-compilation ==="

# Check for macOS SDK in tarballs directory
SDK_TARBALL=""
for f in "$OSXCROSS_DIR"/tarballs/*.tar.*; do
    if [ -f "$f" ]; then
        SDK_TARBALL="$f"
        break
    fi
done

if [ -z "$SDK_TARBALL" ]; then
    echo "Downloading macOS SDK..."
    echo "Available SDKs: https://github.com/joseluisq/macosx-sdks/releases"
    echo "Download a pre-packaged SDK and place it in $OSXCROSS_DIR/tarballs/"
    exit 1
fi

echo "Found SDK: $SDK_TARBALL"

if [ -d "$OSXCROSS_DIR/target" ]; then
    echo "osxcross already built at $OSXCROSS_DIR/target"
    read -p "Rebuild? (y/n): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 0
    fi
    rm -rf "$OSXCROSS_DIR/target" "$OSXCROSS_DIR/build"
fi

cd "$OSXCROSS_DIR"

echo "Building osxcross..."
unset MACOSX_DEPLOYMENT_TARGET
echo "" | UNATTENDED=1 OCPATH=$(pwd) ./build.sh

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
echo "  x86_64-apple-darwin24.5-clang++ --version"