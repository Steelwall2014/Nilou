# src/Runtime/CMakeLists.txt

include(${CMAKE_SOURCE_DIR}/cmake/SourceTree.cmake)

# 收集源文件（排除 .gen.cpp）
file(GLOB_RECURSE NILLOU_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
)
file(GLOB_RECURSE NILOU_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
list(FILTER NILOU_SOURCES EXCLUDE REGEX ".*\\.gen\\.cpp$")

# Nilou
add_executable(Nilou ${NILOU_SOURCES})

target_compile_definitions(Nilou PRIVATE FMT_USE_NONTYPE_TEMPLATE_ARGS=0)
target_compile_definitions(Nilou PRIVATE PROJECT_DIR="${CMAKE_SOURCE_DIR}")
target_compile_definitions(Nilou PRIVATE $<$<CONFIG:Debug>:NILOU_DEBUG>)

if(MSVC)
    target_compile_options(Nilou PRIVATE /bigobj /EHsc)
endif()

# 链接所有依赖（变量来自父作用域）
target_link_libraries(Nilou PRIVATE
    ${_fmt_target}
    ${_glfw_target}
    ${_imgui_target}
    ${_glm_target}
    ${_glad_target}
    ${_spirv_reflect_target}
    ${_crossguid_target}
    base64
    microprofile
    UDRefl_core
    VulkanSDK
)

# 包含头文件
target_include_directories(Nilou PRIVATE
    ${HALF_INCLUDE_DIRS}
    ${Stb_INCLUDE_DIR}
    ${TINYGLTF_INCLUDE_DIRS}
    #
    ${CMAKE_SOURCE_DIR}/External            # 只是用于能找到 thread-pool 的头文件
    ${CMAKE_SOURCE_DIR}/External/include
    #
    ${CMAKE_CURRENT_SOURCE_DIR}/Framework
    ${CMAKE_CURRENT_SOURCE_DIR}/Applications
    ${CMAKE_CURRENT_SOURCE_DIR}/RHI
    ${CMAKE_CURRENT_SOURCE_DIR}/HAL
    ${CMAKE_CURRENT_SOURCE_DIR}/Rendering
    ${CMAKE_CURRENT_SOURCE_DIR}/GameStatics
    ${CMAKE_CURRENT_SOURCE_DIR}/Generated
    ${CMAKE_CURRENT_SOURCE_DIR}/Serialization
    ${CMAKE_CURRENT_SOURCE_DIR}/RenderPass
    ${CMAKE_CURRENT_SOURCE_DIR}/Geospatial
    ${CMAKE_CURRENT_SOURCE_DIR}/Cesium3DTiles
    ${CMAKE_CURRENT_SOURCE_DIR}/RenderGraph
)

# ImGui backends
find_path(IMGUI_BACKENDS_INCLUDE_DIR imgui_impl_glfw.h
    PATH_SUFFIXES imgui/backends backends)
if(IMGUI_BACKENDS_INCLUDE_DIR)
    target_include_directories(Nilou PRIVATE ${IMGUI_BACKENDS_INCLUDE_DIR})
endif()

if(WIN32)
    target_link_libraries(Nilou PRIVATE kernel32 User32 Gdi32 Shell32 Opengl32)
endif()

# 自动生成代码：依赖 NilouHeaderTool
add_custom_command(TARGET Nilou PRE_BUILD
    COMMAND $<TARGET_FILE:NilouHeaderTool>
        -InputDirectory="${CMAKE_SOURCE_DIR}/src"
        -OutputDirectory="${CMAKE_SOURCE_DIR}/src/Runtime/Generated"
        -x c++ -std=c++20
        -I "${CMAKE_SOURCE_DIR}/External/include"
        -I "${CMAKE_SOURCE_DIR}/src/Runtime/Framework"
        -I "${CMAKE_SOURCE_DIR}/src/Runtime/Applications"
        -I "${CMAKE_SOURCE_DIR}/src/Runtime/RHI"
        -I "${CMAKE_SOURCE_DIR}/src/Runtime/HAL"
        -I "${CMAKE_SOURCE_DIR}/src/Runtime/Rendering"
        -I "${CMAKE_SOURCE_DIR}/src/Runtime/GameStatics"
        -I "${CMAKE_SOURCE_DIR}/src/Runtime/Serialization"
        -I "${CMAKE_SOURCE_DIR}/src/Runtime/RenderPass"
        -I "${CMAKE_SOURCE_DIR}/src/Runtime/Geospatial"
        -I "${CMAKE_SOURCE_DIR}/src/Runtime/Cesium3DTiles"
        -I "${CMAKE_SOURCE_DIR}/src/Runtime/RenderGraph"
)

# 添加生成的 .gen.cpp 文件
file(GLOB NILOU_GEN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Generated/*.gen.cpp)
if(NILOU_GEN_SOURCES)
    target_sources(Nilou PRIVATE ${NILOU_GEN_SOURCES})
endif()


auto_dynamic_source_group(src ${CMAKE_SOURCE_DIR}/src ${NILOU_GEN_SOURCES}) # 动态分组，确保 gen.cpp 也在 src 目录下

# 添加头文件到目标（不影响编译，只影响 IDE 显示）
target_sources(Nilou PRIVATE ${NILLOU_HEADERS})

# 分组（.h 和 .cpp）
set(NILOU_ALL_SOURCES ${NILLOU_HEADERS} ${NILOU_SOURCES})
auto_source_group(src ${CMAKE_SOURCE_DIR}/src ${NILOU_ALL_SOURCES})

# 动态分组 .gen.cpp（FILES 方式，支持不存在的文件）
file(GLOB NILOU_GEN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Generated/*.gen.cpp)
auto_dynamic_source_group(src ${CMAKE_SOURCE_DIR}/src ${NILOU_GEN_SOURCES})


# 复制 bin（如 DLL）
add_custom_command(TARGET Nilou POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/External/bin"
        "$<TARGET_FILE_DIR:Nilou>"
)