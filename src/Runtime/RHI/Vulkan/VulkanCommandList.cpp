#include "VulkanCommandList.h"
#include "VulkanCommandBuffer.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanPipelineState.h"
#include "VulkanDescriptorSet.h"
#include "VulkanQueue.h"

namespace nilou {

    static VkDescriptorType GetDescriptorType(EDescriptorType Type)
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

    void VulkanCommandList::BindDescriptorSets(RHIPipelineLayout* PipelineLayout, const std::unordered_map<uint32, RHIDescriptorSet*>& DescriptorSets, EPipelineBindPoint PipelineBindPoint)
    {
        VkPipelineBindPoint BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        switch (PipelineBindPoint) 
        {
        case EPipelineBindPoint::Graphics:
            BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            break;
        case EPipelineBindPoint::Compute:
            BindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
            break;
        case EPipelineBindPoint::RayTracing:
            BindPoint = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
            break;
        }

        VulkanPipelineLayout* VulkanLayout = ResourceCast(PipelineLayout);
        std::vector<VkWriteDescriptorSet> WriteVec;
        std::vector<VkDescriptorSet> DescriptorSetHandles;
        for (auto& [SetIndex, DescriptorSet] : DescriptorSets)
        {
            VulkanDescriptorSet* VulkanDescriptorSet = ResourceCast(const_cast<RHIDescriptorSet*>(DescriptorSet));
            DescriptorSetHandles.push_back(VulkanDescriptorSet->Handle);
            for (auto& [BindingIndex, Info] : VulkanDescriptorSet->Writers)
            {
                WriteVec.push_back(Info.WriteDescriptor);
            }  
        }  
        
        vkUpdateDescriptorSets(CmdBuffer->Device, WriteVec.size(), WriteVec.data(), 0, nullptr);
        vkCmdBindDescriptorSets(
            CmdBuffer->GetHandle(), BindPoint, 
            VulkanLayout->Handle, 0, DescriptorSetHandles.size(), DescriptorSetHandles.data(), 0, nullptr);
    }

    void VulkanCommandList::CopyBufferToImage(
        RHIBuffer* SrcBuffer, RHITexture* DstTexture, 
        int32 MipmapLevel, int32 Xoffset, int32 Yoffset, int32 Zoffset, 
        uint32 Width, uint32 Height, uint32 Depth, int32 BaseArrayLayer)
    {
        VkBufferImageCopy Region{};
        Region.imageSubresource.aspectMask = GetFullAspectMask(DstTexture->GetFormat());
        Region.imageSubresource.mipLevel = MipmapLevel;
        Region.imageSubresource.baseArrayLayer = BaseArrayLayer;
        Region.imageSubresource.layerCount = 1;
        Region.imageOffset.x = Xoffset;
        Region.imageOffset.y = Yoffset;
        Region.imageOffset.z = Zoffset;
        Region.imageExtent.width = Width;
        Region.imageExtent.height = Height;
        Region.imageExtent.depth = Depth;

        VulkanBuffer* vkBuffer = ResourceCast(SrcBuffer);
        VulkanTexture* vkTexture = static_cast<VulkanTexture*>(DstTexture);
        vkCmdCopyBufferToImage(
            CmdBuffer->GetHandle(), vkBuffer->Handle, 
            vkTexture->Handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Region);
    }

    void VulkanCommandList::BindVertexBuffer(int32 BindingPoint, RHIBuffer* Buffer, uint64 Offset)
    {
        VkBuffer vkBuffer = ResourceCast(Buffer)->Handle;
        vkCmdBindVertexBuffers(CmdBuffer->GetHandle(), BindingPoint, 1, &vkBuffer, &Offset);
    }

    void VulkanCommandList::BindIndexBuffer(RHIBuffer* Buffer, uint64 Offset)
    {
        VkBuffer vkBuffer = ResourceCast(Buffer)->Handle;
        vkCmdBindIndexBuffer(CmdBuffer->GetHandle(), vkBuffer, Offset, VK_INDEX_TYPE_UINT32);
    }

