file(GLOB CPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB UI_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.ui")
qt_add_executable(3dsSpriteMaker ${CPP_FILES} ${UI_FILES})
qt_add_ui(3dsSpriteMaker SOURCES ${UI_FILES} INCLUDE_PREFIX .)

target_link_libraries(3dsSpriteMaker PRIVATE Qt6::Widgets)

set_target_properties(3dsSpriteMaker PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)