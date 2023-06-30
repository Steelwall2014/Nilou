#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "VulkanResources.h"
#include "Platform.h"
#include "RHIDefinitions.h"

namespace nilou {

class FVulkanQueue;

class FVulkanSwapChain
{
public:
    FVulkanSwapChain(VkPhysicalDevice PhysDevice, VkDevice Device, VkSurfaceKHR Surface, VkExtent2D Extent, EPixelFormat Format, int32 QueueFamilyIndexCount, uint32* QueueFamilyIndices, std::vector<VkImage>& OutImages);

    VkDevice Device;
    VkSurfaceCapabilitiesKHR Capabilities;
    std::vector<VkSurfaceFormatKHR> Formats;
    std::vector<VkPresentModeKHR> PresentModes;
    VkSwapchainKHR Handle;
    std::vector<VkSemaphore> ImageAcquiredSemaphore;

    void Present(FVulkanQueue* GfxQueue, FVulkanQueue* PresentQueue);

	VkResult AcquireImageIndex(VkSemaphore* OutSemaphore);

private:

    uint32 SemaphoreIndex = 0;
};

}