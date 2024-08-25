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
	VkDescriptorSetLayout Handle;
};

class VulkanDescriptorPool : public RHIDescriptorPool
{
public:
	VulkanDescriptorPool(RHIDescriptorSetLayout* InLayout) : RHIDescriptorPool(InLayout) { }
	VkDescriptorPool Handle;
};

class VulkanDescriptorSet : public RHIDescriptorSet
{
public:

    friend class VulkanCommandList;

    virtual void SetUniformBuffer(uint32 BindingIndex, RHIBuffer* Buffer) override;

    virtual void SetStorageBuffer(uint32 BindingIndex, RHIBuffer* Buffer) override;

    virtual void SetSampler(uint32 BindingIndex, RHISampler Sampler) override;

    virtual void SetStorageImage(uint32 BindingIndex, RHITexture* Image) override;

    VkDescriptorSet Handle;
    VkDescriptorSetLayout LayoutHandle;

private:

    struct VulkanDescriptorSetWriter
    {
        VkDescriptorBufferInfo BufferInfo{};
        VkDescriptorImageInfo ImageInfo{};
        VkWriteDescriptorSet WriteDescriptor;
    };

    std::unordered_map<uint8, VulkanDescriptorSetWriter> Writers;

};

inline VulkanDescriptorSet* ResourceCast(RHIDescriptorSet* DescriptorSet)
{
    return static_cast<VulkanDescriptorSet*>(DescriptorSet);
}


// all below DEPRECATED
class FVulkanDescriptorSetsLayout
{

public:
	FVulkanDescriptorSetsLayout()
	{
		// Add expected descriptor types
		for (uint32 i = VK_DESCRIPTOR_TYPE_BEGIN_RANGE; i <= VK_DESCRIPTOR_TYPE_END_RANGE; ++i)
		{
			LayoutTypes[static_cast<VkDescriptorType>(i)] = 0;
		}
	}
	FVulkanDescriptorSetsLayout(VkDevice InDevice, const std::vector<std::vector<VkDescriptorSetLayoutBinding>>& SetsLayoutBindings)
		: Device(InDevice)
	{
		SetLayouts.resize(SetsLayoutBindings.size());
		Handles.resize(SetsLayoutBindings.size());
		for (int i = 0; i < SetsLayoutBindings.size(); i++)
		{
			auto& SetLayoutBindings = SetsLayoutBindings[i];
			FSetLayout& Layout = SetLayouts[i];
			Layout.LayoutBindings = SetLayoutBindings;
			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = static_cast<uint32_t>(SetLayoutBindings.size());
			layoutInfo.pBindings = SetLayoutBindings.data();
			VkResult res = vkCreateDescriptorSetLayout(Device, &layoutInfo, nullptr, &Handles[i]);
			if (res != VK_SUCCESS)
			{
				NILOU_LOG(Error, "vkCreateDescriptorSetLayout failed!");
			}
			for (auto& Binding : SetLayoutBindings)
			{
				LayoutTypes[static_cast<VkDescriptorType>(Binding.descriptorType)] += 1;
			}
		}

		GenerateHash();
	}

	uint32 GetTypesUsed(VkDescriptorType Type) const
	{
		if (LayoutTypes.contains(Type))
		{
			return LayoutTypes.find(Type)->second;
		}
		else
		{
			return 0;
		}
	}
	struct FSetLayout
	{
		std::vector<VkDescriptorSetLayoutBinding> LayoutBindings;
		uint32 Hash;

		inline void GenerateHash()
		{
			Hash = FCrc::MemCrc32(LayoutBindings.data(), sizeof(VkDescriptorSetLayoutBinding) * LayoutBindings.size());
		}

		friend uint32 GetTypeHash(const FSetLayout& In)
		{
			return In.Hash;
		}

		inline bool operator == (const FSetLayout& In) const
		{
			if (In.Hash != Hash)
			{
				return false;
			}

			const int32 NumBindings = LayoutBindings.size();
			if (In.LayoutBindings.size() != NumBindings)
			{
				return false;
			}

			if (NumBindings != 0 && std::memcmp(In.LayoutBindings.data(), LayoutBindings.data(), NumBindings * sizeof(VkDescriptorSetLayoutBinding)) != 0)
			{
				return false;
			}

			return true;
		}

		inline bool operator != (const FSetLayout& In) const
		{
			return !(*this == In);
		}
	};

    void GenerateHash();

	inline bool operator == (const FVulkanDescriptorSetsLayout& In) const
	{
		if (In.Hash != Hash)
		{
			return false;
		}

		if (In.SetLayouts.size() != SetLayouts.size())
		{
			return false;
		}

		for (int32 Index = 0; Index < In.SetLayouts.size(); ++Index)
		{
			if (In.SetLayouts[Index] != SetLayouts[Index])
			{
				return false;
			}
		}

		return true;
	}

	VkDescriptorSetAllocateInfo GetAllocateInfo() const
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorSetCount = SetLayouts.size();
		allocInfo.pSetLayouts = Handles.data();
		return allocInfo;
	}

	std::map<VkDescriptorType, uint32> LayoutTypes;
	std::vector<FSetLayout> SetLayouts;
	VkDevice Device;
	std::vector<VkDescriptorSetLayout> Handles;

	uint32 Hash = 0;
};

