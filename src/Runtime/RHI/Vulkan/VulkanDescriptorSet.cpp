#include "VulkanDevice.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDynamicRHI.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"

namespace nilou {

RHIDescriptorSetLayoutRef FVulkanDynamicRHI::RHICreateDescriptorSetLayout(const std::vector<RHIDescriptorSetLayoutBinding>& Bindings)
{
	// uint32 Hash = FCrc::MemCrc32(Bindings.data(), sizeof(RHIDescriptorSetLayoutBinding) * Bindings.size());
	// if (RHIDescriptorSetLayoutRef Found = UniqueDescriptorSetLayouts[Hash])
	// {
	// 	return Found;
	// }
    std::vector<VkDescriptorSetLayoutBinding> VulkanBindings(Bindings.size());
    for (int i = 0; i < Bindings.size(); i++)
    {
        const RHIDescriptorSetLayoutBinding& Binding = Bindings[i];
        VkDescriptorSetLayoutBinding& VulkanBinding = VulkanBindings[i];
        VulkanBinding.binding = Binding.BindingIndex;
        VulkanBinding.descriptorCount = Binding.DescriptorCount;
        VulkanBinding.descriptorType = static_cast<VkDescriptorType>(Binding.DescriptorType);
        VulkanBinding.stageFlags = VK_SHADER_STAGE_ALL;
        VulkanBinding.pImmutableSamplers = nullptr;
    }
    TRefCountPtr<VulkanDescriptorSetLayout> VulkanLayout = new VulkanDescriptorSetLayout(Device->Handle, Bindings);
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(VulkanBindings.size());
    layoutInfo.pBindings = VulkanBindings.data();
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(Device->Handle, &layoutInfo, nullptr, &VulkanLayout->Handle));
	// UniqueDescriptorSetLayouts[Hash] = VulkanLayout;
    return VulkanLayout;
}

RHIDescriptorPoolRef FVulkanDynamicRHI::RHICreateDescriptorPool(RHIDescriptorSetLayout* Layout, uint32 PoolSize)
{
	std::vector<VkDescriptorPoolSize> Types;
	for (uint32 TypeIndex = VK_DESCRIPTOR_TYPE_BEGIN_RANGE; TypeIndex <= VK_DESCRIPTOR_TYPE_END_RANGE; ++TypeIndex)
	{
		EDescriptorType DescriptorType = static_cast<EDescriptorType>(TypeIndex);
		uint32 NumTypesUsed = Layout->GetNumTypeUsed(DescriptorType);
		if (NumTypesUsed > 0)
		{
			VkDescriptorPoolSize& Type = Types.emplace_back();
			Type.type = static_cast<VkDescriptorType>(TypeIndex);
			Type.descriptorCount = NumTypesUsed * PoolSize;
		}
	}

	VkDescriptorPoolCreateInfo PoolInfo{};
	PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	PoolInfo.poolSizeCount = Types.size();
	PoolInfo.pPoolSizes = Types.data();
	PoolInfo.maxSets = PoolSize;

	VkDescriptorPool VulkanPoolHandle;
	VK_CHECK_RESULT(vkCreateDescriptorPool(Device->Handle, &PoolInfo, nullptr, &VulkanPoolHandle));
	TRefCountPtr<VulkanDescriptorPool> VulkanPool = new VulkanDescriptorPool(Device->Handle, VulkanPoolHandle, PoolSize, Layout);
	return VulkanPool;
}

VulkanDescriptorPool::VulkanDescriptorPool(VkDevice InDevice, VkDescriptorPool InHandle, int32 InPoolSize, RHIDescriptorSetLayout* InLayout)
    : Device(InDevice)
    , Handle(InHandle)
    , RHIDescriptorPool(InLayout)
{
	for (int i = 0; i < InPoolSize; i++)
	{
		VkDescriptorSetAllocateInfo AllocInfo{};
		AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		AllocInfo.descriptorPool = Handle;
		AllocInfo.descriptorSetCount = 1;
		AllocInfo.pSetLayouts = &ResourceCast(Layout)->Handle;

		TRefCountPtr<VulkanDescriptorSet> NewDescriptorSet = new VulkanDescriptorSet(this);
		VK_CHECK_RESULT(vkAllocateDescriptorSets(Device, &AllocInfo, &NewDescriptorSet->Handle));
		Sets.push_back(NewDescriptorSet);
		FreeSets.push_back(NewDescriptorSet);
	}
}

