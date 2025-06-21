#pragma once
#include <unordered_set>
#include "RenderGraphResources.h"
#include "RHIResources.h"
#include "RHIStaticStates.h"

namespace nilou {

class RDGDescriptorSet : public RDGResource
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
    void SetSampler(const std::string& Name, RDGTextureView* Texture, RHISamplerState* SamplerState=TStaticSamplerState<SF_Trilinear>::GetRHI());
    void SetStorageBuffer(const std::string& Name, RDGBuffer* Buffer);
    void SetStorageImage(const std::string& Name, RDGTextureView* Image);

    RHIDescriptorSet* GetRHI() const { return static_cast<RHIDescriptorSet*>(ResourceRHI.GetReference()); }
    RHIDescriptorSetLayout* GetLayout() const { return Layout; }

private:

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