class FVulkanDescriptorSets
{
public:
	FVulkanDescriptorSets(const FVulkanDescriptorSetsLayout& InLayout) 
		: Layout(InLayout)
		, Handles(InLayout.SetLayouts.size()){ }
	const FVulkanDescriptorSetsLayout& Layout;
	std::vector<VkDescriptorSet> Handles;
	class FVulkanDescriptorPool* Owner;
};

}

namespace std {
    template<>
    struct hash<nilou::FVulkanDescriptorSetsLayout>
    {
        size_t operator()(const nilou::FVulkanDescriptorSetsLayout &_Keyval) const noexcept {
            return _Keyval.Hash;
        }
    };
}

namespace nilou {

class FVulkanDescriptorPool
{
public:
	FVulkanDescriptorPool(VkDevice InDevice, const FVulkanDescriptorSetsLayout& Layout, uint32 MaxSetsAllocations);
	~FVulkanDescriptorPool();

	inline VkDescriptorPool GetHandle() const
	{
		return DescriptorPool;
	}

	inline bool CanAllocate(const FVulkanDescriptorSetsLayout& InLayout) const
	{
		return MaxDescriptorSets > NumAllocatedDescriptorSets + InLayout.SetLayouts.size();
	}

	void TrackAddUsage(const FVulkanDescriptorSetsLayout& InLayout);
	void TrackRemoveUsage(const FVulkanDescriptorSetsLayout& InLayout);

	inline bool IsEmpty() const
	{
		return NumAllocatedDescriptorSets == 0;
	}

	void Reset();
	bool AllocateDescriptorSets(const VkDescriptorSetAllocateInfo& InDescriptorSetAllocateInfo, VkDescriptorSet* OutSets);
	inline uint32 GetNumAllocatedDescriptorSets() const
	{
		return NumAllocatedDescriptorSets;
	}

private:
	VkDevice Device{};

	uint32 MaxDescriptorSets{};
	uint32 NumAllocatedDescriptorSets{};
	uint32 PeakAllocatedDescriptorSets{};

	// Tracks number of allocated types, to ensure that we are not exceeding our allocated limit
	const FVulkanDescriptorSetsLayout& Layout;
	VkDescriptorPool DescriptorPool{};
};

class FVulkanTypedDescriptorPoolSet
{
public:
	FVulkanTypedDescriptorPoolSet(VkDevice InDevice) : PoolCurrent(Pools.end()), Device(InDevice) { }
	bool AllocateDescriptorSets(const FVulkanDescriptorSetsLayout& InLayout, VkDescriptorSet* OutSets, FVulkanDescriptorPool** OutOwner);
	void Reset()
	{
		for (auto& Pool : Pools)
			Pool.Reset();
	}
private:
	VkDevice Device;
	std::list<FVulkanDescriptorPool> Pools{};
	std::list<FVulkanDescriptorPool>::iterator PoolCurrent{};
};

class FVulkanDescriptorPoolSetContainer
{
public:
	FVulkanDescriptorPoolSetContainer(VkDevice InDevice) : Device(InDevice) { }
	void Reset()
	{
		for (auto& [Layout, PoolSet] : TypedDescriptorPools)
			PoolSet->Reset();
	}

	inline void SetUsed(bool bInUsed)
	{
		bUsed = bInUsed;
	}

	inline bool IsUnused() const
	{
		return !bUsed;
	}

	VkDevice Device;
    std::unordered_map<FVulkanDescriptorSetsLayout, std::shared_ptr<FVulkanTypedDescriptorPoolSet>> TypedDescriptorPools;

	FVulkanTypedDescriptorPoolSet* AcquireTypedPoolSet(const FVulkanDescriptorSetsLayout& Layout);

	FVulkanDescriptorSets AllocateDescriptorSets(const FVulkanDescriptorSetsLayout& Layout);

	bool bUsed = true;
};

class FVulkanDescriptorPoolsManager
{
public:
	FVulkanDescriptorPoolsManager(VkDevice InDevice) : Device(InDevice) { }
	FVulkanDescriptorPoolSetContainer* AcquirePoolSetContainer();
	void ReleasePoolSet(FVulkanDescriptorPoolSetContainer* PoolSet);
	VkDevice Device;
	std::vector<std::unique_ptr<FVulkanDescriptorPoolSetContainer>> PoolSets;
};

/**
* PoolsManager作为descriptor管理的顶层，是一个单例类，放在VulkanDynamicRHI里
* 每次要开始使用ActiveCmdBuffer之前，从PoolsManager中分配出一个PoolSetContainer给ActiveCmdBuffer。
* 每次绑定PSO的时候，PoolSetContainer分配一个对应当前layout的DescriptorSets
* PoolSetContainer一个layout对应一个PoolSet，一个PoolSet中有很多Pool，DescriptorSets实际是从Pool中分配出来
* 这里名为DescriptorSets，但实际只分配了一个Set，所以可以看为DescriptorSet
*/

}