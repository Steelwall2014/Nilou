#pragma once
#include "RenderGraphResources.h"
#include "RHIResources.h"
#include "RHIStaticStates.h"

namespace nilou {

class RDGDescriptorSet : public RDGResource
{
public:

    friend class RDGDescriptorSetPool;

    ~RDGDescriptorSet();

    void SetUniformBuffer(const std::string& Name, RDGBuffer* Buffer);
    void SetSampler(const std::string& Name, RDGTextureView* Texture, RHISamplerState* SamplerState=TStaticSamplerState<SF_Trilinear>::GetRHI());
    void SetStorageBuffer(const std::string& Name, RDGBuffer* Buffer);
    void SetStorageImage(const std::string& Name, RDGTextureView* Image);

    void SetUniformBuffer(uint32 BindingIndex, RDGBuffer* Buffer);
    void SetSampler(uint32 BindingIndex, RDGTextureView* Texture, RHISamplerState* SamplerState=TStaticSamplerState<SF_Trilinear>::GetRHI());
    void SetStorageBuffer(uint32 BindingIndex, RDGBuffer* Buffer);
    void SetStorageImage(uint32 BindingIndex, RDGTextureView* Image);

    RHIDescriptorSet* GetRHI() const;

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
    };

    std::map<uint32, WriteDescriptorSet> WriterInfos;

private:

    RHIDescriptorSet* DescriptorSetRHI;
    RDGDescriptorSetPool* Pool = nullptr;

};
using RDGDescriptorSetRef = std::shared_ptr<RDGDescriptorSet>;

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
    RDGDescriptorSetPool(RHIDescriptorSetLayout* InLayout);

    std::vector<RHIDescriptorPoolRef> PoolsRHI;

    std::vector<RHIDescriptorPool*> VacantPools;
    std::list<RHIDescriptorPool*> FullPools;

    RDGDescriptorSetRef Allocate();
    void Release(RDGDescriptorSet* Set);

    bool CanAllocate() const;

    uint32 MaxNumDescriptorSets;

	uint32 NumAllocatedDescriptorSets = 0;

    RHIDescriptorSetLayout* Layout = nullptr;
    
    std::vector<uint32> NumAllocatedSetsPerPool;

    std::map<RHIDescriptorPool*, std::list<RHIDescriptorPool*>::iterator> PoolIterators;

};

}