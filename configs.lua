local Configs = {}
Configs.PROJECT_DIR = "$(projectdir)"

function find_vulkan_sdk()
    local VulkanSDK = "$(env VK_SDK_PATH)"
    if VulkanSDK == "" then
        VulkanSDK = "$(env VULKAN_SDK)"
    end
    if VulkanSDK == "" then
        error("Vulkan SDK path not found. Please set VK_SDK_PATH or VULKAN_SDK environment variable.")
    end
    return VulkanSDK
end
local VulkanSDK = find_vulkan_sdk()

Configs.VULKAN_INCLUDE = VulkanSDK .. "\\Include"
Configs.VULKAN_LIBRARY = {VulkanSDK .. "\\Lib\\vulkan-1", 
                          VulkanSDK .. "\\Lib\\shaderc*",
                          VulkanSDK .. "\\Lib\\glslang"}

Configs.INCLUDE_PATHS = {
    "$(projectdir)\\External\\include", 
    "$(projectdir)\\src\\Runtime\\Framework", 
    "$(projectdir)\\src\\Runtime\\Applications", 
    "$(projectdir)\\src\\Runtime\\RHI", 
    "$(projectdir)\\src\\Runtime\\HAL", 
    "$(projectdir)\\src\\Runtime\\Rendering", 
    "$(projectdir)\\src\\Runtime\\GameStatics",
    "$(projectdir)\\src\\Runtime\\Generated",
    "$(projectdir)\\src\\Runtime\\Serialization",
    "$(projectdir)\\src\\Runtime\\RenderPass",
    "$(projectdir)\\src\\Runtime\\Geospatial",
    "$(projectdir)\\src\\Runtime\\Cesium3DTiles",
    "$(projectdir)\\src\\Runtime\\RenderGraph",
    "$(projectdir)\\Assets\\Shaders",
    Configs.VULKAN_INCLUDE,
    VulkanSDK .. "\\Source\\SPIRV-Reflect"}

function main()
    return Configs
end
function GetConfigs()
    return Configs
end
return Configs;