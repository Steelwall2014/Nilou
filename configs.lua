local Configs = {}
Configs.PROJECT_DIR = "E:/Nilou"
Configs.VULKAN_INCLUDE = "D:/VulkanSDK/1.3.250.0/Include"
Configs.VULKAN_LIBRARY = {"D:/VulkanSDK/1.3.250.0/Lib/vulkan-1", 
                          "D:/VulkanSDK/1.3.250.0/Lib/shaderc*",
                          "D:/VulkanSDK/1.3.250.0/Lib/glslang"}

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