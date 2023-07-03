local Configs = {}
Configs.PROJECT_DIR = "D:/Nilou"
Configs.VULKAN_INCLUDE = "E:/VulkanSDK/1.3.246.1/Include"
Configs.VULKAN_LIBRARY = {"E:/VulkanSDK/1.3.246.1/Lib/vulkan-1", 
                          "E:/VulkanSDK/1.3.246.1/Lib/shaderc*",
                          "E:/VulkanSDK/1.3.246.1/Lib/glslang"}

Configs.INCLUDE_PATHS = {
    "./External/include", 
    "./src/Runtime/Framework", 
    "./src/Runtime/Applications", 
    "./src/Runtime/RHI", 
    "./src/Runtime/HAL", 
    "./src/Runtime/Rendering", 
    "./src/Runtime/GameStatics",
    "./src/Runtime/Generated",
    "./src/Runtime/Serialization",
    "./src/Runtime/RenderPass",
    "./src/Runtime/Geospatial",
    "./src/Runtime/Cesium3DTiles",
    "./Assets/Shaders",
    Configs.VULKAN_INCLUDE}

function main()
    return Configs
end
function GetConfigs()
    return Configs
end
return Configs;