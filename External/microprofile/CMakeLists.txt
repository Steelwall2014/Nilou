# External/microprofile/CMakeLists.txt

# 定义静态库
add_library(microprofile STATIC microprofile.cpp)

# 设置编译定义
target_compile_definitions(microprofile PRIVATE MICROPROFILE_SYSTEM_STB)

# 链接 stb（假设 stb 已通过 find_package 或 add_subdirectory 引入）
# 注意：这里使用 IMPORTED target 或已存在的 target 名称
target_link_libraries(microprofile PRIVATE ${_stb_target})

# target_include_directories(microprofile PRIVATE
#     ${Stb_INCLUDE_DIR}
# )

# 设置 PUBLIC include 目录（供使用者包含头文件）
target_include_directories(microprofile PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    ${Stb_INCLUDE_DIR}
    $<INSTALL_INTERFACE:include>
)