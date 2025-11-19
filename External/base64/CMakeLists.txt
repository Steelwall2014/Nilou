# External/base64/CMakeLists.txt

add_library(base64 STATIC base64.cpp)

target_include_directories(base64 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)