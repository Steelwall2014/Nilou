#include "RenderGraphResources.h"
#include "Common/Crc.h"


namespace std {

size_t hash<nilou::FRDGTextureDesc>::operator()(const nilou::FRDGTextureDesc &_Keyval) const noexcept {
	return FCrc::MemCrc32(&_Keyval, sizeof(_Keyval));
}

size_t hash<nilou::FRDGBufferDesc>::operator()(const nilou::FRDGBufferDesc &_Keyval) const noexcept {
	return FCrc::MemCrc32(&_Keyval, sizeof(_Keyval));
}

size_t hash<nilou::FRDGUniformBufferDesc>::operator()(const nilou::FRDGUniformBufferDesc &_Keyval) const noexcept {
	return FCrc::MemCrc32(&_Keyval, sizeof(_Keyval));
}

}