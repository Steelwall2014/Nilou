#pragma once
#include <unordered_set>
#include "RenderGraphResources.h"
#include "RHIResources.h"
#include "RHIStaticStates.h"

namespace nilou {

class RDGDescriptorSet : public RDGResource
{
public:

    friend class RDGDescriptorSetPool;
    friend class RenderGraph;

    RDGDescriptorSet(const std::string& Name, RHIDescriptorSetLayout* InLayout) 
        : RDGResource(Name, ERDGResourceType::DescriptorSet) 
        , Layout(InLayout)
    { }
    ~RDGDescriptorSet();

    void SetUniformBuffer(const std::string& Name, RDGBuffer* Buffer)
    {
        if (auto Binding = GetBindingByName(Name))
        {
            Ncheck(Binding->DescriptorType == EDescriptorType::UniformBuffer);
            SetUniformBuffer(Binding->BindingIndex, Buffer);
        }
    }
    void SetSampler(const std::string& Name, RDGTextureView* Texture, RHISamplerState* SamplerState=TStaticSamplerState<SF_Trilinear>::GetRHI())
    {
        if (auto Binding = GetBindingByName(Name))
        {
            Ncheck(Binding->DescriptorType == EDescriptorType::CombinedImageSampler);
            SetSampler(Binding->BindingIndex, Texture, SamplerState);
        }
    }
    void SetStorageBuffer(const std::string& Name, RDGBuffer* Buffer, ERHIAccess Access)
    {
        if (auto Binding = GetBindingByName(Name))
        {
            SetStorageBuffer(Binding->BindingIndex, Buffer, Access);
        }
    }
    void SetStorageImage(const std::string& Name, RDGTextureView* Image, ERHIAccess Access)
    {
        if (auto Binding = GetBindingByName(Name))
        {
            SetStorageImage(Binding->BindingIndex, Image, Access);
        }
    }

    void SetUniformBuffer(uint32 BindingIndex, RDGBuffer* Buffer);
    void SetSampler(uint32 BindingIndex, RDGTextureView* Texture, RHISamplerState* SamplerState=TStaticSamplerState<SF_Trilinear>::GetRHI());
    void SetStorageBuffer(uint32 BindingIndex, RDGBuffer* Buffer, ERHIAccess Access);
    void SetStorageImage(uint32 BindingIndex, RDGTextureView* Image, ERHIAccess Access);

    RHIDescriptorSet* GetRHI() const { return static_cast<RHIDescriptorSet*>(ResourceRHI.GetReference()); }

private:

    struct DescriptorBufferInfo
    {
        RDGBuffer* Buffer;
        uint32 Offset;
        uint32 Range;
    };

    struct DescriptorImageInfo
    {
        RHISamplerState* SamplerState;
        RDGTextureView* Texture;
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

    RDGDescriptorSetPool* Pool = nullptr;

    uint32 SetIndex = 0;

    std::map<std::string, RHIDescriptorSetLayoutBinding> NameToBinding;
    RHIDescriptorSetLayout* Layout;

    std::optional<RHIDescriptorSetLayoutBinding> GetBindingByName(const std::string& Name)
    {
        if (NameToBinding.find(Name) != NameToBinding.end())
        {
            return NameToBinding[Name];
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

class RDGDescriptorSetPool
{
public:
    RDGDescriptorSetPool() { }
    RDGDescriptorSetPool(RHIDescriptorSetLayout* InLayout)
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