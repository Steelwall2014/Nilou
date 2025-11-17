#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "VulkanResources.h"
#include "HAL/Platform.h"
#include "RHIDefinitions.h"
#include "RHITransition.h"

namespace nilou {

class VulkanQueue;
class VulkanSemaphore;

class FVulkanSwapChain
{
public:
    FVulkanSwapChain(VkPhysicalDevice PhysDevice, VkDevice Device, VkSurfaceKHR Surface, VkExtent2D Extent, EPixelFormat Format, int32 QueueFamilyIndexCount, uint32* QueueFamilyIndices, std::vector<VkImage>& OutImages);
    ~FVulkanSwapChain() 
    {
        for (int i = 0; i < ImageAcquiredFences.size(); i++)
        {
            vkDestroyFence(Device, ImageAcquiredFences[i], nullptr);
        }
        vkDestroySwapchainKHR(Device, Handle, nullptr); 
    }
    VkDevice Device;
    VkSurfaceCapabilitiesKHR Capabilities;
    std::vector<VkSurfaceFormatKHR> Formats;
    std::vector<VkPresentModeKHR> PresentModes;
    VkSwapchainKHR Handle;
    std::vector<RHISemaphoreRef> ImageAcquiredSemaphore;
    std::vector<VkFence> ImageAcquiredFences;

    void Present(VulkanQueue* GfxQueue, VulkanQueue* PresentQueue);

	int32 AcquireImageIndex(VkSemaphore* OutSemaphore);

    int32 CurrentImageIndex;
    std::vector<RHITextureRef> Images;
    std::vector<RHITextureViewRef> ImageViews;
    RHITextureRef DepthImage;
    RHITextureViewRef DepthImageView;
    VkExtent2D Extent{};

private:

    uint32 SemaphoreIndex = 0;
};

}