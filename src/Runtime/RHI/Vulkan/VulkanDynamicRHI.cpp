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
#include "VulkanPipelineState.h"
#include "VulkanDescriptorSet.h"
#include "VulkanVertexDeclaration.h"
#include "VulkanQueue.h"
#include "VulkanSwapChain.h"
#include "VulkanBarriers.h"
#include "Common/Log.h"

namespace nilou {

namespace VulkanRHI {

std::unordered_map<VkImage, std::tuple<EPixelFormat, int32, uint32, uint32, uint32, ETextureType, ETextureCreateFlags>> GCreatedImage;
VkResult vkCreateImage(
    VkDevice                                    device,
    const VkImageCreateInfo*                    pCreateInfo,
    std::tuple<EPixelFormat, int32, uint32, uint32, uint32, ETextureType, ETextureCreateFlags>                          pAllocator,
    VkImage*                                    pImage)
{
    VkResult res = ::vkCreateImage(device, pCreateInfo, nullptr, pImage);
    GCreatedImage.insert({*pImage, pAllocator});
    return res;
}

void vkDestroyImage(
    VkDevice                                    device,
    VkImage                                     image,
    const VkAllocationCallbacks*                pAllocator)
{
    ::vkDestroyImage(device, image, pAllocator);
    GCreatedImage.erase(image);
    if (GCreatedImage.size() == 1)
        std::cout << 1;
}

}

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

    queueFamilies = std::vector<VkQueueFamilyProperties>(queueFamilyCount);
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
    swapChainImageFormat = Config.SwapChainFormat;
    depthImageFormat = Config.DepthFormat;
}

void FVulkanDynamicRHI::RHIBeginFrame()
{
    CurrentSwapChainImageIndex = SwapChain->AcquireImageIndex(&CurrentImageAcquiredSemaphore);
    DescriptorPoolsManager->Reset();
    FVulkanCmdBuffer* CmdBuffer = CommandBufferManager->GetUploadCmdBuffer();
    VkImageSubresourceRange Region{};
    Region.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    Region.baseArrayLayer = 0;
    Region.baseMipLevel = 0;
    Region.layerCount = 1;
    Region.levelCount = 1;
    FVulkanPipelineBarrier Barrier;
    Barrier.AddImageLayoutTransition(
        swapChainImages[CurrentSwapChainImageIndex]->GetImage(),
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, Region);
    Barrier.Execute(CmdBuffer);
    // TransitionImageLayout(
    //     CmdBuffer->Handle, swapChainImages[CurrentSwapChainImageIndex], 
    //     VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, Region);
    CommandBufferManager->SubmitUploadCmdBuffer();
}

void FVulkanDynamicRHI::RHIEndFrame()
{
    FVulkanCmdBuffer* CmdBuffer = CommandBufferManager->GetUploadCmdBuffer();
    VkImageSubresourceRange Region{};
    Region.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    Region.baseArrayLayer = 0;
    Region.baseMipLevel = 0;
    Region.layerCount = 1;
    Region.levelCount = 1;
    FVulkanPipelineBarrier Barrier;
    Barrier.AddImageLayoutTransition(
        swapChainImages[CurrentSwapChainImageIndex]->GetImage(),
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, Region);
    Barrier.Execute(CmdBuffer);
    // TransitionImageLayout(
    //     CmdBuffer->Handle, swapChainImages[CurrentSwapChainImageIndex], 
    //     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, Region);
    CommandBufferManager->SubmitUploadCmdBuffer();
    SwapChain->Present(GfxQueue.get(), PresentQueue.get());
    CommandBufferManager->FreeUnusedCmdBuffers();
}

