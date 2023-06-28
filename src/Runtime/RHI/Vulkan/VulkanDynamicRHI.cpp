#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanDynamicRHI.h"
#include "BaseApplication.h"
#include "VulkanShader.h"
#include "VulkanResources.h"
#include "VulkanTexture.h"
#include "PipelineStateCache.h"
#include "VulkanCommandBuffer.h"
#include "VulkanMemory.h"
#include "VulkanBuffer.h"
#include "VulkanFramebuffer.h"

namespace nilou {

static VkFormat TranslateVertexElementTypeToVKFormat(EVertexElementType ElementType)
{
    switch (ElementType) 
    {
	case EVertexElementType::VET_Float1:
		return VK_FORMAT_R32_SFLOAT;
	case EVertexElementType::VET_Float2:
		return VK_FORMAT_R32G32_SFLOAT;
	case EVertexElementType::VET_Float3:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case EVertexElementType::VET_Float4:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case EVertexElementType::VET_Half2:
		return VK_FORMAT_R16G16_SFLOAT;
	case EVertexElementType::VET_Half4:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case EVertexElementType::VET_UByte4:
		return VK_FORMAT_R8G8B8A8_UINT;
	case EVertexElementType::VET_UByte4N:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case EVertexElementType::VET_Short2:
		return VK_FORMAT_R16G16_SINT;
	case EVertexElementType::VET_Short4:
		return VK_FORMAT_R16G16B16A16_SINT;
	case EVertexElementType::VET_Short2N:
		return VK_FORMAT_R16G16_SNORM;
	case EVertexElementType::VET_Short4N:		// 4 X 16 bit word: normalized
		return VK_FORMAT_R16G16B16A16_SNORM;
	case EVertexElementType::VET_UShort2:
		return VK_FORMAT_R16G16_UINT;
	case EVertexElementType::VET_UShort4:
		return VK_FORMAT_R16G16B16A16_UINT;
	case EVertexElementType::VET_UShort2N:		// 16 bit word normalized to (value/65535.0:value/65535.0:0:0:1)
		return VK_FORMAT_R16G16_UNORM;
	case EVertexElementType::VET_UShort4N:		// 4 X 16 bit word unsigned: normalized
		return VK_FORMAT_R16G16B16A16_UNORM;
	case EVertexElementType::VET_UInt:
		return VK_FORMAT_R32_UINT;
	default:
		break;
    }
    return VK_FORMAT_UNDEFINED;
}

static VkPrimitiveTopology TranslatePrimitiveMode(EPrimitiveMode PrimitiveMode)
{
    switch(PrimitiveMode)
    {
        case EPrimitiveMode::PM_PointList : return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case EPrimitiveMode::PM_LineList : return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case EPrimitiveMode::PM_TriangleList : return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case EPrimitiveMode::PM_TriangleStrip : return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        default: 
            NILOU_LOG(Error, "Unsupported primitive type {}", magic_enum::enum_name(PrimitiveMode));
            return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
    };
}

VkFormat FVulkanDynamicRHI::TranslatePixelFormatToVKFormat(EPixelFormat Format)
{
    switch (Format) 
    {
    case EPixelFormat::PF_UNKNOWN:
        return VK_FORMAT_UNDEFINED;
    case EPixelFormat::PF_R8:
        return VK_FORMAT_R8_UINT;
    case EPixelFormat::PF_R8UI:
        return VK_FORMAT_R8_UNORM;
    case EPixelFormat::PF_R8G8:
        return VK_FORMAT_R8G8_UINT;
    case EPixelFormat::PF_R8G8B8:
        return VK_FORMAT_R8G8B8_UINT;
    case EPixelFormat::PF_R8G8B8_sRGB:
        return VK_FORMAT_R8G8B8_SRGB;
    case EPixelFormat::PF_B8G8R8:
        return VK_FORMAT_B8G8R8_UINT;
    case EPixelFormat::PF_B8G8R8_sRGB:
        return VK_FORMAT_B8G8R8_SRGB;
    case EPixelFormat::PF_R8G8B8A8:
        return VK_FORMAT_R8G8B8A8_UINT;
    case EPixelFormat::PF_R8G8B8A8_sRGB:
        return VK_FORMAT_R8G8B8A8_SRGB;
    case EPixelFormat::PF_B8G8R8A8:
        return VK_FORMAT_B8G8R8A8_UINT;
    case EPixelFormat::PF_B8G8R8A8_sRGB:
        return VK_FORMAT_B8G8R8A8_SRGB;
    case EPixelFormat::PF_D24S8:
        return VK_FORMAT_D24_UNORM_S8_UINT;
    case EPixelFormat::PF_D32F:
        return VK_FORMAT_D32_SFLOAT;
    case EPixelFormat::PF_D32FS8:
        return VK_FORMAT_D32_SFLOAT_S8_UINT;
    case EPixelFormat::PF_DXT1:
        return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case EPixelFormat::PF_DXT1_sRGB:
        return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    case EPixelFormat::PF_DXT5:
        return VK_FORMAT_BC3_UNORM_BLOCK;
    case EPixelFormat::PF_DXT5_sRGB:
        return VK_FORMAT_BC3_SRGB_BLOCK;
    case EPixelFormat::PF_R16F:
        return VK_FORMAT_R16_SFLOAT;
    case EPixelFormat::PF_R16G16F:
        return VK_FORMAT_R16G16_SFLOAT;
    case EPixelFormat::PF_R16G16B16F:
        return VK_FORMAT_R16G16B16_SFLOAT;
    case EPixelFormat::PF_R16G16B16A16F:
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    case EPixelFormat::PF_R32F:
        return VK_FORMAT_R32_SFLOAT;
    case EPixelFormat::PF_R32G32F:
        return VK_FORMAT_R32G32_SFLOAT;
    case EPixelFormat::PF_R32G32B32F:
        return VK_FORMAT_R32G32B32_SFLOAT;
    case EPixelFormat::PF_R32G32B32A32F:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    default:
        return VK_FORMAT_UNDEFINED;
    }
}
}

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
}