    void VulkanCommandList::PushConstants(RHIPipelineLayout* PipelineLayout, EPipelineBindPoint PipelineBindPoint, uint32 Offset, uint32 Size, const void* Data)
    {
        VulkanPipelineLayout* VulkanLayout = static_cast<VulkanPipelineLayout*>(PipelineLayout);
        VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL;
        if (PipelineBindPoint == EPipelineBindPoint::Compute)
            stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        else
            stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        vkCmdPushConstants(
            CmdBuffer->GetHandle(), 
            VulkanLayout->Handle, 
            stageFlags, 
            Offset, 
            Size, 
            Data);
    }

    VkPipelineStageFlags Translate(EPipelineStageFlags Flags)
    {
        return (VkPipelineStageFlags)Flags;
    }

    VkAccessFlagBits Translate(ERHIAccess Access)
    {
        return (VkAccessFlagBits)Access;
    }

    VkImageLayout Translate(ETextureLayout Layout)
    {
        return (VkImageLayout)Layout;
    }

    VkImageAspectFlags TranslatePlaneSlice(uint8 PlaneSlice)
    {
        switch (PlaneSlice)
        {
        case 0:
            return VK_IMAGE_ASPECT_COLOR_BIT;
        case 1:
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        case 2:
            return VK_IMAGE_ASPECT_STENCIL_BIT;
        default:
            NILOU_LOG(Fatal, "Invalid PlaneSlice");
        };
        return VK_IMAGE_ASPECT_NONE;
    }

    VkMemoryBarrier2 Translate(RHIMemoryBarrier Barrier)
    {
        VkMemoryBarrier2 VkBarrier{};
        VkBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
        VkBarrier.srcStageMask = Translate(Barrier.SrcStage);
        VkBarrier.srcAccessMask = Translate(Barrier.SrcAccess);
        VkBarrier.dstStageMask = Translate(Barrier.DstStage);
        VkBarrier.dstAccessMask = Translate(Barrier.DstAccess);
        return VkBarrier;
    }

    VkBufferMemoryBarrier2 Translate(RHIBufferMemoryBarrier Barrier)
    {
        VkBufferMemoryBarrier2 VkBarrier{};
        VkBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
        VkBarrier.srcStageMask = Translate(Barrier.SrcStage);
        VkBarrier.srcAccessMask = Translate(Barrier.SrcAccess);
        VkBarrier.dstStageMask = Translate(Barrier.DstStage);
        VkBarrier.dstAccessMask = Translate(Barrier.DstAccess);
        VkBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        VkBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        VkBarrier.buffer = ResourceCast(Barrier.Buffer)->Handle;
        VkBarrier.offset = Barrier.Offset;
        VkBarrier.size = Barrier.Size;
        return VkBarrier;
    }

    VkImageMemoryBarrier2 Translate(RHIImageMemoryBarrier Barrier)
    {
        VkImageMemoryBarrier2 VkBarrier{};
        VkBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
        VkBarrier.srcStageMask = Translate(Barrier.SrcStage);
        VkBarrier.srcAccessMask = Translate(Barrier.SrcAccess);
        VkBarrier.dstStageMask = Translate(Barrier.DstStage);
        VkBarrier.dstAccessMask = Translate(Barrier.DstAccess);
        VkBarrier.oldLayout = Translate(Barrier.OldLayout);
        VkBarrier.newLayout = Translate(Barrier.NewLayout);
        VkBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        VkBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        VkBarrier.image = ResourceCast(Barrier.Texture)->Handle;
        VkImageSubresourceRange VkSubresourceRange{};
        VkSubresourceRange.baseArrayLayer = Barrier.Subresource.ArraySlice;
        VkSubresourceRange.baseMipLevel = Barrier.Subresource.MipIndex;
        VkSubresourceRange.layerCount = 1;
        VkSubresourceRange.levelCount = 1;
        VkSubresourceRange.aspectMask = TranslatePlaneSlice(Barrier.Subresource.PlaneSlice);
        VkBarrier.subresourceRange = VkSubresourceRange;
        return VkBarrier;
    }