void FVulkanDynamicRHI::RHISetViewport(int32 Width, int32 Height)
{
    FVulkanCmdBuffer* CmdBuffer = CommandBufferManager->GetActiveCmdBuffer();
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) Width;
    viewport.height = (float) Height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(CmdBuffer->GetHandle(), 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = VkExtent2D{(uint32)Width, (uint32)Height};
    vkCmdSetScissor(CmdBuffer->GetHandle(), 0, 1, &scissor);
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
    CurrentPipelineLayout = VulkanLayout;
    
    FVulkanDescriptorSets Sets = DescriptorPoolsManager->AllocateDescriptorSets(*VulkanLayout->DescriptorSetsLayout);
    CurrentDescriptorState = std::make_unique<FVulkanCommonPipelineDescriptorState>(this, Sets);

    FVulkanCmdBuffer* CmdBuffer = CommandBufferManager->GetActiveCmdBuffer();   
    auto bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
    if (NewState->Initializer.ComputeShader)
    {
        bind_point = VK_PIPELINE_BIND_POINT_COMPUTE;
    }

    vkCmdBindPipeline(CmdBuffer->GetHandle(), bind_point, VulkanPipeline->VulkanPipeline);

    
}

bool FVulkanDynamicRHI::RHISetShaderUniformBuffer(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, RHIUniformBuffer *UniformBufferRHI)
{
    return RHISetShaderUniformBuffer(
        BoundPipelineState, PipelineStage, 
        BoundPipelineState->GetBaseIndexByName(PipelineStage, ParameterName), UniformBufferRHI);
}

bool FVulkanDynamicRHI::RHISetShaderUniformBuffer(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, int BaseIndex, RHIUniformBuffer *UniformBufferRHI)
{
    if (CurrentDescriptorState)
    {
        CurrentDescriptorState->SetUniformBuffer(BaseIndex, UniformBufferRHI);
        return true;
    }
    return false;
}

bool FVulkanDynamicRHI::RHISetShaderSampler(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, const FRHISampler &SamplerRHI)
{
    return RHISetShaderSampler(
        BoundPipelineState, PipelineStage, 
        BoundPipelineState->GetBaseIndexByName(PipelineStage, ParameterName), SamplerRHI);
}

bool FVulkanDynamicRHI::RHISetShaderSampler(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, const FRHISampler &SamplerRHI)
{
    if (CurrentDescriptorState)
    {
        CurrentDescriptorState->SetSampler(BaseIndex, SamplerRHI);
        return true;
    }
    return false;
}

bool FVulkanDynamicRHI::RHISetShaderImage(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, RHITexture *Texture, EDataAccessFlag AccessFlag)
{
    return RHISetShaderImage(
        BoundPipelineState, PipelineStage, 
        BoundPipelineState->GetBaseIndexByName(PipelineStage, ParameterName), Texture);
}

bool FVulkanDynamicRHI::RHISetShaderImage(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, RHITexture *Texture, EDataAccessFlag AccessFlag)
{
    if (CurrentDescriptorState)
    {
        CurrentDescriptorState->SetImage(BaseIndex, Texture, AccessFlag);
        return true;
    }
    return false;
}

void FVulkanDynamicRHI::RHISetStreamSource(uint32 StreamIndex, RHIBuffer* Buffer, uint32 Offset)
{
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(Buffer);
    CurrentStreamSourceOffsets[StreamIndex] = Offset;
    CurrentStreamSourceBuffers[StreamIndex] = vkBuffer->GetHandle();
}

void FVulkanDynamicRHI::RHIBindComputeBuffer(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, RHIBuffer* buffer)
{
    RHIBindComputeBuffer(
        BoundPipelineState, PipelineStage, 
        BoundPipelineState->GetBaseIndexByName(PipelineStage, ParameterName), buffer);
}

