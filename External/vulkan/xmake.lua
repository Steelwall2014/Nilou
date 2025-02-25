
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

target("VulkanSDK")
    set_kind("headeronly")
    local VulkanSDK = find_vulkan_sdk()
    add_includedirs(VulkanSDK .. "/Include", {public = true})
    -- add_includedirs(VulkanSDK .. "/Source/SPIRV-Reflect", {public = true})
    add_links(VulkanSDK .. "/Lib/vulkan-1", {public = true})
    add_links(VulkanSDK .. "/Lib/shaderc*", {public = true})
    add_links(VulkanSDK .. "/Lib/glslang", {public = true})
