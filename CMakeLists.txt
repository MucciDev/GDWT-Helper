cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(GDWTHelper VERSION 1.1.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")

if(NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK environment variable is not defined")
endif()

file(GLOB_RECURSE SOURCES src/*.cpp)

add_library(${PROJECT_NAME} SHARED
    ${SOURCES}
)

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME})