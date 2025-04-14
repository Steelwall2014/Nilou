#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "ShaderReflection.h"

#include "VulkanDynamicRHI.h"
#include "BaseApplication.h"
#include "VulkanDevice.h"
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
#include "RHIStaticStates.h"
#include "Common/Log.h"

namespace nilou {

namespace VulkanRHI {

std::unordered_map<VkImage, std::tuple<EPixelFormat, int32, uint32, uint32, uint32, ETextureDimension, ETextureCreateFlags>> GCreatedImage;
VkResult vkCreateImage(
    VkDevice                                    device,
    const VkImageCreateInfo*                    pCreateInfo,
    std::tuple<EPixelFormat, int32, uint32, uint32, uint32, ETextureDimension, ETextureCreateFlags>                          pAllocator,
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

namespace sr = shader_reflection;

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
    appInfo.apiVersion = VK_API_VERSION_1_1;

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

        VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &instance));
        VK_CHECK_RESULT(CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger))
        NILOU_LOG(Display, "Setup debug messenger")
    }

    /** Create window surface */
    {
        GLFWwindow* window = reinterpret_cast<GLFWwindow*>(GetAppication()->GetWindowContext());
        VK_CHECK_RESULT(glfwCreateWindowSurface(instance, window, nullptr, &surface))
        NILOU_LOG(Display, "Create window surface")
    }

    /** Pick physical device */
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            NILOU_LOG(Fatal, "failed to find GPUs with Vulkan support!")
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
            NILOU_LOG(Fatal, "failed to find a suitable GPU!")
            return 1;
        }
        NILOU_LOG(Display, "Pick physical device")
        Device = new VulkanDevice(this, physicalDevice);
    }
    
    magic_enum::enum_for_each<EPixelFormat>(
        [this](EPixelFormat PixelFormat) 
        {
            if (PixelFormat != PF_Unknown && PixelFormat != PF_MAX)
                vkGetPhysicalDeviceFormatProperties(physicalDevice, TranslatePixelFormatToVKFormat(PixelFormat), &FormatProperties[PixelFormat]);
        });

    RenderPassManager = std::unique_ptr<FVulkanRenderPassManager>(new FVulkanRenderPassManager(Device->Handle));

    DescriptorPoolsManager = std::unique_ptr<FVulkanDescriptorPoolsManager>(new FVulkanDescriptorPoolsManager(Device->Handle));

    CurrentImageAcquiredSemaphore = std::make_shared<FVulkanSemaphore>();

    /** Create swap chain */
    {        
        VkExtent2D extent{GetAppication()->GetConfiguration().screenWidth, GetAppication()->GetConfiguration().screenHeight};
        std::vector<VkImage> TempSwapChainImages;
        SwapChain = std::unique_ptr<FVulkanSwapChain>(new FVulkanSwapChain(
            physicalDevice, Device->Handle, surface, extent, 
            swapChainImageFormat, 
            1, &Device->GfxQueue->FamilyIndex, TempSwapChainImages));
        swapChainImages.resize(TempSwapChainImages.size());

        // RHICommandList* RHICmdList = RHICreateCommandList(Device->TransferCmdBufferPool);
        for (int i = 0; i < swapChainImages.size(); i++)
        {
            auto Texture = new VulkanTexture(
                TempSwapChainImages[i], VK_NULL_HANDLE, GetFullAspectMask(swapChainImageFormat), 
                extent.width, extent.height, 1, 1, 
                0, 0, swapChainImageFormat, "SwapChainImage"+std::to_string(i), ETextureDimension::Texture2D);
            // RHIImageMemoryBarrier SwapChainImageBarrier{
            //     Texture, 
            //     ERHIAccess::Present, ERHIAccess::Present, 
            //     EPipelineStageFlags::BottomOfPipe, EPipelineStageFlags::BottomOfPipe,
            //     ETextureLayout::Undefined, ETextureLayout::PresentSrc,
            //     RHITextureSubresource()};
            // RHICmdList->PipelineBarrier({}, {SwapChainImageBarrier}, {});
            swapChainImages[i] = Texture;
        }

        swapChainExtent = extent;
        FRHITextureCreateInfo TextureInfo;
        TextureInfo.TextureType = ETextureDimension::Texture2D;
        TextureInfo.SizeX = swapChainExtent.width;
        TextureInfo.SizeY = swapChainExtent.height;
        TextureInfo.Format = depthImageFormat;
        TextureInfo.Flags = TexCreate_DepthStencilTargetable | TexCreate_DepthStencilResolveTarget;
        DepthImage = RHICreateTexture(TextureInfo, "Vulkan Render to Screen DepthStencil");
        FRHITextureViewCreateInfo CreateInfo;
        CreateInfo.ViewType = ETextureDimension::Texture2D;
        CreateInfo.Format = depthImageFormat;
        CreateInfo.BaseMipLevel = 0;
        CreateInfo.LevelCount = 1;
        CreateInfo.BaseArrayLayer = 0;
        CreateInfo.LayerCount = 1;
        DepthImageView = RHICreateTextureView(DepthImage, CreateInfo, "Vulkan Render to Screen DepthStencil TextureView");
        auto VkDepthImage = ResourceCast(DepthImage);
        // RHIImageMemoryBarrier SwapChainImageBarrier{
        //     DepthImage, 
        //     ERHIAccess::Present, ERHIAccess::Present, 
        //     EPipelineStageFlags::BottomOfPipe, EPipelineStageFlags::BottomOfPipe,
        //     ETextureLayout::Undefined, ETextureLayout::PresentSrc,
        //     RHITextureSubresource()};
        // SwapChainImageBarrier.Subresource = RHITextureSubresource(0, 0, 1);
        // RHICmdList->PipelineBarrier({}, {SwapChainImageBarrier}, {});
        // SwapChainImageBarrier.Subresource = RHITextureSubresource(0, 0, 2);
        // RHICmdList->PipelineBarrier({}, {SwapChainImageBarrier}, {});
        // this->RHISubmitCommandList(RHICmdList, {}, {});
    }

    /** Create image views */
    {
        RHIRenderTargetLayout RTLayout;
        RTLayout.ColorAttachments[0].Format = swapChainImageFormat;
        RTLayout.DepthStencilAttachment.Format = depthImageFormat;
        VkRenderPass RenderPass = RenderPassManager->GetOrCreateRenderPass(RTLayout);
        
        swapChainImageViews.resize(swapChainImages.size());
        swapChainFramebuffers.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++) {
            // VkImageViewCreateInfo createInfo{};
            // createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            // createInfo.image = ResourceCast(swapChainImages[i])->GetHandle();
            // createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            // createInfo.format = TranslatePixelFormatToVKFormat(swapChainImageFormat);
            // createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            // createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            // createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            // createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            // createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            // createInfo.subresourceRange.baseMipLevel = 0;
            // createInfo.subresourceRange.levelCount = 1;
            // createInfo.subresourceRange.baseArrayLayer = 0;
            // createInfo.subresourceRange.layerCount = 1;

            FRHITextureViewCreateInfo CreateInfo;
            CreateInfo.ViewType = ETextureDimension::Texture2D;
            CreateInfo.Format = swapChainImageFormat;
            CreateInfo.BaseMipLevel = 0;
            CreateInfo.LevelCount = 1;
            CreateInfo.BaseArrayLayer = 0;
            CreateInfo.LayerCount = 1;
            swapChainImageViews[i] = RHICreateTextureView(swapChainImages[i], CreateInfo, "SwapChainImageView");
            swapChainFramebuffers[i] = RenderPassManager->GetOrCreateFramebuffer(RenderPass, { swapChainImageViews[i].GetReference() }, DepthImageView.GetReference());
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

    return 0;
}

