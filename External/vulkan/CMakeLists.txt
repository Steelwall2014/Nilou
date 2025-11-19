# External/vulkan/CMakeLists.txt

# 创建 INTERFACE 库来封装 Vulkan SDK
add_library(VulkanSDK INTERFACE)

# 检查 VK_SDK_PATH 是否已设置
if(NOT VK_SDK_PATH)
    message(FATAL_ERROR "VK_SDK_PATH is not set. Please install LunarG Vulkan SDK and set VK_SDK_PATH.")
endif()

# 添加头文件路径（⚠️ 风险：包含 glm/ 和 SDL2/，建议安装 Vulkan SDK 时不要勾选 GLM 和 SDL2组件的安装选项）
# 建议只包含 vulkan 子目录（见下方“更安全做法”）
target_include_directories(VulkanSDK INTERFACE ${VK_SDK_PATH}/Include)

# 链接库目录和核心库
target_link_directories(VulkanSDK INTERFACE ${VK_SDK_PATH}/Lib)
target_link_libraries(VulkanSDK INTERFACE vulkan-1)

# Shaderc 支持（仅 Windows）, 新版 Vulkan SDK：只需 shaderc_combined
if(WIN32)
    target_link_libraries(VulkanSDK INTERFACE
        $<$<CONFIG:Debug>:shaderc_combinedd>
        $<$<NOT:$<CONFIG:Debug>>:shaderc_combined>
    )
endif()