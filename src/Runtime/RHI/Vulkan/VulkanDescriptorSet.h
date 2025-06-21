#pragma once
#include <vulkan/vulkan.h>
#include <map>
#include <unordered_map>
#include <vector>
#include <memory>
#include "Platform.h"
#include "Common/Crc.h"
#include "RHIResources.h"

#define VK_DESCRIPTOR_TYPE_BEGIN_RANGE VK_DESCRIPTOR_TYPE_SAMPLER
#define VK_DESCRIPTOR_TYPE_END_RANGE VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT

namespace nilou {

class VulkanDescriptorSetLayout : public RHIDescriptorSetLayout
{
public:
    VulkanDescriptorSetLayout(VkDevice InDevice, const std::vector<RHIDescriptorSetLayoutBinding>& InBindings) 
        : RHIDescriptorSetLayout(InBindings)
        , Device(InDevice)
    { }
    VkDevice Device;
	VkDescriptorSetLayout Handle;

    virtual ~VulkanDescriptorSetLayout()
    {
        vkDestroyDescriptorSetLayout(Device, Handle, nullptr);
    }
};
using VulkanDescriptorSetLayoutRef = TRefCountPtr<VulkanDescriptorSetLayout>;

inline VulkanDescriptorSetLayout* ResourceCast(RHIDescriptorSetLayout* SetLayout)
{
	return static_cast<VulkanDescriptorSetLayout*>(SetLayout);
}

class VulkanDescriptorPool : public RHIDescriptorPool
{
public:
	VulkanDescriptorPool(VkDevice InDevice, VkDescriptorPool InHandle, int32 InPoolSize, RHIDescriptorSetLayout* InLayout);
	VkDescriptorPool Handle;
    VkDevice Device;

    // RHIDescriptorPool interface
    virtual RHIDescriptorSet* Allocate() override;
    virtual void Free(RHIDescriptorSet* DescriptorSet) override;
    virtual bool CanAllocate() const override;
    // End of RHIDescriptorPool interface

    std::vector<TRefCountPtr<class VulkanDescriptorSet>> Sets;
    std::vector<VulkanDescriptorSet*> FreeSets;
    std::vector<VulkanDescriptorSet*> UsedSets;
};

class VulkanDescriptorSet : public RHIDescriptorSet
{
public:

    VulkanDescriptorSet(VulkanDescriptorPool* Pool) 
        : RHIDescriptorSet(Pool)
        , Device(Pool->Device)
    { }

    virtual void SetUniformBuffer(uint32 BindingIndex, RHIBuffer* Buffer) override;

    virtual void SetStorageBuffer(uint32 BindingIndex, RHIBuffer* Buffer) override;

    virtual void SetSampler(uint32 BindingIndex, RHITextureView* Texture, RHISamplerState* SamplerState) override;

    virtual void SetStorageImage(uint32 BindingIndex, RHITextureView* InTexture) override;

    VkDescriptorSet Handle;
    VkDevice Device;
private:

    struct VulkanDescriptorSetWriter
    {
        VkDescriptorBufferInfo BufferInfo{};
        VkDescriptorImageInfo ImageInfo{};
        VkWriteDescriptorSet WriteDescriptor;
    };

    std::unordered_map<uint8, VulkanDescriptorSetWriter> Writers;

    friend class VulkanCommandBuffer;

};

inline VulkanDescriptorSet* ResourceCast(RHIDescriptorSet* DescriptorSet)
{
    return static_cast<VulkanDescriptorSet*>(DescriptorSet);
}

}