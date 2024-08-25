#pragma once
#include <vulkan/vulkan.h>
#include "RHIResources.h"

namespace nilou {

class VulkanVertexDeclaration : public FRHIVertexDeclaration
{
public:
    VkPipelineVertexInputStateCreateInfo VertexInputInfo{};
    std::vector<VkVertexInputBindingDescription> BindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> AttributeDescriptions;
};
using VulkanVertexDeclarationRef = std::shared_ptr<VulkanVertexDeclaration>;

}