#include "VulkanDynamicRHI.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanPipelineState.h"
#include "VulkanDescriptorSet.h"
#include "VulkanQueue.h"
#include "VulkanResources.h"

namespace nilou {

    VulkanCommandBuffer::VulkanCommandBuffer(VkDevice InDevice, VkQueue InQueue, VkCommandPool InPool)
        : Device(InDevice)
        , Queue(InQueue)
        , Pool(InPool)
    {
        VkCommandBufferAllocateInfo AllocInfo{};
        AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        AllocInfo.commandPool = Pool;
        AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        AllocInfo.commandBufferCount = 1;
        VK_CHECK_RESULT(vkAllocateCommandBuffers(Device, &AllocInfo, &Handle));

        VkFenceCreateInfo FenceInfo{};
        FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        FenceInfo.flags = 0;
        VK_CHECK_RESULT(vkCreateFence(InDevice, &FenceInfo, nullptr, &Fence));

        State = EState::ReadyForBegin;
    }

    VulkanCommandBuffer::~VulkanCommandBuffer()
    {
        vkFreeCommandBuffers(Device, Pool, 1, &Handle);
    }

    void VulkanCommandBuffer::BeginRenderPass(FRHIRenderPassInfo& Info)
    {
        FVulkanDynamicRHI* DynamicRHI = FVulkanDynamicRHI::GetDynamicRHI();
        VkRenderPass RenderPass = DynamicRHI->RenderPassManager->GetOrCreateRenderPass(Info.RTLayout);
        VkFramebuffer Framebuffer = DynamicRHI->RenderPassManager->GetOrCreateFramebuffer(RenderPass, Info.ColorRenderTargets, Info.DepthStencilRenderTarget);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = RenderPass;
        renderPassInfo.framebuffer = Framebuffer;
        renderPassInfo.renderArea.offset = VkOffset2D{Info.Offset.x, Info.Offset.y};
        renderPassInfo.renderArea.extent = VkExtent2D{Info.Extent.x, Info.Extent.y};        

        std::vector<VkClearValue> clearValues;
        for (auto [Index, ClearColor] : Enumerate(Info.ClearColors))
        {
            auto& RenderTarget = Info.ColorRenderTargets[Index];
            auto& LoadAction = Info.RTLayout.ColorAttachments[Index].LoadAction;
            if (RenderTarget)
            {
                VkClearValue clearColor{};
                if (LoadAction == ERenderTargetLoadAction::Clear)
                {
                    clearColor.color = VkClearColorValue{ {ClearColor.r, ClearColor.g, ClearColor.b, ClearColor.a} };
                }
                clearValues.push_back(clearColor);
            }
        }
        renderPassInfo.clearValueCount = clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();
        
        vkCmdBeginRenderPass(Handle, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        State = EState::IsInsideRenderPass;
    }

    void VulkanCommandBuffer::EndRenderPass()
    {
        State = EState::IsInsideBegin;
    }

    void VulkanCommandBuffer::DrawArrays(uint32 VertexCount, uint32 InstanceCount, uint32 FirstVertex, uint32 FirstInstance)
    {
        vkCmdDraw(Handle, VertexCount, InstanceCount, FirstVertex, FirstInstance);
    }

    void VulkanCommandBuffer::DrawIndexed(uint32 IndexCount, uint32 InstanceCount, uint32 FirstIndex, uint32 VertexOffset, uint32 FirstInstance)
    {
        vkCmdDrawIndexed(Handle, IndexCount, InstanceCount, FirstIndex, VertexOffset, FirstInstance);
    }

    void VulkanCommandBuffer::DispatchCompute(uint32 NumGroupsX, uint32 NumGroupsY, uint32 NumGroupsZ)
    {
        vkCmdDispatch(Handle, NumGroupsX, NumGroupsY, NumGroupsZ);
    }

    void VulkanCommandBuffer::DrawIndirect(RHIBuffer* Buffer, uint32 Offset)
    {
        VulkanBuffer* VkBuffer = ResourceCast(Buffer);
        vkCmdDrawIndirect(Handle, VkBuffer->Handle, Offset, 1, 0);
    }

    void VulkanCommandBuffer::DrawIndexedIndirect(RHIBuffer* Buffer, uint32 Offset)
    {
        VulkanBuffer* VkBuffer = ResourceCast(Buffer);
        vkCmdDrawIndexedIndirect(Handle, VkBuffer->Handle, Offset, 1, 0);
    }

    void VulkanCommandBuffer::DispatchComputeIndirect(RHIBuffer* Buffer, uint32 Offset)
    {
        VulkanBuffer* VkBuffer = ResourceCast(Buffer);
        vkCmdDispatchIndirect(Handle, VkBuffer->Handle, 0);
    }

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

    void VulkanCommandBuffer::BindGraphicsPipelineState(RHIGraphicsPipelineState *NewPipelineState)
    {
        VulkanGraphicsPipelineState* PSO = ResourceCast(NewPipelineState);
        vkCmdBindPipeline(Handle, VK_PIPELINE_BIND_POINT_GRAPHICS, PSO->Handle);
    }
    
    void VulkanCommandBuffer::BindComputePipelineState(RHIComputePipelineState *NewPipelineState)
    {
        VulkanComputePipelineState* PSO = ResourceCast(NewPipelineState);
        vkCmdBindPipeline(Handle, VK_PIPELINE_BIND_POINT_COMPUTE, PSO->Handle);
    }

    void VulkanCommandBuffer::BindDescriptorSets(RHIPipelineLayout* PipelineLayout, const std::unordered_map<uint32, RHIDescriptorSet*>& DescriptorSets, EPipelineBindPoint PipelineBindPoint)
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
        default:
            Ncheck(0);
        }