void FVulkanDynamicRHI::RHIBindComputeBuffer(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, RHIBuffer* buffer)
{
    if (CurrentDescriptorState)
    {
        CurrentDescriptorState->SetBuffer(BaseIndex, buffer);
    }
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
        NILOU_LOG(Info, "Setup debug messenger")
    }

    /** Create window surface */
    {
        GLFWwindow* window = reinterpret_cast<GLFWwindow*>(GetAppication()->GetWindowContext());
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            NILOU_LOG(Error, "failed to create window surface!")
            return 1;
        }
        NILOU_LOG(Info, "Create window surface")
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
        NILOU_LOG(Info, "Pick physical device")
    }

    std::optional<uint32> GfxQueueIndex, PresentQueueIndex;
    /** Create logical device */
    {
        uint32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        queueFamilies.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
        
        for (int i = 0; i < queueFamilies.size(); i++) {
            const auto& queueFamily = queueFamilies[i];
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                GfxQueueIndex = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

            if (presentSupport) {
                PresentQueueIndex = i;
            }

            if (GfxQueueIndex.has_value() && PresentQueueIndex.has_value()) {
                break;
            }
        }

        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = GfxQueueIndex.value();
        queueCreateInfo.queueCount = 1;
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        std::vector<const char*> deviceExtensions = {
            "VK_KHR_swapchain"
        };
        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.shaderFloat64 = VK_TRUE;
        deviceFeatures.wideLines = VK_TRUE;
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
        NILOU_LOG(Info, "Create logical device")
    }

    {
        GfxQueue = std::unique_ptr<FVulkanQueue>(new FVulkanQueue(device, GfxQueueIndex.value()));
        PresentQueue = std::unique_ptr<FVulkanQueue>(new FVulkanQueue(device, PresentQueueIndex.value()));
        NILOU_LOG(Info, "Create queues")
    }

    magic_enum::enum_for_each<ETextureType>([](ETextureType TextureType) {
        magic_enum::enum_for_each<EPixelFormat>([TextureType](EPixelFormat PixelFormat) {
            ivec3 &PageSize = FDynamicRHI::SparseTextureTileSizes[(int)TextureType][(int)PixelFormat];
            PageSize = ivec3(256, 128, 1);    
        });
    }); 
    
    magic_enum::enum_for_each<EPixelFormat>(
        [this](EPixelFormat PixelFormat) 
        {
            if (PixelFormat != PF_UNKNOWN && PixelFormat != PF_PixelFormatNum)
                vkGetPhysicalDeviceFormatProperties(physicalDevice, TranslatePixelFormatToVKFormat(PixelFormat), &FormatProperties[PixelFormat]);
        });

    CommandBufferManager = std::unique_ptr<FVulkanCommandBufferManager>(new FVulkanCommandBufferManager(device, this));

    MemoryManager = std::unique_ptr<FVulkanMemoryManager>(new FVulkanMemoryManager(device, physicalDevice));

    StagingManager = std::unique_ptr<FVulkanStagingManager>(new FVulkanStagingManager(device, this));

    RenderPassManager = std::unique_ptr<FVulkanRenderPassManager>(new FVulkanRenderPassManager(device));

    DescriptorPoolsManager = std::unique_ptr<FVulkanDescriptorPoolsManager>(new FVulkanDescriptorPoolsManager(device));

    /** Create swap chain */
    {        
        VkExtent2D extent{GetAppication()->GetConfiguration().screenWidth, GetAppication()->GetConfiguration().screenHeight};
        std::vector<VkImage> TempSwapChainImages;
        SwapChain = std::unique_ptr<FVulkanSwapChain>(new FVulkanSwapChain(
            physicalDevice, device, surface, extent, 
            swapChainImageFormat, 
            1, &GfxQueueIndex.value(), TempSwapChainImages));
        swapChainImages.resize(TempSwapChainImages.size());
        FVulkanCmdBuffer* CmdBuffer = CommandBufferManager->GetUploadCmdBuffer();
        FVulkanPipelineBarrier Barrier;
        for (int i = 0; i < swapChainImages.size(); i++)
        {
            auto VulkanTexture = std::make_shared<VulkanTexture2D>(
                TempSwapChainImages[i], VK_NULL_HANDLE, VK_NULL_HANDLE, 
                FVulkanImageLayout{VK_IMAGE_LAYOUT_UNDEFINED, 1, 1}, extent.width, extent.height, 1, 1, 
                swapChainImageFormat, "SwapChainImage"+std::to_string(i));
            Barrier.AddImageLayoutTransition(
                VulkanTexture->GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, 
                VulkanTexture->GetImageLayout(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
            swapChainImages[i] = VulkanTexture;
        }

        swapChainExtent = extent;
        DepthImage = std::static_pointer_cast<VulkanTexture2D>(RHICreateTexture2D(
            "Vulkan Render to Screen DepthStencil", depthImageFormat, 1, swapChainExtent.width, swapChainExtent.height, 
            TexCreate_DepthStencilTargetable | TexCreate_DepthStencilResolveTarget));
        Barrier.AddImageLayoutTransition(
            DepthImage->GetImage(), VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 
            DepthImage->GetImageLayout(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

        Barrier.Execute(CmdBuffer);
    }

    /** Create image views */
    {
        swapChainImageViews.resize(swapChainImages.size());
        swapChainFramebuffers.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i]->GetImage();
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = TranslatePixelFormatToVKFormat(swapChainImageFormat);
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
            swapChainImages[i]->TextureBase.ImageView = swapChainImageViews[i];

            // std::array<VkImageView, 2> attachments = {
            //     swapChainImageViews[i],
            //     depthImageView
            // };

            // VkFramebufferCreateInfo framebufferInfo{};
            // framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            // FVulkanRenderTargetLayout RTLayout({
            //     { FA_Color_Attachment0, swapChainImageFormat }, 
            //     { FA_Depth_Stencil_Attachment, depthImageFormat}} );
            // RenderToScreenPass = RenderPassManager->GetOrCreateRenderPass(RTLayout);
            // framebufferInfo.renderPass = RenderToScreenPass->Handle;
            // framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            // framebufferInfo.pAttachments = attachments.data();
            // framebufferInfo.width = swapChainExtent.width;
            // framebufferInfo.height = swapChainExtent.height;
            // framebufferInfo.layers = 1;

            // if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            //     throw std::runtime_error("failed to create framebuffer!");
            // }
            swapChainFramebuffers[i] = std::shared_ptr<VulkanFramebuffer>(
                new VulkanFramebuffer(this, {
                        { FA_Color_Attachment0, swapChainImages[i] }, 
                        { FA_Depth_Stencil_Attachment, DepthImage}}));
        }

    }
    
    {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

    vkGetPhysicalDeviceProperties(physicalDevice, &GpuProps);

    shader_compiler = shaderc_compiler_initialize();

    CommandBufferManager->PrepareForNewActiveCommandBuffer();

    return 0;
}

void FVulkanDynamicRHI::Finalize()
{
    SamplerMap.clear();
    StagingManager = nullptr;
    RenderPassManager = nullptr;
    DescriptorPoolsManager = nullptr;
    SwapChain = nullptr;
    swapChainFramebuffers.clear();
    DepthImage = nullptr;
    FPipelineStateCache::ClearCacheGraphicsPSO();
    FPipelineStateCache::ClearCacheVertexDeclarations();
    CommandBufferManager = nullptr;
    MemoryManager = nullptr;
    for (auto ImageView : swapChainImageViews)
        vkDestroyImageView(device, ImageView, nullptr);

    shaderc_compiler_release(shader_compiler);

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
    case EShaderParameterType::SPT_Sampler: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
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
    VulkanPipelineLayoutRef PipelineLayout = std::make_shared<VulkanPipelineLayout>(device);
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

    PipelineLayout->DescriptorSetsLayout = std::shared_ptr<FVulkanDescriptorSetsLayout>(new FVulkanDescriptorSetsLayout(device, {DescriptorSetsVec}));

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = PipelineLayout->DescriptorSetsLayout->Handles.size();
    pipelineLayoutInfo.pSetLayouts = PipelineLayout->DescriptorSetsLayout->Handles.data();

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &PipelineLayout->PipelineLayout) != VK_SUCCESS) {
        NILOU_LOG(Error, "failed to create pipeline layout!");
        return nullptr;
    }
    return PipelineLayout;
}

