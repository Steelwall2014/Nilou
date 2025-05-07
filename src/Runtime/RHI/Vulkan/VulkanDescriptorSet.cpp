#include "VulkanDevice.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDynamicRHI.h"

namespace nilou {

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
    TRefCountPtr<VulkanDescriptorSetLayout> VulkanLayout = new VulkanDescriptorSetLayout(Device->Handle);
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(VulkanBindings.size());
    layoutInfo.pBindings = VulkanBindings.data();
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(Device->Handle, &layoutInfo, nullptr, &VulkanLayout->Handle));
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
	VK_CHECK_RESULT(vkCreateDescriptorPool(Device->Handle, &PoolInfo, nullptr, &VulkanPool->Handle));
	return VulkanPool;
}

}