#pragma once
#include <unordered_set>
#include "RenderGraphResources.h"
#include "RHIResources.h"

namespace nilou {

class RENDERCORE_API RDGDescriptorSet : public RDGResource
{
public:

    friend class RHIDescriptorSetPools;
    friend class RenderGraph;

    RDGDescriptorSet(const std::string& Name, RHIDescriptorSetLayout* InLayout) 
        : RDGResource(Name, ERDGResourceType::DescriptorSet) 
        , Layout(InLayout)
    { 
        Ncheck(Layout);
    }
    ~RDGDescriptorSet();

    void SetUniformBuffer(const std::string& Name, RDGBuffer* Buffer);
    void SetSampler(const std::string& Name, RDGTextureView* Texture, RHISamplerState* SamplerState=nullptr);
    void SetStorageBuffer(const std::string& Name, RDGBuffer* Buffer);
    void SetStorageImage(const std::string& Name, RDGTextureView* Image);

    void SetUniformBuffer(int32 BindingIndex, RDGBuffer* Buffer);
    void SetCombinedTextureSampler(int32 BindingIndex, RDGTextureView* Texture, RHISamplerState* SamplerState=nullptr);
    void SetStorageBuffer(int32 BindingIndex, RDGBuffer* Buffer);
    void SetStorageImage(int32 BindingIndex, RDGTextureView* Image);
    void SetSamplerState(int32 BindingIndex, RHISamplerState* SamplerState);

    RHIDescriptorSet* GetRHI() const { return static_cast<RHIDescriptorSet*>(ResourceRHI.GetReference()); }
    RHIDescriptorSetLayout* GetLayout() const { return Layout; }

private:

    void SetUniformBufferInternal(const RHIDescriptorSetLayoutBinding& Binding, RDGBuffer* Buffer);
    void SetSamplerInternal(const RHIDescriptorSetLayoutBinding& Binding, RDGTextureView* Texture, RHISamplerState* SamplerState=nullptr);
    void SetStorageBufferInternal(const RHIDescriptorSetLayoutBinding& Binding, RDGBuffer* Buffer);
    void SetStorageImageInternal(const RHIDescriptorSetLayoutBinding& Binding, RDGTextureView* Image);
    void SetSamplerStateInternal(const RHIDescriptorSetLayoutBinding& Binding, RHISamplerState* SamplerState);

    struct DescriptorBufferInfo
    {
        RDGBuffer* Buffer = nullptr;
        uint32 Offset = 0;
        uint32 Range = 0;
    };

    struct DescriptorImageInfo
    {
        RHISamplerState* SamplerState = nullptr;
        RDGTextureView* Texture = nullptr;
    };

    struct WriteDescriptorSet
    {
        uint32 DstBinding;
        uint32 DstArrayElement;
        EDescriptorType DescriptorType;
        DescriptorImageInfo ImageInfo;
        DescriptorBufferInfo BufferInfo;
        ERHIAccess Access = ERHIAccess::None;
    };

    std::map<uint32, WriteDescriptorSet> WriterInfos;

    RHIDescriptorSetPools* Pools = nullptr;

    uint32 SetIndex = 0;

    RHIDescriptorSetLayout* Layout;

    std::optional<RHIDescriptorSetLayoutBinding> GetBindingByName(const std::string& Name)
    {
        for (auto& Binding : Layout->Bindings)
        {
            if (Binding.Name == Name)
            {
                return Binding;
            }
        }
        return std::nullopt;
    }

    std::optional<RHIDescriptorSetLayoutBinding> GetBindingByIndex(int32 BindingIndex)
    {
        for (auto& Binding : Layout->Bindings)
        {
            if (Binding.BindingIndex == BindingIndex)
            {
                return Binding;
            }
        }
        return std::nullopt;
    }

};
using RDGDescriptorSetRef = TRefCountPtr<RDGDescriptorSet>;

// class RDGDescriptorPool
// {
// public:
//     RDGDescriptorPool(RHIDescriptorSetLayout* InLayout, uint32 InMaxNumDescriptorSets);

//     std::vector<RHIDescriptorSetRef> DescriptorSets;
//     std::list<RHIDescriptorSet*> AllocatedDescriptorSets;
//     std::list<RHIDescriptorSet*> VacantDescriptorSets;

//     RHIDescriptorSet* Allocate();

//     RHIDescriptorPoolRef PoolRHI;
// };

class RHIDescriptorSetPools
{
public:
    RHIDescriptorSetPools(RHIDescriptorSetLayout* InLayout)
        : Layout(InLayout)
    { }

    RHIDescriptorSet* Allocate();
    void Release(RHIDescriptorSet* DescriptorSet);

private:

    std::vector<RHIDescriptorPoolRef> PoolsRHI;
    std::vector<RHIDescriptorPool*> VacantPoolsRHI;
    std::unordered_set<RHIDescriptorPool*> FullPoolsRHI;

    RHIDescriptorSetLayout* Layout = nullptr;
    
	uint32 NumAllocatedDescriptorSets = 0;
};

}