FRHIGraphicsPipelineStateRef FVulkanDynamicRHI::RHICreateGraphicsPSO(const FGraphicsPipelineStateInitializer &Initializer)
{
    VulkanGraphicsPipelineStateRef PSO = std::make_shared<VulkanGraphicsPipelineState>(device, Initializer);

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

    VkVertexInputBindingDescription Bindings[MaxVertexElementCount];
    uint32 BindingMask = 0;
    VulkanVertexDeclaration* vkVertexDeclaration = static_cast<VulkanVertexDeclaration*>(Initializer.VertexDeclaration);
    for (auto& Element : vkVertexDeclaration->Elements)
    {
        VkVertexInputBindingDescription& CurrBinding = Bindings[Element.StreamIndex];
        if ((1 << Element.StreamIndex) & BindingMask != 0)
        {
            Ncheck(CurrBinding.binding == Element.StreamIndex);
            Ncheck(CurrBinding.inputRate == VK_VERTEX_INPUT_RATE_VERTEX);
            Ncheck(CurrBinding.stride == Element.Stride);
        }
        else
        {
            CurrBinding.binding = Element.StreamIndex;
            CurrBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            CurrBinding.stride = Element.Stride;
            bindingDescriptions.push_back(CurrBinding);
        }
        VkVertexInputAttributeDescription AttrDesc;
        AttrDesc.binding = Element.StreamIndex;
        AttrDesc.format = TranslateVertexElementTypeToVKFormat(Element.Type);
        AttrDesc.location = Element.AttributeIndex;
        AttrDesc.offset = Element.Offset;
        attributeDescriptions.push_back(AttrDesc);
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
    colorBlending.attachmentCount = Initializer.NumRenderTargetsEnabled;
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

    PSO->PipelineLayout = PipelineLayout;

    FVulkanRenderTargetLayout RTLayout(Initializer);
    PSO->RenderPass = RenderPassManager->GetOrCreateRenderPass(RTLayout);

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
    VulkanGraphicsPipelineStateRef PSO = std::make_shared<VulkanGraphicsPipelineState>(device, Initializer);

    VulkanComputeShader* ComputeShader = static_cast<VulkanComputeShader*>(Initializer.ComputeShader);

    VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = ComputeShader->Module;
    computeShaderStageInfo.pName = "main";

    VulkanPipelineLayoutRef PipelineLayout = RHICreatePipelineLayout(Initializer);

    PSO->PipelineLayout = PipelineLayout;

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

FRHIVertexDeclarationRef FVulkanDynamicRHI::RHICreateVertexDeclaration(const std::vector<FVertexElement>& Elements)
{
    VulkanVertexDeclarationRef Declaration = std::make_shared<VulkanVertexDeclaration>();
    Declaration->Elements = Elements;
    return Declaration;
}

void FVulkanDynamicRHI::RHIBeginRenderPass(const FRHIRenderPassInfo &InInfo)
{
    FVulkanCmdBuffer* CmdBuffer = CommandBufferManager->GetActiveCmdBuffer();
    VulkanFramebuffer* Framebuffer = static_cast<VulkanFramebuffer*>(InInfo.Framebuffer);
    CurrentFramebuffer = Framebuffer;
    if (Framebuffer)
    {
        if (InInfo.Framebuffer)
        {
            FVulkanImageLayoutBarrierHelper Barrier;
            for (auto& [Attachment, Texture] : InInfo.Framebuffer->Attachments)
            {
                VulkanTextureBase* vkTexture = ResourceCast(Texture.get());
                VkImage Image{};
                VkImageAspectFlags Aspect = GetFullAspectMask(Texture->GetFormat());
                FVulkanImageLayout SrcLayout = vkTexture->GetImageLayout();
                VkImageLayout DstLayout = Attachment == FA_Depth_Stencil_Attachment ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                Barrier.AddImageLayoutTransition(vkTexture, Aspect, SrcLayout, DstLayout);
            }
            Barrier.Execute(CommandBufferManager->GetUploadCmdBuffer());
            //CommandBufferManager->SubmitUploadCmdBuffer();
        }
        FVulkanRenderTargetLayout RTLayout(InInfo);
        FVulkanRenderPass* RenderPass = RenderPassManager->GetOrCreateRenderPass(RTLayout);
        CmdBuffer->BeginRenderPass(InInfo, RenderPass->Handle, Framebuffer->Handle);
    }
    else 
    {
        CmdBuffer->BeginRenderPass(InInfo, RenderToScreenPass->Handle, swapChainFramebuffers[CurrentSwapChainImageIndex]->Handle);
    }
}

void FVulkanDynamicRHI::RHIDrawArrays(uint32 First, uint32 Count, int32 InstanceCount)
{
    PrepareForDraw();
    FVulkanCmdBuffer* CmdBuffer = CommandBufferManager->GetActiveCmdBuffer();
    vkCmdDraw(CmdBuffer->GetHandle(), Count, InstanceCount, First, 0);
}

void FVulkanDynamicRHI::RHIDrawIndexed(RHIBuffer *IndexBuffer, int32 InstanceCount)
{
    PrepareForDraw();
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
    PrepareForDraw();
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
    PrepareForDispatch();
    FVulkanCmdBuffer* CmdBuffer = CommandBufferManager->GetActiveCmdBuffer();
    vkCmdDispatch(CmdBuffer->GetHandle(), num_groups_x, num_groups_y, num_groups_z);
    CommandBufferManager->SubmitActiveCmdBuffer({});
    CommandBufferManager->PrepareForNewActiveCommandBuffer();
    CurrentDescriptorState->DescriptorSets.Owner->TrackRemoveUsage(CurrentDescriptorState->DescriptorSets.Layout);
}

void FVulkanDynamicRHI::RHIDispatchIndirect(RHIBuffer *indirectArgs, uint32 IndirectOffset)
{
    PrepareForDispatch();
    FVulkanCmdBuffer* CmdBuffer = CommandBufferManager->GetActiveCmdBuffer();
    VulkanBuffer* vkIndirectBuffer = static_cast<VulkanBuffer*>(indirectArgs);
    vkCmdDispatchIndirect(CmdBuffer->GetHandle(), vkIndirectBuffer->GetHandle(), IndirectOffset);
    CommandBufferManager->SubmitActiveCmdBuffer({});
    CommandBufferManager->PrepareForNewActiveCommandBuffer();
    CurrentDescriptorState->DescriptorSets.Owner->TrackRemoveUsage(CurrentDescriptorState->DescriptorSets.Layout);
}

void FVulkanDynamicRHI::RHIEndRenderPass()
{
	FVulkanCmdBuffer* CmdBuffer = CommandBufferManager->GetActiveCmdBuffer();
    CmdBuffer->EndRenderPass();
    if (CommandBufferManager->HasPendingUploadCmdBuffer())
    {
        CommandBufferManager->SubmitUploadCmdBuffer();
    }
    if (CurrentFramebuffer == GetRenderToScreenFramebuffer())
    {
        CmdBuffer->AddWaitSemaphore(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, CurrentImageAcquiredSemaphore);
    }
    CommandBufferManager->SubmitActiveCmdBuffer({});
    CommandBufferManager->PrepareForNewActiveCommandBuffer();
    CurrentDescriptorState->DescriptorSets.Owner->TrackRemoveUsage(CurrentDescriptorState->DescriptorSets.Layout);
}

void FVulkanDynamicRHI::PrepareForDispatch()
{
    CurrentDescriptorState->UpdateDescriptorSet();
    FVulkanCmdBuffer* CmdBuffer = CommandBufferManager->GetActiveCmdBuffer();
    vkCmdBindDescriptorSets(
        CmdBuffer->GetHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, 
        CurrentPipelineLayout->PipelineLayout, 0, 
        CurrentDescriptorState->DescriptorSets.Handles.size(), 
        CurrentDescriptorState->DescriptorSets.Handles.data(), 
        0, nullptr);
}

void FVulkanDynamicRHI::PrepareForDraw()
{
    CurrentDescriptorState->UpdateDescriptorSet();
    FVulkanCmdBuffer* CmdBuffer = CommandBufferManager->GetActiveCmdBuffer();
    vkCmdBindDescriptorSets(
        CmdBuffer->GetHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, 
        CurrentPipelineLayout->PipelineLayout, 0, 
        CurrentDescriptorState->DescriptorSets.Handles.size(), 
        CurrentDescriptorState->DescriptorSets.Handles.data(), 
        0, nullptr);
    for (int i = 0; i < MaxVertexElementCount; i++)
    {
        if (CurrentStreamSourceBuffers[i] != VK_NULL_HANDLE)
        {
            vkCmdBindVertexBuffers(
                CmdBuffer->GetHandle(), i, 1, 
                &CurrentStreamSourceBuffers[i], &CurrentStreamSourceOffsets[i]);
        }
    }
    RHISetViewport(swapChainExtent.width, swapChainExtent.height);
}

RHIFramebuffer* FVulkanDynamicRHI::GetRenderToScreenFramebuffer()
{
    return swapChainFramebuffers[CurrentSwapChainImageIndex].get();
}


}