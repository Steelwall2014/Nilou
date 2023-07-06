#pragma once
#include <vulkan/vulkan.h>
#include "RHIResources.h"

namespace nilou {

class VulkanVertexDeclaration : public FRHIVertexDeclaration
{
public:
    FVertexDeclarationElementList Elements;
};
using VulkanVertexDeclarationRef = std::shared_ptr<VulkanVertexDeclaration>;

}