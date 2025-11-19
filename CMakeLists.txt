cmake_minimum_required(VERSION 3.20)
project(Nilou VERSION 0.1.0 LANGUAGES C CXX)

# 引入自定义 CMake 函数/宏
include(cmake/Utils.cmake)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

    # 启用多线程编译
    add_compile_options(/MP /EHsc)
endif()

if(WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

set(VK_SDK_PATH "$ENV{VK_SDK_PATH}" CACHE PATH "Path to Vulkan SDK")

# stb
find_package(Stb REQUIRED)

# magic_enum
find_package(magic_enum CONFIG REQUIRED)
set(_magic_enum_target)
if(TARGET magic_enum::magic_enum)
    set(_magic_enum_target magic_enum::magic_enum)
elseif(TARGET magic_enum)
    set(_magic_enum_target magic_enum)
endif()

# fmt
find_package(fmt CONFIG)
set(_fmt_target )
if(TARGET fmt::fmt)
    set(_fmt_target fmt::fmt)
elseif(TARGET fmt)
    set(_fmt_target fmt)
endif()

# nlohmann_json
find_package(nlohmann_json CONFIG REQUIRED)
set(_nlohmann_json_target)
if(TARGET nlohmann_json::nlohmann_json)
    set(_nlohmann_json_target nlohmann_json::nlohmann_json)
elseif(TARGET nlohmann_json)
    set(_nlohmann_json_target nlohmann_json)
endif()

# crossguid
find_package(crossguid CONFIG REQUIRED)
set(_crossguid_target)
if(TARGET crossguid::crossguid)
    set(_crossguid_target crossguid::crossguid)
elseif(TARGET crossguid)
    set(_crossguid_target crossguid)
endif()

# glm
find_package(glm CONFIG REQUIRED)
set(_glm_target)
if(TARGET glm::glm)
    set(_glm_target glm::glm)
elseif(TARGET glm)
    set(_glm_target glm)
endif()

# glfw3
find_package(glfw3 CONFIG)
set(_glfw_target )
if(TARGET glfw)
    set(_glfw_target glfw)
elseif(TARGET glfw3)
    set(_glfw_target glfw3)
elseif(TARGET glfw3::glfw)
    set(_glfw_target glfw3::glfw)
endif()

# imgui
find_package(imgui CONFIG)
set(_imgui_target )
if(TARGET imgui::imgui)
    set(_imgui_target imgui::imgui)
elseif(TARGET imgui)
    set(_imgui_target imgui)
endif()

# tiny_gltf
find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")

# half.hpp
find_path(HALF_INCLUDE_DIRS "half.hpp")

# glad
find_package(glad CONFIG REQUIRED)
set(_glad_target)
if(TARGET glad::glad)
    set(_glad_target glad::glad)
elseif(TARGET glad)
    set(_glad_target glad)
endif()

# spirv-reflect
find_package(unofficial-spirv-reflect CONFIG REQUIRED)
if(TARGET unofficial::spirv-reflect AND NOT _spirv_reflect_target)
    set(_spirv_reflect_target unofficial::spirv-reflect)
endif()

# base64
add_subdirectory(External/base64)

# microprofile
add_subdirectory(External/microprofile)

# UDRefl
# 关闭其示例和测试
set(Ubpa_BuildTest_UDRefl OFF CACHE BOOL "" FORCE)
set(Ubpa_UDRefl_Build_ext_Bootstrap OFF CACHE BOOL "" FORCE)
set(Ubpa_BuildTest_UTemplate OFF CACHE BOOL "" FORCE)
set(Ubpa_BuildTest_USmallFlat OFF CACHE BOOL "" FORCE)
set(Ubpa_BuildTest_UDRefl OFF CACHE BOOL "" FORCE)
set(Ubpa_UTemplate_Build_Example OFF CACHE BOOL "" FORCE)
set(Ubpa_USmallFlat_Build_Example OFF CACHE BOOL "" FORCE)
set(Ubpa_UDRefl_Build_ext_Bootstrap OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/External/UDRefl UDRefl-build)

# 添加 thread_pool 依赖库（作为源码子项目）===
# thread-pool没有 CMakeLists.txt ，因此，直接设置头文件目录即可
# add_subdirectory(${CMAKE_SOURCE_DIR}/External/thread-pool thread-pool)

# VulkanSDK
add_subdirectory(External/vulkan)

# 检查所有必需的第三方库 target 是否已正确设置
check_required_targets(
    TARGETS
        _magic_enum_target
        _fmt_target
        _nlohmann_json_target
        _glm_target
        _glfw_target
        _imgui_target
        _spirv_reflect_target
        _glad_target
        _crossguid_target
)

# Nilou
add_subdirectory(src/Runtime)

# NilouHeaderTool
add_subdirectory(src/Programs/NilouHeaderTool)

# test program，test_*[.exe]
add_subdirectory(src/misc) 
