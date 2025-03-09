#include "VulkanDevice.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDynamicRHI.h"

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
    , MaxDescriptorSets(MaxSetsAllocations * InLayout.SetLayouts.size())
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
	PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	PoolInfo.poolSizeCount = Types.size();
	PoolInfo.pPoolSizes = Types.data();
	PoolInfo.maxSets = MaxDescriptorSets;

	vkCreateDescriptorPool(Device, &PoolInfo, nullptr, &DescriptorPool);
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
		VkResult res = vkResetDescriptorPool(Device, DescriptorPool, 0);
		if (res != VK_SUCCESS)
		{
			NILOU_LOG(Error, "vkResetDescriptorPool failed!");
		}
	}

	NumAllocatedDescriptorSets = 0;
}

bool FVulkanDescriptorPool::AllocateDescriptorSets(const VkDescriptorSetAllocateInfo& InDescriptorSetAllocateInfo, VkDescriptorSet* OutSets)
{
	VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo = InDescriptorSetAllocateInfo;
	DescriptorSetAllocateInfo.descriptorPool = DescriptorPool;

	return VK_SUCCESS == vkAllocateDescriptorSets(Device, &DescriptorSetAllocateInfo, OutSets);
}

bool FVulkanTypedDescriptorPoolSet::AllocateDescriptorSets(const FVulkanDescriptorSetsLayout& InLayout, VkDescriptorSet* OutSets, FVulkanDescriptorPool** OutOwner)
{
	if (!InLayout.Handles.empty())
	{
		if (PoolCurrent == Pools.end())
		{
			Pools.emplace_back(Device, InLayout, 32);
			PoolCurrent = Pools.begin();
		}
		while (!PoolCurrent->CanAllocate(InLayout) || !PoolCurrent->AllocateDescriptorSets(InLayout.GetAllocateInfo(), OutSets))
		{
			Pools.emplace_back(Device, InLayout, 32);
			PoolCurrent = std::next(PoolCurrent);
		}

		PoolCurrent->TrackAddUsage(InLayout);

		*OutOwner = &(*PoolCurrent);

		return true;
	}

	return false;
}

FVulkanTypedDescriptorPoolSet* FVulkanDescriptorPoolSetContainer::AcquireTypedPoolSet(const FVulkanDescriptorSetsLayout& Layout)
{
	auto Found = TypedDescriptorPools.find(Layout);
	if (Found == TypedDescriptorPools.end())
	{
		Found = TypedDescriptorPools.insert({Layout, std::make_shared<FVulkanTypedDescriptorPoolSet>(Device)}).first;
	}
	FVulkanTypedDescriptorPoolSet* PoolSet = Found->second.get();
	return PoolSet;
	
}

FVulkanDescriptorSets FVulkanDescriptorPoolSetContainer::AllocateDescriptorSets(const FVulkanDescriptorSetsLayout& Layout)
{
	FVulkanDescriptorSets Sets{Layout};
	AcquireTypedPoolSet(Layout)->AllocateDescriptorSets(Layout, Sets.Handles.data(), &Sets.Owner);
	return Sets;
}

FVulkanDescriptorPoolSetContainer* FVulkanDescriptorPoolsManager::AcquirePoolSetContainer()
{
	for (auto& PoolSet : PoolSets)
	{
		if (PoolSet->IsUnused())
		{
			PoolSet->SetUsed(true);
			return PoolSet.get();
		}
	}

	FVulkanDescriptorPoolSetContainer* PoolSet = new FVulkanDescriptorPoolSetContainer(Device);
	PoolSets.push_back(std::unique_ptr<FVulkanDescriptorPoolSetContainer>(PoolSet));

	return PoolSet;
}

void FVulkanDescriptorPoolsManager::ReleasePoolSet(FVulkanDescriptorPoolSetContainer* PoolSetContainer)
{
	PoolSetContainer->Reset();
	PoolSetContainer->SetUsed(false);
}

RHIDescriptorSetLayoutRef FVulkanDynamicRHI::RHICreateDescriptorSetLayout(const std::vector<RHIDescriptorSetLayoutBinding>& Bindings)
{
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
    TRefCountPtr<VulkanDescriptorSetLayout> VulkanLayout = new VulkanDescriptorSetLayout();
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(VulkanBindings.size());
    layoutInfo.pBindings = VulkanBindings.data();
    VkResult res = vkCreateDescriptorSetLayout(Device->Handle, &layoutInfo, nullptr, &VulkanLayout->Handle);
    if (res != VK_SUCCESS)
    {
        NILOU_LOG(Error, "vkCreateDescriptorSetLayout failed!");
        return nullptr;
    }
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

	TRefCountPtr<VulkanDescriptorPool> VulkanPool = new VulkanDescriptorPool(Layout);
	VkResult res = vkCreateDescriptorPool(Device->Handle, &PoolInfo, nullptr, &VulkanPool->Handle);
    if (res != VK_SUCCESS)
    {
        NILOU_LOG(Error, "vkCreateDescriptorPool failed!");
        return nullptr;
    }
	return VulkanPool;
}

}