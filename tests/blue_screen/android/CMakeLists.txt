cmake_minimum_required(VERSION 3.18)

project(TestRenderAndroid)

set(SDL_SOURCE_DIR "" CACHE PATH "Path to SDL source directory")

if(NOT SDL_SOURCE_DIR)
    message(FATAL_ERROR "SDL_SOURCE_DIR must be set")
endif()

set(SDL_INCLUDE_DIR "${SDL_SOURCE_DIR}/include")
set(SDL_DIR "${SDL_SOURCE_DIR}")

add_executable(test_render ${CMAKE_CURRENT_SOURCE_DIR}/../test_render.c)

target_include_directories(test_render PRIVATE ${SDL_INCLUDE_DIR})

add_subdirectory(${SDL_DIR} sdl-build EXCLUDE_FROM_ALL)

target_link_libraries(test_render PRIVATE SDL3::SDL3)
