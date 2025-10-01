cmake_minimum_required(VERSION 3.16)

project(AmiusAdventurePC VERSION 0.2.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_definitions("DEBUG=$<CONFIG:Debug>")

add_subdirectory(../amius_adventure amius_adventure_engine)

include_directories(${PROJECT_SOURCE_DIR}/../include ${PROJECT_SOURCE_DIR}/../SDL/include ${PROJECT_SOURCE_DIR}/../SDL_mixer/include ${PROJECT_SOURCE_DIR}/../SDL_image/include ${PROJECT_SOURCE_DIR}/../fastgltf/include)

add_subdirectory(../SDL EXCLUDE_FROM_ALL sdl)
add_subdirectory(../SDL_mixer EXCLUDE_FROM_ALL sdl_mixer)
add_subdirectory(../SDL_image EXCLUDE_FROM_ALL sdl_image)
add_subdirectory(../fastgltf EXCLUDE_FROM_ALL fastgltf)

if (APPLE) 
find_package(Vulkan REQUIRED COMPONENTS MoltenVK)
else()
find_package(Vulkan REQUIRED)
endif()
find_package(glm CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

add_subdirectory(src)