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
    std::vector<std::shared_ptr<FVulkanSemaphore>> ImageAcquiredSemaphore;

    void Present(FVulkanQueue* GfxQueue, FVulkanQueue* PresentQueue);

	int32 AcquireImageIndex(VkSemaphore* OutSemaphore);

    int32 CurrentImageIndex;

private:

    uint32 SemaphoreIndex = 0;
};

}