namespace nilou {

FVulkanDynamicRHI::FVulkanDynamicRHI(const GfxConfiguration& Config)
    : FDynamicRHI(Config)
{
    swapChainImageFormat = TranslatePixelFormatToVKFormat(Config.SwapChainFormat);
    depthImageFormat = TranslatePixelFormatToVKFormat(Config.DepthFormat);
}

void FVulkanDynamicRHI::RHIBeginFrame()
{

}

void FVulkanDynamicRHI::RHIEndFrame()
{
    
    currentFrame = (currentFrame+1) % GetFramesInFlight();
}

void FVulkanDynamicRHI::RHISetViewport(int32 Width, int32 Height)
{

}

FRHIGraphicsPipelineState *FVulkanDynamicRHI::RHISetComputeShader(RHIComputeShader *ComputeShader)
{
    FGraphicsPipelineStateInitializer Initializer;
    Initializer.ComputeShader = ComputeShader;
    FRHIGraphicsPipelineState *PSO = RHIGetOrCreatePipelineStateObject(Initializer);
    RHISetGraphicsPipelineState(PSO);
    return PSO;
}

void FVulkanDynamicRHI::RHISetGraphicsPipelineState(FRHIGraphicsPipelineState *NewState)
{
    VulkanGraphicsPipelineState* VulkanPipeline = static_cast<VulkanGraphicsPipelineState*>(NewState);
    VulkanPipelineLayout* VulkanLayout = static_cast<VulkanPipelineLayout*>(VulkanPipeline->PipelineLayout.get());
    std::vector<VkDescriptorSetLayout> layouts(GetFramesInFlight(), VulkanLayout->DescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = GetFramesInFlight();
    allocInfo.pSetLayouts = layouts.data();
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    auto bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
    if (NewState->Initializer.ComputeShader)
        bind_point = VK_PIPELINE_BIND_POINT_COMPUTE;
        
}

bool FVulkanDynamicRHI::RHISetShaderUniformBuffer(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, const std::string &ParameterName, RHIUniformBuffer *)
{
    return true;
}

bool FVulkanDynamicRHI::RHISetShaderUniformBuffer(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, int BaseIndex, RHIUniformBuffer *)
{
    return true;
}

bool FVulkanDynamicRHI::RHISetShaderSampler(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, const std::string &ParameterName, const FRHISampler &SamplerRHI)
{
    return true;
}

bool FVulkanDynamicRHI::RHISetShaderSampler(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, int BaseIndex, const FRHISampler &SamplerRHI)
{
    return true;
}

bool FVulkanDynamicRHI::RHISetShaderImage(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, RHITexture *, EDataAccessFlag AccessFlag)
{
    return true;
}

bool FVulkanDynamicRHI::RHISetShaderImage(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, RHITexture *, EDataAccessFlag AccessFlag)
{
    return true;
}


int FVulkanDynamicRHI::Initialize()
{
    FDynamicRHI::Initialize();
    uint32 extensionCount = 0;
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

    CommandBufferManager = new FVulkanCommandBufferManager(device);

    MemoryManager = new FVulkanMemoryManager(device, physicalDevice);

    StagingManager = new FVulkanStagingManager(device, this);

    LayoutManager = new FVulkanLayoutManager(device);

    /** Create descriptor pool */
    {
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = GetFramesInFlight();
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = GetFramesInFlight();

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = GetFramesInFlight();

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }

        descriptorSets.resize(GetFramesInFlight());
    }

    vkGetPhysicalDeviceProperties(physicalDevice, &GpuProps);

    shader_compiler = shaderc_compiler_initialize();

    magic_enum::enum_for_each<ETextureType>([](ETextureType TextureType) {
        magic_enum::enum_for_each<EPixelFormat>([TextureType](EPixelFormat PixelFormat) {
            ivec3 &PageSize = FDynamicRHI::SparseTextureTileSizes[(int)TextureType][(int)PixelFormat];
            PageSize = ivec3(256, 128, 1);    
        });
    }); 
    
    magic_enum::enum_for_each<EPixelFormat>(
        [this](EPixelFormat PixelFormat) 
        {
            if (PixelFormat != PF_UNKNOWN)
                vkGetPhysicalDeviceFormatProperties(physicalDevice, TranslatePixelFormatToVKFormat(PixelFormat), &FormatProperties[PixelFormat]);
        });

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

static VkDescriptorType TranslateDescriptorType(EShaderParameterType Type)
{
    switch (Type) 
    {
    case EShaderParameterType::SPT_None: return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    case EShaderParameterType::SPT_Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
    case EShaderParameterType::SPT_UniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case EShaderParameterType::SPT_Image: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    default: return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }
}

static VkShaderStageFlagBits TranslateShaderStageFlagBits(EPipelineStage Stage)
{
    switch (Stage) 
    {
    case PS_Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
    case PS_Pixel: return VK_SHADER_STAGE_FRAGMENT_BIT;
    case PS_Compute: return VK_SHADER_STAGE_COMPUTE_BIT;
    default: return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    }
}

std::shared_ptr<VulkanPipelineLayout> FVulkanDynamicRHI::RHICreatePipelineLayout(const FGraphicsPipelineStateInitializer& Initializer)
{
    VulkanPipelineLayoutRef PipelineLayout = std::make_shared<VulkanPipelineLayout>();
    AllocateParameterBindingPoint(PipelineLayout.get(), Initializer);
    std::map<std::string, VkDescriptorSetLayoutBinding> DescriptorSets;
    for (int PipelineStage = PS_Vertex; PipelineStage <= PS_Compute; PipelineStage++)
    {
        for (auto& [Name, binding] : PipelineLayout->DescriptorSets[PipelineStage].Bindings)
        {
            if (DescriptorSets.contains(Name))
            {
                DescriptorSets[Name].stageFlags = DescriptorSets[Name].stageFlags | TranslateShaderStageFlagBits((EPipelineStage)PipelineStage);
            }
            else 
            {
                VkDescriptorSetLayoutBinding LayoutBinding{};
                LayoutBinding.binding = binding.BindingPoint;
                LayoutBinding.descriptorCount = 1;
                LayoutBinding.descriptorType = TranslateDescriptorType(binding.ParameterType);
                LayoutBinding.pImmutableSamplers = nullptr;
                LayoutBinding.stageFlags = TranslateShaderStageFlagBits((EPipelineStage)PipelineStage);
                DescriptorSets[Name] = LayoutBinding;
            }
        }
    }
    std::vector<VkDescriptorSetLayoutBinding> DescriptorSetsVec;
    DescriptorSetsVec.reserve(DescriptorSets.size());
    for (auto& [Name, binding] : DescriptorSets)
        DescriptorSetsVec.push_back(binding);

    VkDescriptorSetLayoutCreateInfo descriptorSetlayoutInfo{};
    descriptorSetlayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetlayoutInfo.bindingCount = DescriptorSetsVec.size();
    descriptorSetlayoutInfo.pBindings = DescriptorSetsVec.data();
    if (vkCreateDescriptorSetLayout(device, &descriptorSetlayoutInfo, nullptr, &PipelineLayout->DescriptorSetLayout) != VK_SUCCESS) {
        NILOU_LOG(Error, "failed to create descriptor set layout!");
        return nullptr;
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &PipelineLayout->DescriptorSetLayout;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &PipelineLayout->PipelineLayout) != VK_SUCCESS) {
        NILOU_LOG(Error, "failed to create pipeline layout!");
        return nullptr;
    }
    return PipelineLayout;
}

FRHIGraphicsPipelineStateRef FVulkanDynamicRHI::RHICreateGraphicsPSO(const FGraphicsPipelineStateInitializer &Initializer)
{
    VulkanGraphicsPipelineStateRef PSO = std::make_shared<VulkanGraphicsPipelineState>();

    VulkanVertexShader* VertexShader = static_cast<VulkanVertexShader*>(Initializer.VertexShader);
    VulkanPixelShader* PixelShader = static_cast<VulkanPixelShader*>(Initializer.PixelShader);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = VertexShader->Module;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = PixelShader->Module;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    std::map<RHIBuffer*, int> VertexBufferBindingMap;
    int binding = 0;
    for (auto& VertexInput : *Initializer.VertexInputList)
    {
        if (!VertexBufferBindingMap.contains(VertexInput.VertexBuffer))
        {
            VertexBufferBindingMap[VertexInput.VertexBuffer] = binding;
            VkVertexInputBindingDescription& bindingDescription = bindingDescriptions.emplace_back();
            bindingDescription.binding = binding;
            bindingDescription.stride = VertexInput.VertexBuffer->GetStride();
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            binding++;
        }
        VkVertexInputAttributeDescription& attributeDescription = attributeDescriptions.emplace_back();
        attributeDescription.binding = VertexBufferBindingMap[VertexInput.VertexBuffer];
        attributeDescription.location = VertexInput.Location;
        attributeDescription.format = TranslateVertexElementTypeToVKFormat(VertexInput.Type);
        attributeDescription.offset = VertexInput.Offset;
    }
    
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32>(bindingDescriptions.size());
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = TranslatePrimitiveMode(Initializer.PrimitiveMode);
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VulkanDepthStencilState* DepthStencilState = static_cast<VulkanDepthStencilState*>(Initializer.DepthStencilState);
    VulkanRasterizerState* RasterizerState = static_cast<VulkanRasterizerState*>(Initializer.RasterizerState);
    VulkanBlendState* BlendState = static_cast<VulkanBlendState*>(Initializer.BlendState);

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = MAX_SIMULTANEOUS_RENDERTARGETS;
    colorBlending.pAttachments = BlendState->BlendStates;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_STENCIL_REFERENCE,
        VK_DYNAMIC_STATE_DEPTH_BOUNDS
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VulkanPipelineLayoutRef PipelineLayout = RHICreatePipelineLayout(Initializer);

    FVulkanRenderTargetLayout RTLayout(Initializer);
    PSO->RenderPass = LayoutManager->GetOrCreateRenderPass(RTLayout);

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &RasterizerState->RasterizerState;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &DepthStencilState->DepthStencilState;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = PipelineLayout->PipelineLayout;
    pipelineInfo.renderPass = PSO->RenderPass->Handle;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &PSO->VulkanPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    return PSO;
}

FRHIGraphicsPipelineStateRef FVulkanDynamicRHI::RHICreateComputePSO(const FGraphicsPipelineStateInitializer &Initializer)
{
    VulkanGraphicsPipelineStateRef PSO = std::make_shared<VulkanGraphicsPipelineState>();

    VulkanComputeShader* ComputeShader = static_cast<VulkanComputeShader*>(Initializer.ComputeShader);

    VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = ComputeShader->Module;
    computeShaderStageInfo.pName = "main";

    VulkanPipelineLayoutRef PipelineLayout = RHICreatePipelineLayout(Initializer);

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = PipelineLayout->PipelineLayout;
    pipelineInfo.stage = computeShaderStageInfo;

    if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &PSO->VulkanPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
    return PSO;
}

FRHIGraphicsPipelineState *FVulkanDynamicRHI::RHIGetOrCreatePipelineStateObject(const FGraphicsPipelineStateInitializer &Initializer)
{
    FRHIGraphicsPipelineState* CachedPSO = FPipelineStateCache::FindCachedGraphicsPSO(Initializer);
    if (CachedPSO)
        return CachedPSO;
    FRHIGraphicsPipelineStateRef PSO = nullptr;
    if (Initializer.ComputeShader)
    {
        PSO = RHICreateComputePSO(Initializer);
    }
    else 
    {
        PSO = RHICreateGraphicsPSO(Initializer);
    }
    if (PSO)
        FPipelineStateCache::CacheGraphicsPSO(Initializer, PSO);
    
    return PSO.get();
}

RHIDepthStencilStateRef FVulkanDynamicRHI::RHICreateDepthStencilState(const FDepthStencilStateInitializer &Initializer)
{
    return std::make_shared<VulkanDepthStencilState>(Initializer);
}

RHIRasterizerStateRef FVulkanDynamicRHI::RHICreateRasterizerState(const FRasterizerStateInitializer &Initializer)
{
    return std::make_shared<VulkanRasterizerState>(Initializer);
}

RHIBlendStateRef FVulkanDynamicRHI::RHICreateBlendState(const FBlendStateInitializer &Initializer)
{
    return std::make_shared<VulkanBlendState>(Initializer);
}

void FVulkanDynamicRHI::RHIBeginRenderPass(const FRHIRenderPassInfo &InInfo)
{
    FVulkanCmdBuffer* CmdBuffer = CommandBufferManager->GetActiveCmdBuffer();
    FVulkanRenderPass* RenderPass = LayoutManager->GetOrCreateRenderPass(InInfo);
    VulkanFramebuffer* Framebuffer = static_cast<VulkanFramebuffer*>(InInfo.Framebuffer);
    CmdBuffer->BeginRenderPass(InInfo, RenderPass->Handle, Framebuffer->Handle);
    CurrentFramebuffer = Framebuffer;
    CurrentRenderPass = RenderPass;
}

void FVulkanDynamicRHI::RHIDrawIndexed(RHIBuffer *IndexBuffer, int32 InstanceCount)
{
    FVulkanCmdBuffer* CmdBuffer = CommandBufferManager->GetActiveCmdBuffer();
    VulkanBuffer* vkIndexBuffer = static_cast<VulkanBuffer*>(IndexBuffer);
    VkIndexType IndexType;
    switch (IndexBuffer->GetStride()) 
    {
    case 1: IndexType = VK_INDEX_TYPE_UINT8_EXT; break;
    case 2: IndexType = VK_INDEX_TYPE_UINT16; break;
    case 4: IndexType = VK_INDEX_TYPE_UINT32; break;
    default: throw "Invalid Stride";
    }
    vkCmdBindIndexBuffer(CmdBuffer->GetHandle(), vkIndexBuffer->GetHandle(), 0, IndexType);
    vkCmdDrawIndexed(CmdBuffer->GetHandle(), static_cast<uint32>(IndexBuffer->GetCount()), InstanceCount, 0, 0, 0);
}

void FVulkanDynamicRHI::RHIDrawIndexedIndirect(RHIBuffer *IndexBuffer, RHIBuffer *IndirectBuffer, uint32 IndirectOffset)
{
    FVulkanCmdBuffer* CmdBuffer = CommandBufferManager->GetActiveCmdBuffer();
    VulkanBuffer* vkIndexBuffer = static_cast<VulkanBuffer*>(IndexBuffer);
    VulkanBuffer* vkIndirectBuffer = static_cast<VulkanBuffer*>(IndirectBuffer);
    VkIndexType IndexType;
    switch (IndexBuffer->GetStride()) 
    {
    case 1: IndexType = VK_INDEX_TYPE_UINT8_EXT; break;
    case 2: IndexType = VK_INDEX_TYPE_UINT16; break;
    case 4: IndexType = VK_INDEX_TYPE_UINT32; break;
    default: throw "Invalid Stride";
    }
    vkCmdBindIndexBuffer(CmdBuffer->GetHandle(), vkIndexBuffer->GetHandle(), 0, IndexType);
    vkCmdDrawIndexedIndirect(CmdBuffer->GetHandle(), vkIndirectBuffer->GetHandle(), IndirectOffset, 1, sizeof(VkDrawIndexedIndirectCommand));
}

void FVulkanDynamicRHI::RHIDispatch(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z)
{
    FVulkanCmdBuffer* CmdBuffer = CommandBufferManager->GetActiveCmdBuffer();
    vkCmdDispatch(CmdBuffer->GetHandle(), num_groups_x, num_groups_y, num_groups_z);
}

void FVulkanDynamicRHI::RHIDispatchIndirect(RHIBuffer *indirectArgs, uint32 IndirectOffset)
{
    FVulkanCmdBuffer* CmdBuffer = CommandBufferManager->GetActiveCmdBuffer();
    VulkanBuffer* vkIndirectBuffer = static_cast<VulkanBuffer*>(indirectArgs);
    vkCmdDispatchIndirect(CmdBuffer->GetHandle(), vkIndirectBuffer->GetHandle(), IndirectOffset);
}

void FVulkanDynamicRHI::RHIEndRenderPass()
{
	FVulkanCmdBuffer* CmdBuffer = CommandBufferManager->GetActiveCmdBuffer();
    CmdBuffer->EndRenderPass();
    if (CommandBufferManager->HasPendingUploadCmdBuffer())
    {
        CommandBufferManager->SubmitUploadCmdBuffer();
    }
    CommandBufferManager->SubmitActiveCmdBuffer({});
    CommandBufferManager->PrepareForNewActiveCommandBuffer();
}

}