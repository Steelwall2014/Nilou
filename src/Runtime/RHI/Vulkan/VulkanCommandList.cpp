#include "VulkanCommandList.h"
#include "VulkanCommandBuffer.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanPipelineState.h"
#include "VulkanDescriptorSet.h"

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
            vkTexture->Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Region);
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

}
