cmake_minimum_required(VERSION 3.16)
project(TestRenderMacOS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/sdl")

find_package(SDL3 QUIET PATHS "${SDL_DIR}" NO_DEFAULT_PATH)

set(OSXCROSS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/osxcross")
set(OSXCROSS_SDK "${OSXCROSS_DIR}/target/SDK/MacOSX15.5.sdk")

add_executable(test_render_macos ${CMAKE_CURRENT_SOURCE_DIR}/../mac/TestRender/test_render.cpp)

target_include_directories(test_render_macos PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/SDL"
    "${SDL_DIR}/include"
)

target_link_libraries(test_render_macos PRIVATE 
    "-framework Cocoa"
    "-framework CoreVideo"
    "-framework AudioToolbox"
)

set_target_properties(test_render_macos PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build"
)
