#include "RenderGraphDescriptorSet.h"
#include "DynamicRHI.h"

namespace nilou {

constexpr int MAX_NUM_DESCRIPTORSETS_PER_POOL = 1024;

RDGDescriptorSet::~RDGDescriptorSet()
{
    if (Pools && GetRHI())
    {
        Pools->Release(GetRHI());
    }
}

void RDGDescriptorSet::SetUniformBuffer(const std::string& Name, RDGBuffer* Buffer)
{
    if (auto Binding = GetBindingByName(Name))
	{
		Ncheck(Binding->DescriptorType == EDescriptorType::UniformBuffer);
		DescriptorBufferInfo BufferInfo;
		BufferInfo.Buffer = Buffer;
		BufferInfo.Offset = 0;
		BufferInfo.Range = Buffer->Desc.GetSize();
		WriteDescriptorSet WriteDescriptor;
		WriteDescriptor.DstBinding = Binding->BindingIndex;
		WriteDescriptor.DstArrayElement = 0;
		WriteDescriptor.DescriptorType = EDescriptorType::UniformBuffer;
		WriteDescriptor.BufferInfo = BufferInfo;
		WriteDescriptor.Access = ERHIAccess::UniformRead;
		WriterInfos[Binding->BindingIndex] = WriteDescriptor;
	}
}

void RDGDescriptorSet::SetSampler(const std::string& Name, RDGTextureView* Texture, RHISamplerState* SamplerState)
{
	if (auto Binding = GetBindingByName(Name))
	{
		Ncheck(Binding->DescriptorType == EDescriptorType::CombinedImageSampler);
		DescriptorImageInfo ImageInfo;
		ImageInfo.SamplerState = SamplerState;
		ImageInfo.Texture = Texture;
		WriteDescriptorSet WriteDescriptor;
		WriteDescriptor.DstBinding = Binding->BindingIndex;
		WriteDescriptor.DstArrayElement = 0;
		WriteDescriptor.DescriptorType = EDescriptorType::CombinedImageSampler;
		WriteDescriptor.ImageInfo = ImageInfo;
		WriteDescriptor.Access = ERHIAccess::ShaderResourceRead;
		WriterInfos[Binding->BindingIndex] = WriteDescriptor;
	}
}

void RDGDescriptorSet::SetStorageBuffer(const std::string& Name, RDGBuffer* Buffer)
{
	if (auto Binding = GetBindingByName(Name))
	{
		Ncheck(Binding->DescriptorType == EDescriptorType::StorageBuffer);
		DescriptorBufferInfo BufferInfo;
		BufferInfo.Buffer = Buffer;
		BufferInfo.Offset = 0;
		BufferInfo.Range = Buffer->Desc.GetSize();
		WriteDescriptorSet WriteDescriptor;
		WriteDescriptor.DstBinding = Binding->BindingIndex;
		WriteDescriptor.DstArrayElement = 0;
		WriteDescriptor.DescriptorType = EDescriptorType::StorageBuffer;
		WriteDescriptor.BufferInfo = BufferInfo;
		WriteDescriptor.Access = ERHIAccess::ShaderResourceReadWrite;
		if ((Binding->Flags & EDescriptorDecorationFlags::NonReadable) != EDescriptorDecorationFlags::None)
		{
			WriteDescriptor.Access &= ~ERHIAccess::ShaderResourceRead;
		}
		if ((Binding->Flags & EDescriptorDecorationFlags::NonWritable) != EDescriptorDecorationFlags::None)
		{
			WriteDescriptor.Access &= ~ERHIAccess::ShaderResourceWrite;
		}
		Ncheck(WriteDescriptor.Access != ERHIAccess::None);
		WriterInfos[Binding->BindingIndex] = WriteDescriptor;
	}
}

void RDGDescriptorSet::SetStorageImage(const std::string& Name, RDGTextureView* Image)
{
	if (auto Binding = GetBindingByName(Name))
	{
		Ncheck(Binding->DescriptorType == EDescriptorType::StorageImage);
		DescriptorImageInfo ImageInfo;
		ImageInfo.Texture = Image;
		WriteDescriptorSet WriteDescriptor;
		WriteDescriptor.DstBinding = Binding->BindingIndex;
		WriteDescriptor.DstArrayElement = 0;
		WriteDescriptor.DescriptorType = EDescriptorType::StorageImage;
		WriteDescriptor.ImageInfo = ImageInfo;
		WriteDescriptor.Access = ERHIAccess::ShaderResourceReadWrite;
		if ((Binding->Flags & EDescriptorDecorationFlags::NonReadable) != EDescriptorDecorationFlags::None)
		{
			WriteDescriptor.Access &= ~ERHIAccess::ShaderResourceRead;
		}
		if ((Binding->Flags & EDescriptorDecorationFlags::NonWritable) != EDescriptorDecorationFlags::None)
		{
			WriteDescriptor.Access &= ~ERHIAccess::ShaderResourceWrite;
		}
		Ncheck(WriteDescriptor.Access != ERHIAccess::None);
		WriterInfos[Binding->BindingIndex] = WriteDescriptor;
	}
}

RHIDescriptorSet* RHIDescriptorSetPools::Allocate()
{
    if (VacantPoolsRHI.size() == 0)
    {
        RHIDescriptorPoolRef PoolRHI = RHICreateDescriptorPool(Layout, MAX_NUM_DESCRIPTORSETS_PER_POOL);
        PoolsRHI.push_back(PoolRHI);
        VacantPoolsRHI.push_back(PoolRHI.GetReference());
    }

    RHIDescriptorPool* VacantPoolRHI = VacantPoolsRHI.back();
    RHIDescriptorSet* DescriptorSetRHI = VacantPoolRHI->Allocate();
	Ncheck(DescriptorSetRHI);

    if (!VacantPoolRHI->CanAllocate())
    {
        VacantPoolsRHI.pop_back();
        FullPoolsRHI.insert(VacantPoolRHI);
    }

	NumAllocatedDescriptorSets++;
    return DescriptorSetRHI;
}

void RHIDescriptorSetPools::Release(RHIDescriptorSet* DescriptorSetRHI)
{
    RHIDescriptorPool* PoolRHI = DescriptorSetRHI->GetPool();
    PoolRHI->Free(DescriptorSetRHI);

	auto Found = FullPoolsRHI.find(PoolRHI);
    if (Found != FullPoolsRHI.end())
    {
        FullPoolsRHI.erase(Found);
        VacantPoolsRHI.push_back(PoolRHI);
    }

    NumAllocatedDescriptorSets--;
}

} // namespace nilou