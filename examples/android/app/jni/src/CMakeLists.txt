cmake_minimum_required(VERSION 3.6)

project(my_app)

# SDL3 should be defined in parent CMakeLists.txt
# Create alias if needed
if(NOT TARGET SDL3::SDL3)
    if(TARGET SDL3)
        add_library(SDL3::SDL3 ALIAS SDL3)
    endif()
endif()

# Include directories - use SDL symlink or third_party
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../SDL/include")
    set(SDL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../SDL/include")
else()
    set(SDL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/sdl/src/include")
endif()

add_library(main SHARED
    YourSourceHere.c
)

target_include_directories(main PRIVATE ${SDL_INCLUDE_DIR})
target_link_libraries(main PRIVATE SDL3::SDL3)
