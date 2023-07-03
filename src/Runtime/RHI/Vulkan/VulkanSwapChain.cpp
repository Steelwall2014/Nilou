#include "VulkanSwapChain.h"
#include "VulkanQueue.h"
#include "Common/Log.h"

namespace nilou {

FVulkanSwapChain::FVulkanSwapChain(VkPhysicalDevice PhysDevice, VkDevice InDevice, VkSurfaceKHR Surface, VkExtent2D Extent, EPixelFormat Format, int32 QueueFamilyIndexCount, uint32* QueueFamilyIndices, std::vector<VkImage>& OutImages)
    : Device(InDevice)
{
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysDevice, Surface, &Capabilities);

    uint32 formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(PhysDevice, Surface, &formatCount, nullptr);

    if (formatCount != 0) {
        Formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(PhysDevice, Surface, &formatCount, Formats.data());
    }

    uint32 presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(PhysDevice, Surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        PresentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(PhysDevice, Surface, &presentModeCount, PresentModes.data());
    }

    uint32 imageCount = Capabilities.minImageCount + 1;
    if (Capabilities.maxImageCount > 0 && imageCount > Capabilities.maxImageCount) {
        imageCount = Capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = Surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = TranslatePixelFormatToVKFormat(Format);
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = Extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;;

    if (QueueFamilyIndices == nullptr)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = QueueFamilyIndexCount;
        createInfo.pQueueFamilyIndices = QueueFamilyIndices;
    }
    else 
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = Capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(Device, &createInfo, nullptr, &Handle) != VK_SUCCESS) {
        throw("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(Device, Handle, &imageCount, nullptr);
    OutImages.resize(imageCount);
    vkGetSwapchainImagesKHR(Device, Handle, &imageCount, OutImages.data());

    ImageAcquiredSemaphore.resize(imageCount);

    for (int i = 0; i < ImageAcquiredSemaphore.size(); i++)
    {
        ImageAcquiredSemaphore[i] = CreateSemephore(Device);
    }

}

void FVulkanSwapChain::Present(FVulkanQueue* GfxQueue, FVulkanQueue* PresentQueue)
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