cmake_minimum_required(VERSION 3.20)
project(TestRender)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")
set(CMAKE_OSX_SYSROOT "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX15.4.sdk")

set(SDL3_DIR "${CMAKE_SOURCE_DIR}/../../third_party/sdl")
set(SDL3_BUILD "${SDL3_DIR}/build/macos")

find_library(SDL3_LIBRARY NAMES SDL3 PATHS "${SDL3_BUILD}" NO_DEFAULT_PATH)

add_executable(test_render MACOSX_BUNDLE test_render.cpp)

target_include_directories(test_render PRIVATE 
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/../SDL"
)

target_link_libraries(test_render 
    "/Users/furkanbostanci/Sites/sdl-engine/third_party/sdl/build/macos/libSDL3.dylib"
    "-framework Cocoa"
    "-framework CoreVideo"
    "-framework AudioToolbox"
)

set_target_properties(test_render PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "TestRender"
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.sdl.testrender"
    PRODUCT_VERSION "1.0"
    MARKETING_VERSION "1.0"
)
