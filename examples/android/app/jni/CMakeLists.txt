cmake_minimum_required(VERSION 3.6)

project(GAME)

# Use pre-built SDL3 from libs folder (jniLibs)
# Gradle copies libs/ to APK automatically
set(SDL3_SO_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../libs/${ANDROID_ABI}/libSDL3.so")

# Check for pre-built or fall back to SDL source
if(EXISTS ${SDL3_SO_PATH})
    message(STATUS "=== Using pre-built SDL3: ${SDL3_SO_PATH} ===")
    
    add_library(SDL3 SHARED IMPORTED)
    set_target_properties(SDL3 PROPERTIES
        IMPORTED_LOCATION "${SDL3_SO_PATH}"
    )
    add_library(SDL3::SDL3 ALIAS SDL3)
    
    # SDL symlink is in jni/SDL
    set(SDL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SDL/include")
    target_include_directories(SDL3 INTERFACE ${SDL_INCLUDE_DIR})
else()
    message(STATUS "=== Building SDL3 from source ===")
    add_subdirectory(SDL)
endif()

add_subdirectory(src)
