#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "=== Running game_2048 ==="
cd "$SCRIPT_DIR/linux/build"
./game_2048
