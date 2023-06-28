#include "VulkanDescriptorSet.h"

namespace nilou {

void FVulkanDescriptorSetsLayout::GenerateHash()
{
	const int32 LayoutCount = SetLayouts.size();

	for (int32 layoutIndex = 0; layoutIndex < LayoutCount; ++layoutIndex)
	{
		SetLayouts[layoutIndex].GenerateHash();
		Hash = FCrc::MemCrc32(&SetLayouts[layoutIndex].Hash, sizeof(uint32), Hash);
	}
}

FVulkanDescriptorPool::FVulkanDescriptorPool(VkDevice InDevice, const FVulkanDescriptorSetsLayout& InLayout, uint32 MaxSetsAllocations)
    : Device(InDevice)
    , Layout(InLayout)
    , MaxDescriptorSets(MaxSetsAllocations * Layout.SetLayouts.size())
{
	std::vector<VkDescriptorPoolSize> Types;
	for (uint32 TypeIndex = VK_DESCRIPTOR_TYPE_BEGIN_RANGE; TypeIndex <= VK_DESCRIPTOR_TYPE_END_RANGE; ++TypeIndex)
	{
		VkDescriptorType DescriptorType = (VkDescriptorType)TypeIndex;
		uint32 NumTypesUsed = Layout.GetTypesUsed(DescriptorType);
		if (NumTypesUsed > 0)
		{
			VkDescriptorPoolSize& Type = Types.emplace_back();
			Type.type = DescriptorType;
			Type.descriptorCount = NumTypesUsed * MaxDescriptorSets;
		}
	}

	VkDescriptorPoolCreateInfo PoolInfo{};
	PoolInfo.poolSizeCount = Types.size();
	PoolInfo.pPoolSizes = Types.data();
	PoolInfo.maxSets = MaxDescriptorSets;
}

FVulkanDescriptorPool::~FVulkanDescriptorPool()
{
	if (DescriptorPool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(Device, DescriptorPool, nullptr);
		DescriptorPool = VK_NULL_HANDLE;
	}
}

void FVulkanDescriptorPool::TrackAddUsage(const FVulkanDescriptorSetsLayout& InLayout)
{
	NumAllocatedDescriptorSets += InLayout.SetLayouts.size();
	PeakAllocatedDescriptorSets = std::max(NumAllocatedDescriptorSets, PeakAllocatedDescriptorSets);
}

void FVulkanDescriptorPool::TrackRemoveUsage(const FVulkanDescriptorSetsLayout& InLayout)
{
	NumAllocatedDescriptorSets -= InLayout.SetLayouts.size();
}

void FVulkanDescriptorPool::Reset()
{
	if (DescriptorPool != VK_NULL_HANDLE)
	{
		vkResetDescriptorPool(Device, DescriptorPool, 0);
	}

	NumAllocatedDescriptorSets = 0;
}

bool FVulkanDescriptorPool::AllocateDescriptorSets(const VkDescriptorSetAllocateInfo& InDescriptorSetAllocateInfo, VkDescriptorSet* OutSets)
{
	VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo = InDescriptorSetAllocateInfo;
	DescriptorSetAllocateInfo.descriptorPool = DescriptorPool;

	return VK_SUCCESS == vkAllocateDescriptorSets(Device, &DescriptorSetAllocateInfo, OutSets);
}

}