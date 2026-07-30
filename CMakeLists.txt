cmake_minimum_required(VERSION 3.10)
project(shootoid)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.hpp src/*.h)

option(SDL_VENDORED "Use vendored SDL" OFF)

if (SDL_VENDORED)
    add_subdirectory(vendor/SDL EXCLUDE_FROM_ALL)
else()
    # 1. Look for a SDL3 package
    # 2. Look for the SDL3 shared component
    # 3. Fail if the shared component cannot be found
    find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
endif()

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
