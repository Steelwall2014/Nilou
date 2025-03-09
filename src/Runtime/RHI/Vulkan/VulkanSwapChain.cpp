#include "VulkanSwapChain.h"
#include "VulkanQueue.h"
#include "Common/Log.h"
#define DEBUG_HELPER NILOU_LOG(Display, "{} {}", __FILE__, __LINE__);
// FVulkanSwapChain的构造函数看起来会导致一些堆损坏的bug，可能是在这之前某些数组越界了

namespace nilou {

FVulkanSwapChain::FVulkanSwapChain(VkPhysicalDevice PhysDevice, VkDevice InDevice, VkSurfaceKHR Surface, VkExtent2D Extent, EPixelFormat Format, int32 QueueFamilyIndexCount, uint32* QueueFamilyIndices, std::vector<VkImage>& OutImages)
    : Device(InDevice)
{
    DEBUG_HELPER vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysDevice, Surface, &Capabilities);

    uint32 formatCount;
    DEBUG_HELPER vkGetPhysicalDeviceSurfaceFormatsKHR(PhysDevice, Surface, &formatCount, nullptr);

    DEBUG_HELPER if (formatCount != 0) {
    DEBUG_HELPER     Formats.resize(formatCount);
    DEBUG_HELPER     vkGetPhysicalDeviceSurfaceFormatsKHR(PhysDevice, Surface, &formatCount, Formats.data());
    }

    uint32 presentModeCount;
    DEBUG_HELPER vkGetPhysicalDeviceSurfacePresentModesKHR(PhysDevice, Surface, &presentModeCount, nullptr);

    DEBUG_HELPER if (presentModeCount != 0) {
    DEBUG_HELPER     PresentModes.resize(presentModeCount);
    DEBUG_HELPER     vkGetPhysicalDeviceSurfacePresentModesKHR(PhysDevice, Surface, &presentModeCount, PresentModes.data());
    }

    DEBUG_HELPER uint32 imageCount = Capabilities.minImageCount + 1;
    DEBUG_HELPER if (Capabilities.maxImageCount > 0 && imageCount > Capabilities.maxImageCount) {
    DEBUG_HELPER     imageCount = Capabilities.maxImageCount;
    }

    DEBUG_HELPER VkSwapchainCreateInfoKHR createInfo{};
    DEBUG_HELPER createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    DEBUG_HELPER createInfo.surface = Surface;

    DEBUG_HELPER createInfo.minImageCount = imageCount;
    DEBUG_HELPER createInfo.imageFormat = TranslatePixelFormatToVKFormat(Format);
    DEBUG_HELPER createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    DEBUG_HELPER createInfo.imageExtent = Extent;
    DEBUG_HELPER createInfo.imageArrayLayers = 1;
    DEBUG_HELPER createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;;

    DEBUG_HELPER if (QueueFamilyIndices == nullptr)
    {
    DEBUG_HELPER     createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    DEBUG_HELPER     createInfo.queueFamilyIndexCount = QueueFamilyIndexCount;
    DEBUG_HELPER     createInfo.pQueueFamilyIndices = QueueFamilyIndices;
    }
    else 
    {
    DEBUG_HELPER     createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    DEBUG_HELPER createInfo.preTransform = Capabilities.currentTransform;
    DEBUG_HELPER createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    DEBUG_HELPER createInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    DEBUG_HELPER createInfo.clipped = VK_TRUE;

    DEBUG_HELPER createInfo.oldSwapchain = VK_NULL_HANDLE;

    DEBUG_HELPER if (vkCreateSwapchainKHR(Device, &createInfo, nullptr, &Handle) != VK_SUCCESS) {
    DEBUG_HELPER     throw("failed to create swap chain!");
    }

    DEBUG_HELPER vkGetSwapchainImagesKHR(Device, Handle, &imageCount, nullptr);
    DEBUG_HELPER OutImages.resize(imageCount);
    DEBUG_HELPER vkGetSwapchainImagesKHR(Device, Handle, &imageCount, OutImages.data());

    DEBUG_HELPER ImageAcquiredSemaphore.resize(imageCount);

    DEBUG_HELPER for (int i = 0; i < ImageAcquiredSemaphore.size(); i++)
    {
    DEBUG_HELPER     ImageAcquiredSemaphore[i] = CreateSemephore(Device);
    }

}

void FVulkanSwapChain::Present(VulkanQueue* GfxQueue, VulkanQueue* PresentQueue)
{
	VkPresentInfoKHR Info{};
    Info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	Info.swapchainCount = 1;
	Info.pSwapchains = &Handle;
	Info.pImageIndices = (uint32*)&CurrentImageIndex;
    vkQueuePresentKHR(PresentQueue->Handle, &Info); 
}

int32 FVulkanSwapChain::AcquireImageIndex(VkSemaphore* OutSemaphore)
{
    SemaphoreIndex = (SemaphoreIndex+1) % ImageAcquiredSemaphore.size();
    uint32 ImageIndex;
    VkResult result = vkAcquireNextImageKHR(
        Device, Handle, UINT64_MAX, 
        ImageAcquiredSemaphore[SemaphoreIndex]->Handle, 
        VK_NULL_HANDLE, &ImageIndex);
    CurrentImageIndex = ImageIndex;
    *OutSemaphore = ImageAcquiredSemaphore[SemaphoreIndex]->Handle;
    return CurrentImageIndex;
}

}