        VulkanPipelineLayout* VulkanLayout = ResourceCast(PipelineLayout);
        std::vector<VkWriteDescriptorSet> WriteVec;
        std::vector<VkDescriptorSet> DescriptorSetHandles;
        for (auto& [SetIndex, DescriptorSet] : DescriptorSets)
        {
            Ncheck(DescriptorSet);
            VulkanDescriptorSet* VulkanDescriptorSet = ResourceCast(const_cast<RHIDescriptorSet*>(DescriptorSet));
            DescriptorSetHandles.push_back(VulkanDescriptorSet->Handle);
            for (auto& [BindingIndex, Info] : VulkanDescriptorSet->Writers)
            {
                WriteVec.push_back(Info.WriteDescriptor);
            }  
        }  
        
        vkUpdateDescriptorSets(Device, WriteVec.size(), WriteVec.data(), 0, nullptr);
        vkCmdBindDescriptorSets(
            Handle, BindPoint, 
            VulkanLayout->Handle, 0, DescriptorSetHandles.size(), DescriptorSetHandles.data(), 0, nullptr);
    }

    void VulkanCommandBuffer::CopyBuffer(RHIBuffer* SrcBuffer, RHIBuffer* DstBuffer, uint32 SrcOffset, uint32 DstOffset, uint32 NumBytes)
    {
        VkBufferCopy Region{};
        Region.srcOffset = SrcOffset;
        Region.dstOffset = DstOffset;
        Region.size = NumBytes;
        VulkanBuffer* vkBufferSrc = ResourceCast(SrcBuffer);
        VulkanBuffer* vkBufferDst = ResourceCast(DstBuffer);
        vkCmdCopyBuffer(
            Handle, vkBufferSrc->Handle, vkBufferDst->Handle, 
            1, &Region);
    }

    void VulkanCommandBuffer::CopyBufferToImage(
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
        VulkanTexture* vkTexture = ResourceCast(DstTexture);
        vkCmdCopyBufferToImage(
            Handle, vkBuffer->Handle, vkTexture->Handle, 
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Region);
    }

    void VulkanCommandBuffer::BindVertexBuffer(int32 BindingPoint, RHIBuffer* Buffer, uint64 Offset)
    {
        VkBuffer vkBuffer = ResourceCast(Buffer)->Handle;
        vkCmdBindVertexBuffers(Handle, BindingPoint, 1, &vkBuffer, &Offset);
    }

    void VulkanCommandBuffer::BindIndexBuffer(RHIBuffer* Buffer, uint64 Offset)
    {
        VkBuffer vkBuffer = ResourceCast(Buffer)->Handle;
        vkCmdBindIndexBuffer(Handle, vkBuffer, Offset, VK_INDEX_TYPE_UINT32);
    }

    void VulkanCommandBuffer::PushConstants(RHIPipelineLayout* PipelineLayout, EShaderStage StageFlags, uint32 Offset, uint32 Size, const void* Data)
    {
        VulkanPipelineLayout* VulkanLayout = static_cast<VulkanPipelineLayout*>(PipelineLayout);
        VkShaderStageFlags stageFlags = (VkShaderStageFlags)StageFlags;
        vkCmdPushConstants(
            Handle, 
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
        VkBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
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
        VkBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
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

    void VulkanCommandBuffer::PipelineBarrier(
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
        vkCmdPipelineBarrier2(Handle, &DependencyInfo);
    }

    void VulkanCommandBuffer::RefreshState()
    {
        if (State == EState::Submitted)
        {
            VkResult result = vkGetFenceStatus(Device, Fence);
            if (result == VK_SUCCESS) 
            {
                for (RHIBuffer* Buffer : StagingBuffers)
                {
                    StagingManager->ReleaseBuffer(Buffer);
                }
                StagingBuffers.clear();
                vkResetCommandBuffer(Handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
                State = EState::ReadyForBegin;
            }
        }
    }

    VulkanCommandBufferPool::VulkanCommandBufferPool(VkDevice InDevice, VkQueue InQueue, int32 QueueFamilyIndex)
        : Device(InDevice)
        , Queue(InQueue)
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = QueueFamilyIndex;
        VK_CHECK_RESULT(vkCreateCommandPool(Device, &poolInfo, nullptr, &Handle));
    }

    VulkanCommandBufferPool::~VulkanCommandBufferPool()
    {
        vkDestroyCommandPool(Device, Handle, nullptr);
    }

    VulkanCommandBuffer* VulkanCommandBufferPool::Allocate()
    {
        for (int i = FreeCmdBuffers.size()-1; i >= 0; i--)
        {
            TRefCountPtr<VulkanCommandBuffer> RHICmdList = FreeCmdBuffers[i];
            CmdBuffers.push_back(FreeCmdBuffers[i]);
            FreeCmdBuffers.pop_back();
            return RHICmdList;
        }

        TRefCountPtr<VulkanCommandBuffer> RHICmdList = new VulkanCommandBuffer(Device, Queue, Handle);
        CmdBuffers.push_back(RHICmdList);
        return RHICmdList;
    }

    void VulkanCommandBufferPool::FreeUnusedCmdBuffers()
    {
        for (int i = CmdBuffers.size()-1; i >= 0; i--)
        {
            TRefCountPtr<VulkanCommandBuffer> RHICmdList = CmdBuffers[i];
            RHICmdList->RefreshState();
            if (RHICmdList->State == VulkanCommandBuffer::EState::ReadyForBegin)
            {
                std::swap(CmdBuffers[CmdBuffers.size()-1], CmdBuffers[i]);
                CmdBuffers.resize(CmdBuffers.size()-1);
                FreeCmdBuffers.push_back(RHICmdList);
            }
        }
    }

    RHICommandList* FVulkanDynamicRHI::RHICreateGfxCommandList()
    {
        return RHICreateCommandList(Device->GfxCmdBufferPool);
    }

    RHICommandList* FVulkanDynamicRHI::RHICreateComputeCommandList()
    {
        return RHICreateCommandList(Device->ComputeCmdBufferPool);
    }

    RHICommandList* FVulkanDynamicRHI::RHICreateTransferCommandList()
    {
        return RHICreateCommandList(Device->TransferCmdBufferPool);
    }

    RHICommandList* FVulkanDynamicRHI::RHICreateCommandList(VulkanCommandBufferPool* Pool)
    {
        VulkanCommandBuffer* VulkanCmdList = Pool->Allocate();
        Ncheck(VulkanCmdList->State == VulkanCommandBuffer::EState::ReadyForBegin);
        VkCommandBufferBeginInfo CmdBufBeginInfo{};
        CmdBufBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        CmdBufBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK_RESULT(vkBeginCommandBuffer(VulkanCmdList->Handle, &CmdBufBeginInfo));
        VulkanCmdList->State = VulkanCommandBuffer::EState::IsInsideBegin;
        return VulkanCmdList;
    }

    void FVulkanDynamicRHI::RHISubmitCommandList(RHICommandList* RHICmdList, const std::vector<RHISemaphoreRef>& SemaphoresToWait, const std::vector<RHISemaphoreRef>& SemaphoresToSignal)
    {
        VulkanCommandBuffer* VulkanCmdList = ResourceCast(RHICmdList);
        Ncheck(VulkanCmdList->IsOutsideRenderPass() && VulkanCmdList->HasBegun());
        VK_CHECK_RESULT(vkEndCommandBuffer(VulkanCmdList->Handle));
        std::vector<VkSemaphoreSubmitInfo> WaitSemephores;
        std::vector<VkSemaphoreSubmitInfo> SignalSemephores;
        for (RHISemaphoreRef Semaphore : SemaphoresToWait)
        {
            VulkanSemaphore* VkSemaphore = ResourceCast(Semaphore);
            VkSemaphoreSubmitInfo SemaphoreInfo{};
            SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            SemaphoreInfo.semaphore = VkSemaphore->Handle;
            WaitSemephores.push_back(SemaphoreInfo);
        }
        for (RHISemaphoreRef Semaphore : SemaphoresToSignal)
        {
            VulkanSemaphore* VkSemaphore = ResourceCast(Semaphore);
            VkSemaphoreSubmitInfo SemaphoreInfo{};
            SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            SemaphoreInfo.semaphore = VkSemaphore->Handle;
            SignalSemephores.push_back(SemaphoreInfo);
        }
        VkSubmitInfo2 SubmitInfo{};
        SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        SubmitInfo.waitSemaphoreInfoCount = WaitSemephores.size();
        SubmitInfo.pWaitSemaphoreInfos = WaitSemephores.data();
        VkCommandBufferSubmitInfo CmdBufferInfo{};
        CmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        CmdBufferInfo.commandBuffer = VulkanCmdList->Handle;
        SubmitInfo.commandBufferInfoCount = 1;
        SubmitInfo.pCommandBufferInfos = &CmdBufferInfo;
        SubmitInfo.signalSemaphoreInfoCount = SignalSemephores.size();
        SubmitInfo.pSignalSemaphoreInfos = SignalSemephores.data();
        vkQueueSubmit2(VulkanCmdList->Queue, 1, &SubmitInfo, VulkanCmdList->Fence);
        VulkanCmdList->State = VulkanCommandBuffer::EState::Submitted;
    }

    RHIBuffer* VulkanCommandBuffer::AcquireStagingBuffer(uint32 Size)
    {
        RHIBuffer* Buffer = StagingManager->AcquireBuffer(Size);
        StagingBuffers.push_back(Buffer);
        return Buffer;
    }

}
