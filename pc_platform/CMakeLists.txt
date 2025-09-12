cmake_minimum_required(VERSION 3.16)

project(AmiusAdventurePC VERSION 0.2.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_definitions("DEBUG=$<CONFIG:Debug>")

add_subdirectory(../amius_adventure amius_adventure_engine)

add_subdirectory(../SDL EXCLUDE_FROM_ALL sdl)
add_subdirectory(../SDL_mixer EXCLUDE_FROM_ALL sdl_mixer)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/../include)

if (APPLE) 
    find_package(Vulkan REQUIRED COMPONENTS MoltenVK)
else()
    find_package(Vulkan REQUIRED)
endif()
find_package(glm CONFIG REQUIRED)
find_package(SDL3_image REQUIRED)

add_subdirectory(src)