RHIDescriptorSet* VulkanDescriptorPool::Allocate()
{
	if (FreeSets.empty())
	{
		return nullptr;
	}
	VulkanDescriptorSet* Set = FreeSets.back();
	FreeSets.pop_back();
	UsedSets.push_back(Set);
	return Set;
}

void VulkanDescriptorPool::Free(RHIDescriptorSet* Set)
{
	VulkanDescriptorSet* VulkanSet = ResourceCast(Set);
	FreeSets.push_back(VulkanSet);
	UsedSets.erase(std::find(UsedSets.begin(), UsedSets.end(), VulkanSet));
}

bool VulkanDescriptorPool::CanAllocate() const
{
	return FreeSets.size() > 0;
}

void VulkanDescriptorSet::SetUniformBuffer(uint32 BindingIndex, RHIBuffer* Buffer)
{
	VulkanBuffer* VulkanBuffer = ResourceCast(Buffer);
	Ncheck(VulkanBuffer && VulkanBuffer->Handle != VK_NULL_HANDLE);

	VkDescriptorBufferInfo BufferInfo{};
	BufferInfo.buffer = VulkanBuffer->Handle;
	BufferInfo.offset = 0;
	BufferInfo.range = Buffer->GetSize();

	VkWriteDescriptorSet WriteDescriptor{};
	WriteDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteDescriptor.dstSet = Handle;
	WriteDescriptor.dstBinding = BindingIndex;
	WriteDescriptor.dstArrayElement = 0;
	WriteDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	WriteDescriptor.descriptorCount = 1;
	WriteDescriptor.pBufferInfo = &BufferInfo;

	vkUpdateDescriptorSets(Device, 1, &WriteDescriptor, 0, nullptr);
}

void VulkanDescriptorSet::SetStorageBuffer(uint32 BindingIndex, RHIBuffer* Buffer)
{
	VulkanBuffer* VulkanBuffer = ResourceCast(Buffer);
	Ncheck(VulkanBuffer && VulkanBuffer->Handle != VK_NULL_HANDLE);

	VkDescriptorBufferInfo BufferInfo{};
	BufferInfo.buffer = VulkanBuffer->Handle;
	BufferInfo.offset = 0;
	BufferInfo.range = Buffer->GetSize();

	VkWriteDescriptorSet WriteDescriptor{};
	WriteDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteDescriptor.dstSet = Handle;
	WriteDescriptor.dstBinding = BindingIndex;
	WriteDescriptor.dstArrayElement = 0;
	WriteDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	WriteDescriptor.descriptorCount = 1;
	WriteDescriptor.pBufferInfo = &BufferInfo;

	vkUpdateDescriptorSets(Device, 1, &WriteDescriptor, 0, nullptr);
}

void VulkanDescriptorSet::SetSampler(uint32 BindingIndex, RHITextureView* InTexture, RHISamplerState* InSamplerState)
{
	VulkanSamplerState* SamplerState = ResourceCast(InSamplerState);
	VulkanTextureView* Texture = ResourceCast(InTexture);
	Ncheck(SamplerState && SamplerState->Handle != VK_NULL_HANDLE);
	Ncheck(Texture && Texture->Handle != VK_NULL_HANDLE);

	VkDescriptorImageInfo ImageInfo{};
	ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	ImageInfo.imageView = Texture->Handle;
	ImageInfo.sampler = SamplerState->Handle;

	VkWriteDescriptorSet WriteDescriptor{};
	WriteDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteDescriptor.dstSet = Handle;
	WriteDescriptor.dstBinding = BindingIndex;
	WriteDescriptor.dstArrayElement = 0;
	WriteDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	WriteDescriptor.descriptorCount = 1;
	WriteDescriptor.pImageInfo = &ImageInfo;

	vkUpdateDescriptorSets(Device, 1, &WriteDescriptor, 0, nullptr);
}

void VulkanDescriptorSet::SetStorageImage(uint32 BindingIndex, RHITextureView* InTexture)
{
	VulkanTextureView* Texture = ResourceCast(InTexture);
	Ncheck(Texture && Texture->Handle != VK_NULL_HANDLE);

	VkDescriptorImageInfo ImageInfo{};
	ImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	ImageInfo.imageView = Texture->Handle;

	VkWriteDescriptorSet WriteDescriptor{};
	WriteDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteDescriptor.dstSet = Handle;
	WriteDescriptor.dstBinding = BindingIndex;
	WriteDescriptor.dstArrayElement = 0;
	WriteDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	WriteDescriptor.descriptorCount = 1;
	WriteDescriptor.pImageInfo = &ImageInfo;

	vkUpdateDescriptorSets(Device, 1, &WriteDescriptor, 0, nullptr);
}

}