cmake_minimum_required(VERSION 3.16)
project(TestWebGPU)

set(SDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/sdl")
set(WGPU_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/wgpu-native")

find_package(SDL3 QUIET PATHS "${SDL_DIR}" NO_DEFAULT_PATH)

add_executable(test_webgpu ${CMAKE_CURRENT_SOURCE_DIR}/test_webgpu.c)

target_include_directories(test_webgpu PRIVATE 
    "${SDL_DIR}/include"
    "${WGPU_DIR}/ffi/webgpu-headers"
    "${WGPU_DIR}/ffi"
)

target_sources(test_webgpu PRIVATE 
    "${SDL_DIR}/sdl3webgpu.c"
)

target_link_libraries(test_webgpu PRIVATE 
    "${SDL_DIR}/build/linux/libSDL3.so"
    "${WGPU_DIR}/target/release/libwgpu_native.a"
    -lpthread -ldl -lEGL -lvulkan -lglfw -lwayland-client -lwayland-cursor -lxkbcommon -lz -lm
)

set_target_properties(test_webgpu PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build"
)
