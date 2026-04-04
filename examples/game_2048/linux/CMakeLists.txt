cmake_minimum_required(VERSION 3.18)

project(Game2048Linux)

set(SDL_SOURCE_DIR "" CACHE PATH "Path to SDL source directory")
set(SDL_TTF_SOURCE_DIR "" CACHE PATH "Path to SDL_ttf source directory")

if(NOT SDL_SOURCE_DIR)
    message(FATAL_ERROR "SDL_SOURCE_DIR must be set")
endif()

set(SDL_INCLUDE_DIR "${SDL_SOURCE_DIR}/include")
set(SDL_TTF_INCLUDE_DIR "${SDL_TTF_SOURCE_DIR}/include")
set(SDL_DIR "${SDL_SOURCE_DIR}")
set(SDL_TTF_DIR "${SDL_TTF_SOURCE_DIR}")

add_executable(game_2048 ${CMAKE_CURRENT_SOURCE_DIR}/../game_2048.c)

target_include_directories(game_2048 PRIVATE ${SDL_INCLUDE_DIR})
target_include_directories(game_2048 PRIVATE ${SDL_TTF_INCLUDE_DIR})

add_subdirectory(${SDL_DIR} sdl-build EXCLUDE_FROM_ALL)
add_subdirectory(${SDL_TTF_DIR} sdl-ttf-build EXCLUDE_FROM_ALL)

target_link_libraries(game_2048 PRIVATE SDL3::SDL3 SDL3_ttf::SDL3_ttf)
