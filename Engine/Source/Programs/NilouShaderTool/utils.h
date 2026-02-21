#include <cassert>
#include <slang.h>
#include <filesystem>
#include <fstream>
#include <iostream>

inline void diagnoseIfNeeded(slang::IBlob* diagnosticsBlob)
{
    if (diagnosticsBlob != nullptr)
    {
        std::cout << (const char*)diagnosticsBlob->getBufferPointer() << std::endl;
        __debugbreak();
    }
}

bool IsSlangModule(const std::filesystem::path& SlangFilePath);

inline bool LoadFileToString(std::string& OutString, const std::filesystem::path& FilePath)
{
    if (!std::filesystem::exists(FilePath))
    {
        return false;
    }
    std::ifstream file(FilePath);
    if (!file.is_open())
    {
        return false;
    }
    std::stringstream ss;
    ss << file.rdbuf();
    OutString = ss.str();
    return true;
}

inline std::string MapSlangBindingTypeToEDescriptorType(slang::BindingType bindingType)
{
    switch (bindingType)
    {
#define CASE(FROM, TO)             \
case slang::BindingType::FROM: \
    return "EDescriptorType::"#TO

        CASE(Sampler, Sampler);
        CASE(CombinedTextureSampler, CombinedImageSampler);
        CASE(Texture, SampledImage);
        CASE(MutableTexture, StorageImage);
        CASE(TypedBuffer, UniformTexelBuffer);
        CASE(MutableTypedBuffer, StorageTexelBuffer);
        CASE(ConstantBuffer, UniformBuffer);
        CASE(RawBuffer, StorageBuffer);
        CASE(MutableRawBuffer, StorageBuffer);
        CASE(InputRenderTarget, InputAttachment);
        CASE(InlineUniformData, InlineUniformBlock);
        CASE(RayTracingAccelerationStructure, AccelerationStructure);

#undef CASE

    default:
        return "";
    }
}
