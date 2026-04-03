# iOS Test Build

iOS builds require a macOS machine with Xcode and iOS SDK.

## Requirements

- macOS machine
- Xcode 12.2+
- iOS 14.2+ SDK

## Building

On macOS, use the provided Xcode project or build via command line:

```bash
# Using the SDL xcframework
# Add test_render.c to your Xcode project with SDL3.xcframework linked

# Or via command line (on macOS only)
clang -o test_render_ios test_render.c \
    -framework SDL3 \
    -isysroot $(xcrun --sdk iphoneos --show-sdk-path) \
    -target arm64-apple-ios14.0
```

## Testing

- Run on iOS Simulator
- Run on physical device (requires signing certificate)