void FVulkanDynamicRHI::Finalize()
{
    SamplerMap.clear();
    RenderPassManager = nullptr;
    DescriptorPoolsManager = nullptr;
    SwapChain = nullptr;
    swapChainFramebuffers.clear();
    DepthImage = nullptr;
    FPipelineStateCache::ClearCacheGraphicsPSO();
    FPipelineStateCache::ClearCacheVertexDeclarations();
    for (auto ImageView : swapChainImageViews)
        vkDestroyImageView(Device->Handle, ResourceCast(ImageView)->GetHandle(), nullptr);

    shaderc_compiler_release(shader_compiler);

    vkDestroyDevice(Device->Handle, nullptr);

    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    FDynamicRHI::Finalize();
}

void FVulkanDynamicRHI::GetError(const char *file, int line)
{
    
}

static VkDescriptorType TranslateDescriptorType(EDescriptorType Type)
{
    switch (Type)
    {
    case EDescriptorType::UniformBuffer:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case EDescriptorType::StorageBuffer:
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case EDescriptorType::Sampler:
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case EDescriptorType::StorageImage:
        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    default:
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }
}

static VkShaderStageFlagBits TranslateShaderStageFlagBits(EShaderStage StageFlags)
{
    return (VkShaderStageFlagBits)StageFlags;
}

