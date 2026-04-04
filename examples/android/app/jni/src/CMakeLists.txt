cmake_minimum_required(VERSION 3.6)

project(my_app)

add_library(main SHARED
    merge_and_devour.c
)

target_include_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../SDL)
target_include_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../SDL_ttf)
target_link_libraries(main PRIVATE SDL3::SDL3)
target_link_libraries(main PRIVATE SDL3_ttf::SDL3_ttf)
