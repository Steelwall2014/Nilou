#include "RenderGraphResources.h"
#include "Common/Crc.h"


namespace std {

size_t hash<nilou::RDGTextureDesc>::operator()(const nilou::RDGTextureDesc &_Keyval) const noexcept {
	return FCrc::MemCrc32(&_Keyval, sizeof(_Keyval));
}

size_t hash<nilou::RDGBufferDesc>::operator()(const nilou::RDGBufferDesc &_Keyval) const noexcept {
	return FCrc::MemCrc32(&_Keyval, sizeof(_Keyval));
}

// size_t hash<nilou::RDGUniformBufferDesc>::operator()(const nilou::RDGUniformBufferDesc &_Keyval) const noexcept {
// 	return FCrc::MemCrc32(&_Keyval, sizeof(_Keyval));
// }

}

namespace nilou {


void RDGDescriptorSet::SetUniformBuffer(uint32 BindingIndex, RDGBuffer* Buffer)
{
	DescriptorBufferInfo BufferInfo;
	BufferInfo.Buffer = Buffer;
	BufferInfo.Offset = 0;
	BufferInfo.Range = Buffer->Desc.Size;
	WriteDescriptorSet WriteDescriptor;
	WriteDescriptor.DstBinding = BindingIndex;
	WriteDescriptor.DstArrayElement = 0;
	WriteDescriptor.DescriptorType = EDescriptorType::UniformBuffer;
	WriteDescriptor.BufferInfo = BufferInfo;
	WriterInfos[BindingIndex] = WriteDescriptor;
}

void RDGDescriptorSet::SetSampler(uint32 BindingIndex, RHISamplerState* SamplerState, RDGTexture* Texture)
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

void RDGDescriptorSet::SetStorageBuffer(uint32 BindingIndex, RDGBuffer* Buffer)
{
	DescriptorBufferInfo BufferInfo;
	BufferInfo.Buffer = Buffer;
	BufferInfo.Offset = 0;
	BufferInfo.Range = Buffer->Desc.Size;
	WriteDescriptorSet WriteDescriptor;
	WriteDescriptor.DstBinding = BindingIndex;
	WriteDescriptor.DstArrayElement = 0;
	WriteDescriptor.DescriptorType = EDescriptorType::StorageBuffer;
	WriteDescriptor.BufferInfo = BufferInfo;
	WriterInfos[BindingIndex] = WriteDescriptor;
}

void RDGDescriptorSet::SetStorageImage(uint32 BindingIndex, RDGTexture* Image)
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

}