cmake_minimum_required(VERSION 3.16)

project(3dsSpriteMaker VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (WIN32)
    set(CMAKE_PREFIX_PATH /usr/x86_64-w64-mingw32)
    include_directories(/usr/x86_64-w64-mingw32/include)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

add_subdirectory(src)