    void VulkanCommandList::PipelineBarrier(
        const std::vector<RHIMemoryBarrier>& MemoryBarriers, 
        const std::vector<RHIImageMemoryBarrier>& ImageBarriers, 
        const std::vector<RHIBufferMemoryBarrier>& BufferBarriers)
    {
        std::vector<VkMemoryBarrier2> VkMemoryBarriers;
        std::vector<VkImageMemoryBarrier2> VkImageBarriers;
        std::vector<VkBufferMemoryBarrier2> VkBufferBarriers;

        VkMemoryBarriers.reserve(MemoryBarriers.size());
        VkImageBarriers.reserve(ImageBarriers.size());
        VkBufferBarriers.reserve(BufferBarriers.size());
        for (const RHIMemoryBarrier& Barrier : MemoryBarriers)
        {
            VkMemoryBarrier2 VkBarrier = Translate(Barrier);
            VkMemoryBarriers.push_back(VkBarrier);
        }
        for (const RHIImageMemoryBarrier& Barrier : ImageBarriers)
        {
            VkImageMemoryBarrier2 VkBarrier = Translate(Barrier);
            VkImageBarriers.push_back(VkBarrier);
        }
        for (const RHIBufferMemoryBarrier& Barrier : BufferBarriers)
        {
            VkBufferMemoryBarrier2 VkBarrier = Translate(Barrier);
            VkBufferBarriers.push_back(VkBarrier);
        }

        VkDependencyInfo DependencyInfo{};
        DependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        DependencyInfo.memoryBarrierCount = VkMemoryBarriers.size();
        DependencyInfo.pMemoryBarriers = VkMemoryBarriers.data();
        DependencyInfo.bufferMemoryBarrierCount = VkBufferBarriers.size();
        DependencyInfo.pBufferMemoryBarriers = VkBufferBarriers.data();
        DependencyInfo.imageMemoryBarrierCount = VkImageBarriers.size();
        DependencyInfo.pImageMemoryBarriers = VkImageBarriers.data();
        vkCmdPipelineBarrier2(CmdBuffer->GetHandle(), &DependencyInfo);
    }

    void VulkanCommandList::Submit(const std::vector<RHISemaphoreRef>& SemaphoresToWait, const std::vector<RHISemaphoreRef>& SemaphoresToSignal)
    {
        std::vector<VkSemaphoreSubmitInfo> WaitSemephores;
        std::vector<VkSemaphoreSubmitInfo> SignalSemephores;
        for (RHISemaphoreRef Semaphore : SemaphoresToWait)
        {
            VulkanSemaphore* VkSemaphore = ResourceCast(Semaphore);
            VkSemaphoreSubmitInfo SemaphoreInfo{};
            SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
            SemaphoreInfo.semaphore = VkSemaphore->Handle;
            WaitSemephores.push_back(SemaphoreInfo);
        }
        for (RHISemaphoreRef Semaphore : SemaphoresToSignal)
        {
            VulkanSemaphore* VkSemaphore = ResourceCast(Semaphore);
            VkSemaphoreSubmitInfo SemaphoreInfo{};
            SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
            SemaphoreInfo.semaphore = VkSemaphore->Handle;
            SignalSemephores.push_back(SemaphoreInfo);
        }
        VkSubmitInfo2 SubmitInfo{};
        SubmitInfo.waitSemaphoreInfoCount = WaitSemephores.size();
        SubmitInfo.pWaitSemaphoreInfos = WaitSemephores.data();
        VkCommandBufferSubmitInfo CmdBufferInfo{};
        CmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        CmdBufferInfo.commandBuffer = CmdBuffer->GetHandle();
        SubmitInfo.commandBufferInfoCount = 1;
        SubmitInfo.pCommandBufferInfos = &CmdBufferInfo;
        SubmitInfo.signalSemaphoreInfoCount = SignalSemephores.size();
        SubmitInfo.pSignalSemaphoreInfos = SignalSemephores.data();
        vkQueueSubmit2(Queue->Handle, 1, &SubmitInfo, VK_NULL_HANDLE);
    }


}
