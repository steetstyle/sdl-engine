cmake_minimum_required(VERSION 3.16)

project(TestRenderMacOS)

set(SDL_DIR "" CACHE PATH "Path to SDL third_party directory")
set(TEST_SOURCE_DIR "" CACHE PATH "Path to tests directory")

if(NOT SDL_DIR)
    message(FATAL_ERROR "SDL_DIR must be set")
endif()

if(NOT TEST_SOURCE_DIR)
    set(TEST_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")
endif()

set(SDL_SOURCE_DIR "${SDL_DIR}/src")
set(SDL_INCLUDE_DIR "${SDL_DIR}/src/include")
set(SDL_LIB_DIR "${SDL_DIR}/macos/lib/universal")

add_executable(test_render_macos ${TEST_SOURCE_DIR}/test_render.c)

target_include_directories(test_render_macos PRIVATE ${SDL_INCLUDE_DIR})

target_link_libraries(test_render_macos PRIVATE 
    ${SDL_LIB_DIR}/libSDL3.dylib
)
