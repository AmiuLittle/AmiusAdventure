cmake_minimum_required(VERSION 3.16)

# Set project name and version
project(AmiusAdventure VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(glm CONFIG REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/../include)

add_subdirectory(src)
