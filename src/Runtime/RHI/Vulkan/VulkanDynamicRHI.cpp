#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanDynamicRHI.h"
#include "BaseApplication.h"
#include "VulkanShader.h"
#include "PipelineStateCache.h"

namespace nilou {

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        // Message is important enough to show
    }

    return VK_FALSE;
}

FVulkanDynamicRHI::QueueFamilyIndices FVulkanDynamicRHI::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}


FVulkanDynamicRHI::SwapChainSupportDetails FVulkanDynamicRHI::querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32 formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32 presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

bool FVulkanDynamicRHI::isDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);
    bool swapChainAdequate = false;
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    return indices.graphicsFamily.has_value() && swapChainAdequate;
}

int FVulkanDynamicRHI::Initialize()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    char AppName[256];
    sprintf(AppName, "%ws", GetAppication()->GetConfiguration().appName);
    appInfo.pApplicationName = AppName;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    std::vector<const char*> extensions = {
        "VK_KHR_surface",
        "VK_KHR_win32_surface",
        "VK_EXT_debug_utils"
    };

    std::vector<const char*> layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = layers.size();
    createInfo.ppEnabledLayerNames = layers.data();

    /** Setup debug messenger */
    {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugCallback;

        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
        if (result != VK_SUCCESS) {
            NILOU_LOG(Error, "RHI load failed!")
            return 1;
        }
        if (CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            NILOU_LOG(Error, "failed to set up debug messenger!")
            return 1;
        }
    }

    /** Create window surface */
    {
        GLFWwindow* window = reinterpret_cast<GLFWwindow*>(GetAppication()->GetWindowContext());
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            NILOU_LOG(Error, "failed to create window surface!")
            return 1;
        }
    }

    /** Pick physical device */
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            NILOU_LOG(Error, "failed to find GPUs with Vulkan support!")
            return 1;
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            NILOU_LOG(Error, "failed to find a suitable GPU!")
            return 1;
        }
    }

    /** Create logical device */
    {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        std::vector<const char*> deviceExtensions = {
            "VK_KHR_swapchain"
        };
        VkPhysicalDeviceFeatures deviceFeatures{};
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.enabledExtensionCount = deviceExtensions.size();
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        createInfo.ppEnabledLayerNames = layers.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            NILOU_LOG(Error, "failed to create logical device!")
            return 1;
        }
    }

    /** Create swap chain */
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
        VkPresentModeKHR presentMode = VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR;
        VkExtent2D extent{GetAppication()->GetConfiguration().screenWidth, GetAppication()->GetConfiguration().screenHeight};
        uint32 imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
        createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32 queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            NILOU_LOG(Error, "failed to create swap chain!")
            return 1;
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = VK_FORMAT_B8G8R8A8_SRGB;
        swapChainExtent = extent;
    }

    /** Create image views */
    {
        swapChainImageViews.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                NILOU_LOG(Error, "failed to create image views!")
                return 1;
            }
        }
    }

    shader_compiler = shaderc_compiler_initialize();

    return 0;
}

void FVulkanDynamicRHI::Finalize()
{
    shaderc_compiler_release(shader_compiler);
    
    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);
    vkDestroyDevice(device, nullptr);

    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    FDynamicRHI::Finalize();
}

void FVulkanDynamicRHI::GetError(const char *file, int line)
{
    
}

RHIVertexShaderRef FVulkanDynamicRHI::RHICreateVertexShader(const std::string& code)
{
    auto [Module, result] = 
        RHICompileShaderInternal(code, shaderc_vertex_shader);

    if (Module && result)
    {
        VulkanVertexShaderRef VulkanShader = std::make_shared<VulkanVertexShader>();
        VulkanShader->Module = Module;
        VulkanShader->ShadercResult = result;
        return VulkanShader;
    }

    NILOU_LOG(Error, "failed to create vertex shader!");
    return nullptr;
}

RHIPixelShaderRef FVulkanDynamicRHI::RHICreatePixelShader(const std::string& code)
{
    auto [Module, result] = 
        RHICompileShaderInternal(code, shaderc_fragment_shader);

    if (Module && result)
    {
        VulkanPixelShaderRef VulkanShader = std::make_shared<VulkanPixelShader>();
        VulkanShader->Module = Module;
        VulkanShader->ShadercResult = result;
        return VulkanShader;
    }

    NILOU_LOG(Error, "failed to create pixel shader!");
    return nullptr;
}

RHIComputeShaderRef FVulkanDynamicRHI::RHICreateComputeShader(const std::string& code)
{
    auto [Module, result] = 
        RHICompileShaderInternal(code, shaderc_compute_shader);

    if (Module && result)
    {
        VulkanComputeShaderRef VulkanShader = std::make_shared<VulkanComputeShader>();
        VulkanShader->Module = Module;
        VulkanShader->ShadercResult = result;
        return VulkanShader;
    }

    NILOU_LOG(Error, "failed to create compute shader!");
    return nullptr;
}

void FVulkanDynamicRHI::RHIDestroyShader(RHIShader* Shader)
{
    VkShaderModule Module = VK_NULL_HANDLE;
    shaderc_compilation_result_t Result = nullptr;
    if (Shader->ResourceType == ERHIResourceType::RRT_VertexShader)
    {
        VulkanVertexShader* VulkanShader = reinterpret_cast<VulkanVertexShader*>(Shader);
        Module = VulkanShader->Module;
        Result = VulkanShader->ShadercResult;
    }
    else if (Shader->ResourceType == ERHIResourceType::RRT_PixelShader)
    {
        VulkanPixelShader* VulkanShader = reinterpret_cast<VulkanPixelShader*>(Shader);
        Module = VulkanShader->Module;
        Result = VulkanShader->ShadercResult;
    }
    else if (Shader->ResourceType == ERHIResourceType::RRT_ComputeShader)
    {
        VulkanComputeShader* VulkanShader = reinterpret_cast<VulkanComputeShader*>(Shader);
        Module = VulkanShader->Module;
        Result = VulkanShader->ShadercResult;
    }
    if (Module != VK_NULL_HANDLE)
        vkDestroyShaderModule(device, Module, nullptr);
    if (Result != nullptr)
        shaderc_result_release(Result);
}

std::pair<VkShaderModule, shaderc_compilation_result_t> 
FVulkanDynamicRHI::RHICompileShaderInternal(const std::string& code, shaderc_shader_kind shader_kind)
{
    shaderc_compilation_result_t result = shaderc_compile_into_spv(shader_compiler, 
        code.c_str(), code.size(), shaderc_vertex_shader, 
        "", "main", nullptr);
    shaderc_compilation_status status = shaderc_result_get_compilation_status(result);
    if (status != shaderc_compilation_status_success) {
        const char* msg = shaderc_result_get_error_message(result);
        return {};
    }

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderc_result_get_length(result);
    createInfo.pCode = reinterpret_cast<const uint32*>(shaderc_result_get_bytes(result));
    VkShaderModule Module{};
    if (vkCreateShaderModule(device, &createInfo, nullptr, &Module) != VK_SUCCESS) {
        return {};
    }

    return { Module, result };

}

FRHIGraphicsPipelineState *FVulkanDynamicRHI::RHIGetOrCreatePipelineStateObject(const FGraphicsPipelineStateInitializer &Initializer)
{
    FRHIGraphicsPipelineState* CachedPSO = FPipelineStateCache::FindCachedGraphicsPSO(Initializer);
    if (CachedPSO)
        return CachedPSO;

    
}

}