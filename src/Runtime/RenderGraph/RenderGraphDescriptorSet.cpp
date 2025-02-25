#include "RenderGraphDescriptorSet.h"
#include "DynamicRHI.h"

namespace nilou {

constexpr int MAX_NUM_DESCRIPTORSETS_PER_POOL = 1024;

RDGDescriptorSet::~RDGDescriptorSet()
{
    if (Pool)
    {
        Pool->Release(this);
    }
}

void RDGDescriptorSet::SetUniformBuffer(uint32 BindingIndex, RDGBuffer* Buffer)
{
	DescriptorBufferInfo BufferInfo;
	BufferInfo.Buffer = Buffer;
	BufferInfo.Offset = 0;
	BufferInfo.Range = Buffer->Desc.GetSize();
	WriteDescriptorSet WriteDescriptor;
	WriteDescriptor.DstBinding = BindingIndex;
	WriteDescriptor.DstArrayElement = 0;
	WriteDescriptor.DescriptorType = EDescriptorType::UniformBuffer;
	WriteDescriptor.BufferInfo = BufferInfo;
	WriterInfos[BindingIndex] = WriteDescriptor;
}

void RDGDescriptorSet::SetSampler(uint32 BindingIndex, RDGTextureView* Texture, RHISamplerState* SamplerState)
{
	DescriptorImageInfo ImageInfo;
	ImageInfo.SamplerState = SamplerState;
	ImageInfo.Texture = Texture;
	WriteDescriptorSet WriteDescriptor;
	WriteDescriptor.DstBinding = BindingIndex;
	WriteDescriptor.DstArrayElement = 0;
	WriteDescriptor.DescriptorType = EDescriptorType::CombinedImageSampler;
	WriteDescriptor.ImageInfo = ImageInfo;
	WriterInfos[BindingIndex] = WriteDescriptor;
}

void RDGDescriptorSet::SetStorageBuffer(uint32 BindingIndex, RDGBuffer* Buffer, ERHIAccess Access)
{
	DescriptorBufferInfo BufferInfo;
	BufferInfo.Buffer = Buffer;
	BufferInfo.Offset = 0;
	BufferInfo.Range = Buffer->Desc.GetSize();
	WriteDescriptorSet WriteDescriptor;
	WriteDescriptor.DstBinding = BindingIndex;
	WriteDescriptor.DstArrayElement = 0;
	WriteDescriptor.DescriptorType = EDescriptorType::StorageBuffer;
	WriteDescriptor.BufferInfo = BufferInfo;
	WriterInfos[BindingIndex] = WriteDescriptor;
}

void RDGDescriptorSet::SetStorageImage(uint32 BindingIndex, RDGTextureView* Image, ERHIAccess Access)
{
	DescriptorImageInfo ImageInfo;
	ImageInfo.Texture = Image;
	WriteDescriptorSet WriteDescriptor;
	WriteDescriptor.DstBinding = BindingIndex;
	WriteDescriptor.DstArrayElement = 0;
	WriteDescriptor.DescriptorType = EDescriptorType::StorageImage;
	WriteDescriptor.ImageInfo = ImageInfo;
	WriterInfos[BindingIndex] = WriteDescriptor;
}

// RDGDescriptorPool::RDGDescriptorPool(RHIDescriptorSetLayout* InLayout, uint32 InMaxNumDescriptorSets)
// {
//     PoolRHI = RHICreateDescriptorPool(InLayout, InMaxNumDescriptorSets);
//     for (int i = 0; i < InMaxNumDescriptorSets; i++)
//     {
//         RDGDescriptorSetRef DescriptorSetRDG = std::make_shared<RDGDescriptorSet>();
//         DescriptorSetRDG->DescriptorSetRHI = RHIAllocateDescriptorSet(PoolRHI.get());
//         DescriptorSets.push_back(DescriptorSetRDG);
//         VacantDescriptorSets.push_back(DescriptorSetRDG.get());
//     }
// }

// RDGDescriptorSet* RDGDescriptorPool::Allocate()
// {
//     if (VacantDescriptorSets.size() == 0)
//     {
//         return nullptr;
//     }

//     RDGDescriptorSet* AllocatedDescriptorSet = VacantDescriptorSets.front();
//     VacantDescriptorSets.pop_front();
//     AllocatedDescriptorSets.push_back(AllocatedDescriptorSet);
//     return AllocatedDescriptorSet;
// }

RDGDescriptorSetPool::RDGDescriptorSetPool(RHIDescriptorSetLayout* InLayout)
    : Layout(InLayout)
{

}

RDGDescriptorSetRef RDGDescriptorSetPool::Allocate()
{
    if (VacantPoolsRHI.size() == 0)
    {
        RHIDescriptorPoolRef PoolRHI = RHICreateDescriptorPool(Layout, MAX_NUM_DESCRIPTORSETS_PER_POOL);
        PoolsRHI.push_back(PoolRHI);
        VacantPoolsRHI.push_back(PoolRHI);
    }

    RHIDescriptorPool* VacantPoolRHI = VacantPoolsRHI.back();
    RHIDescriptorSet* DescriptorSetRHI = VacantPoolRHI->Allocate();

    if (!VacantPoolRHI->CanAllocate())
    {
        VacantPoolsRHI.pop_back();
        FullPoolsRHI.insert(VacantPoolRHI);
    }

    RDGDescriptorSetRef DescriptorSetRDG = new RDGDescriptorSet("", Layout, DescriptorSetRHI);
    DescriptorSetRDG->DescriptorSetRHI = DescriptorSetRHI;

    return DescriptorSetRDG;
}

void RDGDescriptorSetPool::Release(RDGDescriptorSet* DescriptorSet)
{
    RHIDescriptorSet* DescriptorSetRHI = DescriptorSet->DescriptorSetRHI;
    RHIDescriptorPool* PoolRHI = DescriptorSetRHI->GetPool();
    PoolRHI->Free(DescriptorSetRHI);

	auto Found = FullPoolsRHI.find(PoolRHI);
    if (Found != FullPoolsRHI.end())
    {
        FullPoolsRHI.erase(Found);
        VacantPoolsRHI.push_back(PoolRHI);
    }
    
}

} // namespace nilou