RHIPipelineLayoutRef FVulkanDynamicRHI::RHICreatePipelineLayout(const std::vector<RHIShader*>& shaders, const std::vector<RHIPushConstantRange>& PushConstantRanges)
{
    VulkanPipelineLayoutRef PipelineLayout = new VulkanPipelineLayout(Device->Handle);
    auto& NameToPositions = PipelineLayout->UniformPositions;
    for (RHIShader* shader : shaders)
    {
        for (auto& [SetIndex, SetLayout] : shader->Reflection)
        {
            std::vector<VkDescriptorSetLayoutBinding> Bindings;
            for (auto& [BindingIndex, Descriptor] : SetLayout)
            {
                // Map the name of the uniform variable within the uniform block to its position.
                // The position is a combination of the set index, binding index and offset within the uniform block.
                if (Descriptor.DescriptorType == sr::EDescriptorType::UniformBuffer)
                {
                    for (const sr::BlockVariable& BlockVariable : Descriptor.Block.Members)
                    {
                        RHIPipelineLayout::UniformPosition Position;
                        Position.SetIndex = SetIndex;
                        Position.BindingIndex = BindingIndex;
                        Position.Offset = BlockVariable.Offset;
                        NameToPositions[BlockVariable.Name] = Position;
                    }
                }
                else if (Descriptor.DescriptorType == sr::EDescriptorType::CombinedImageSampler)
                {
                    RHIPipelineLayout::UniformPosition Position;
                    Position.SetIndex = SetIndex;
                    Position.BindingIndex = BindingIndex;
                    Position.Offset = 0;
                    NameToPositions[Descriptor.Name] = Position;
                }

                // Fill the vulkan structure
                VkDescriptorSetLayoutBinding LayoutBinding{};
                LayoutBinding.binding = BindingIndex;
                LayoutBinding.descriptorCount = 1;
                LayoutBinding.descriptorType = static_cast<VkDescriptorType>(Descriptor.DescriptorType);
                LayoutBinding.pImmutableSamplers = nullptr;
                LayoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
                Bindings.push_back(LayoutBinding);
            }
            
            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = Bindings.size();
            layoutInfo.pBindings = Bindings.data();
            if (SetIndex >= PipelineLayout->SetLayoutHandles.size())
                PipelineLayout->SetLayoutHandles.resize(SetIndex + 1, VK_NULL_HANDLE);
            Ncheck(vkCreateDescriptorSetLayout(Device->Handle, &layoutInfo, nullptr, &PipelineLayout->SetLayoutHandles[SetIndex]));
        }
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = PipelineLayout->SetLayoutHandles.size();
    pipelineLayoutInfo.pSetLayouts = PipelineLayout->SetLayoutHandles.data();

    std::vector<VkPushConstantRange> VulkanPushConstantRanges;
    for (const RHIPushConstantRange& Range : PushConstantRanges)
    {
        VkPushConstantRange VulkanRange{};
        VulkanRange.stageFlags = TranslateShaderStageFlagBits(Range.StageFlags);
        VulkanRange.offset = Range.Offset;
        VulkanRange.size = Range.Size;
        VulkanPushConstantRanges.push_back(VulkanRange);
    }
    pipelineLayoutInfo.pushConstantRangeCount = VulkanPushConstantRanges.size();
    pipelineLayoutInfo.pPushConstantRanges = VulkanPushConstantRanges.data();

    VK_CHECK_RESULT(vkCreatePipelineLayout(Device->Handle, &pipelineLayoutInfo, nullptr, &PipelineLayout->Handle));
    return PipelineLayout;
}

RHIGraphicsPipelineStateRef FVulkanDynamicRHI::RHICreateGraphicsPSO(const FGraphicsPipelineStateInitializer &Initializer, const std::vector<RHIPushConstantRange>& PushConstantRanges)
{
    VulkanGraphicsPipelineStateRef PSO = new VulkanGraphicsPipelineState(Device->Handle, Initializer);

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

    VulkanVertexDeclaration* vkVertexDeclaration = static_cast<VulkanVertexDeclaration*>(Initializer.VertexDeclaration);
    VkPipelineVertexInputStateCreateInfo& vertexInputInfo = vkVertexDeclaration->VertexInputInfo;

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

    static RHIDepthStencilState* DefaultDepthStencilState = TStaticDepthStencilState<>::CreateRHI();
    static RHIRasterizerState* DefaultRasterizerState = TStaticRasterizerState<>::CreateRHI();
    static RHIBlendState* DefaultBlendState = TStaticBlendState<>::CreateRHI();
    VulkanDepthStencilState* DepthStencilState = static_cast<VulkanDepthStencilState*>(Initializer.DepthStencilState ? Initializer.DepthStencilState : DefaultDepthStencilState);
    VulkanRasterizerState* RasterizerState = static_cast<VulkanRasterizerState*>(Initializer.RasterizerState ? Initializer.RasterizerState : DefaultRasterizerState);
    VulkanBlendState* BlendState = static_cast<VulkanBlendState*>(Initializer.BlendState ? Initializer.BlendState : DefaultBlendState);

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = MaxSimultaneousRenderTargets;
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

    RHIPipelineLayoutRef PipelineLayout = RHICreatePipelineLayout( {Initializer.VertexShader, Initializer.PixelShader}, PushConstantRanges );

    PSO->PipelineLayout = PipelineLayout;

    PSO->RenderPass = RenderPassManager->GetOrCreateRenderPass(Initializer.RTLayout);

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
    pipelineInfo.layout = ResourceCast(PipelineLayout)->Handle;
    pipelineInfo.renderPass = PSO->RenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(Device->Handle, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &PSO->Handle) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    return PSO;
}

RHIComputePipelineStateRef FVulkanDynamicRHI::RHICreateComputePSO(RHIComputeShader* ComputeShader, const std::vector<RHIPushConstantRange>& PushConstantRanges)
{
    VulkanComputeShader* vkComputeShader = static_cast<VulkanComputeShader*>(ComputeShader);

    VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = vkComputeShader->Module;
    computeShaderStageInfo.pName = "main";

    RHIPipelineLayoutRef PipelineLayout = RHICreatePipelineLayout( {ComputeShader}, PushConstantRanges );

    VulkanComputePipelineStateRef PSO = new VulkanComputePipelineState(Device->Handle, ComputeShader);
    PSO->PipelineLayout = PipelineLayout;

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = ResourceCast(PipelineLayout)->Handle;
    pipelineInfo.stage = computeShaderStageInfo;

    if (vkCreateComputePipelines(Device->Handle, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &PSO->Handle) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
    return PSO;
}

RHIGraphicsPipelineState *FVulkanDynamicRHI::RHICreateGraphicsPipelineState(const FGraphicsPipelineStateInitializer &Initializer, const std::vector<RHIPushConstantRange>& PushConstantRanges)
{
    RHIGraphicsPipelineState* CachedPSO = FPipelineStateCache::FindCachedGraphicsPSO(Initializer);
    if (CachedPSO)
        return CachedPSO;
    RHIGraphicsPipelineStateRef PSO = RHICreateGraphicsPSO(Initializer, PushConstantRanges);
    FPipelineStateCache::CacheGraphicsPSO(Initializer, PSO);
    
    return PSO;
}

RHIComputePipelineState* FVulkanDynamicRHI::RHICreateComputePipelineState(RHIComputeShader* ComputeShader, const std::vector<RHIPushConstantRange>& PushConstantRanges)
{
    RHIComputePipelineState* CachedPSO = FPipelineStateCache::FindCachedComputePSO(ComputeShader);
    if (CachedPSO)
        return CachedPSO;
    RHIComputePipelineStateRef PSO = RHICreateComputePSO(ComputeShader, PushConstantRanges);
    FPipelineStateCache::CacheComputePSO(ComputeShader, PSO);
    
    return PSO;
}

RHIDepthStencilStateRef FVulkanDynamicRHI::RHICreateDepthStencilState(const FDepthStencilStateInitializer &Initializer)
{
    return new VulkanDepthStencilState(Initializer);
}

RHIRasterizerStateRef FVulkanDynamicRHI::RHICreateRasterizerState(const FRasterizerStateInitializer &Initializer)
{
    return new VulkanRasterizerState(Initializer);
}

RHIBlendStateRef FVulkanDynamicRHI::RHICreateBlendState(const FBlendStateInitializer &Initializer)
{
    return new VulkanBlendState(Initializer);
}

inline VkSamplerMipmapMode TranslateMipFilterMode(ESamplerFilter InFilter)
{
	switch (InFilter)
	{
		case SF_Point:				return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		case SF_Bilinear:			return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		case SF_Trilinear:			return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		case SF_AnisotropicPoint:	return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		case SF_AnisotropicLinear:	return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		default:
			break;
	}

	NILOU_LOG(Error, "Unknown Mip ESamplerFilter {}", magic_enum::enum_name(InFilter));
	return VK_SAMPLER_MIPMAP_MODE_MAX_ENUM;
}

inline VkFilter TranslateMinMagFilterMode(ESamplerFilter InFilter)
{
	switch (InFilter)
	{
		case SF_Point:				return VK_FILTER_NEAREST;
		case SF_Bilinear:			return VK_FILTER_LINEAR;
		case SF_Trilinear:			return VK_FILTER_LINEAR;
		case SF_AnisotropicPoint:
		case SF_AnisotropicLinear:	return VK_FILTER_LINEAR;
		default:
			break;
	}

    NILOU_LOG(Error, "Unknown MinMag ESamplerFilter {}", magic_enum::enum_name(InFilter));
	return VK_FILTER_MAX_ENUM;
}

inline VkSamplerAddressMode TranslateWrapMode(ESamplerAddressMode InAddressMode)
{
	switch (InAddressMode)
	{
		case AM_Wrap:		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case AM_Clamp:		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case AM_Mirror:		return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case AM_Border:		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		default:
			break;
	}

    NILOU_LOG(Error, "Unknown ESamplerAddressMode {}", magic_enum::enum_name(InAddressMode));
	return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
}

inline VkCompareOp TranslateSamplerCompareFunction(ESamplerCompareFunction InSamplerComparisonFunction)
{
	switch (InSamplerComparisonFunction)
	{
		case SCF_Less:	return VK_COMPARE_OP_LESS;
		case SCF_Never:	return VK_COMPARE_OP_NEVER;
		default:
			break;
	};

    NILOU_LOG(Error, "Unknown ESamplerCompareFunction {}", magic_enum::enum_name(InSamplerComparisonFunction));
	return VK_COMPARE_OP_MAX_ENUM;
}

RHISamplerStateRef FVulkanDynamicRHI::RHICreateSamplerState(const FSamplerStateInitializer &Initializer)
{
    VulkanSamplerStateRef Sampler = new VulkanSamplerState(Initializer, Device->Handle);
    VkSamplerCreateInfo SamplerInfo{};
    SamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

	SamplerInfo.magFilter = TranslateMinMagFilterMode(Initializer.Filter);
	SamplerInfo.minFilter = TranslateMinMagFilterMode(Initializer.Filter);
	SamplerInfo.mipmapMode = TranslateMipFilterMode(Initializer.Filter);
	SamplerInfo.addressModeU = TranslateWrapMode(Initializer.AddressU);
	SamplerInfo.addressModeV = TranslateWrapMode(Initializer.AddressV);
	SamplerInfo.addressModeW = TranslateWrapMode(Initializer.AddressW);

	SamplerInfo.mipLodBias = Initializer.MipBias;
	
	SamplerInfo.maxAnisotropy = GpuProps.limits.maxSamplerAnisotropy;
	// if (Initializer.Filter == SF_AnisotropicLinear || Initializer.Filter == SF_AnisotropicPoint)
	// {
	// 	SamplerInfo.maxAnisotropy = FMath::Clamp((float)ComputeAnisotropyRT(Initializer.MaxAnisotropy), 1.0f, InDevice.GetLimits().maxSamplerAnisotropy);
	// }
	SamplerInfo.anisotropyEnable = SamplerInfo.maxAnisotropy > 1.0f;

	SamplerInfo.compareEnable = Initializer.SamplerComparisonFunction != SCF_Never ? VK_TRUE : VK_FALSE;
	SamplerInfo.compareOp = TranslateSamplerCompareFunction(Initializer.SamplerComparisonFunction);
	SamplerInfo.minLod = Initializer.MinMipLevel;
	SamplerInfo.maxLod = Initializer.MaxMipLevel;
	SamplerInfo.borderColor = Initializer.BorderColor == 0 ? VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK : VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    
    VK_CHECK_RESULT(vkCreateSampler(Device->Handle, &SamplerInfo, nullptr, &Sampler->Handle));

	uint32 CRC = FCrc::MemCrc32(&SamplerInfo, sizeof(SamplerInfo));
    SamplerMap[CRC] = Sampler;

    return Sampler;
}

FRHIVertexDeclaration* FVulkanDynamicRHI::RHICreateVertexDeclaration(const FVertexDeclarationElementList& ElementList)
{
    FRHIVertexDeclaration* CachedDeclaration = FPipelineStateCache::FindVertexDeclaration(ElementList);
    if (CachedDeclaration)
        return CachedDeclaration;
    
    VulkanVertexDeclarationRef Declaration = new VulkanVertexDeclaration();

    VkPipelineVertexInputStateCreateInfo& vertexInputInfo = Declaration->VertexInputInfo;
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    std::vector<VkVertexInputBindingDescription>& bindingDescriptions = Declaration->BindingDescriptions;
    std::vector<VkVertexInputAttributeDescription>& attributeDescriptions = Declaration->AttributeDescriptions;

    VkVertexInputBindingDescription Bindings[MAX_VERTEX_ELEMENTS];
    uint32 BindingMask = 0;
    for (auto& Element : ElementList)
    {
        VkVertexInputBindingDescription& CurrBinding = Bindings[Element.StreamIndex];
        if (((1 << Element.StreamIndex) & BindingMask) != 0)
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

    FPipelineStateCache::CacheVertexDeclaration(ElementList, Declaration);
    return Declaration;
}

RHISemaphoreRef FVulkanDynamicRHI::RHICreateSemaphore()
{
    return new VulkanSemaphore(Device->Handle);
}

uint32 FVulkanDynamicRHI::RHIComputeMemorySize(RHITexture* TextureRHI)
{
	if(!TextureRHI)
	{
		return 0;
	}

    VkMemoryRequirements memRequirements{};
    vkGetImageMemoryRequirements(Device->Handle, ResourceCast(TextureRHI)->Handle, &memRequirements);
    return memRequirements.size;
}

std::string FVulkanDynamicRHI::ErrorString(VkResult errorCode)
{
    switch (errorCode)
    {
#define STR(r) case VK_##r: return #r
        STR(NOT_READY);
        STR(TIMEOUT);
        STR(EVENT_SET);
        STR(EVENT_RESET);
        STR(INCOMPLETE);
        STR(ERROR_OUT_OF_HOST_MEMORY);
        STR(ERROR_OUT_OF_DEVICE_MEMORY);
        STR(ERROR_INITIALIZATION_FAILED);
        STR(ERROR_DEVICE_LOST);
        STR(ERROR_MEMORY_MAP_FAILED);
        STR(ERROR_LAYER_NOT_PRESENT);
        STR(ERROR_EXTENSION_NOT_PRESENT);
        STR(ERROR_FEATURE_NOT_PRESENT);
        STR(ERROR_INCOMPATIBLE_DRIVER);
        STR(ERROR_TOO_MANY_OBJECTS);
        STR(ERROR_FORMAT_NOT_SUPPORTED);
        STR(ERROR_SURFACE_LOST_KHR);
        STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
        STR(SUBOPTIMAL_KHR);
        STR(ERROR_OUT_OF_DATE_KHR);
        STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
        STR(ERROR_VALIDATION_FAILED_EXT);
        STR(ERROR_INVALID_SHADER_NV);
        STR(ERROR_INCOMPATIBLE_SHADER_BINARY_EXT);
#undef STR
    default:
        return "UNKNOWN_ERROR";
    }
}

}