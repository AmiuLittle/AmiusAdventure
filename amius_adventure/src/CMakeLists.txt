file(GLOB CPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(AmiusAdventure STATIC ${CPP_FILES})

target_include_directories(AmiusAdventure PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)