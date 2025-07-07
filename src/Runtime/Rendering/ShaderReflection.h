#pragma once
#include <map>
#include <optional>
#include <unordered_map>
#include <vector>
#include <shaderc/shaderc.h>
#include "Platform.h"
#include "RHIDefinitions.h"
#include "Templates/RefCounting.h"

namespace nilou {

namespace shader_reflection {

// keep the same with spirv-reflect SpvDim_
enum class EImageDim
{
    _1D = 0,
    _2D = 1,
    _3D = 2,
    Cube = 3,
    Rect = 4,
    Buffer = 5,
    SubpassData = 6,
    Max = 0x7fffffff,
};

// keep the same with spirv-reflect SpvImageFormat_
enum class EImageFormat
{
    Unknown = 0,
    Rgba32f = 1,
    Rgba16f = 2,
    R32f = 3,
    Rgba8 = 4,
    Rgba8Snorm = 5,
    Rg32f = 6,
    Rg16f = 7,
    R11fG11fB10f = 8,
    R16f = 9,
    Rgba16 = 10,
    Rgb10A2 = 11,
    Rg16 = 12,
    Rg8 = 13,
    R16 = 14,
    R8 = 15,
    Rgba16Snorm = 16,
    Rg16Snorm = 17,
    Rg8Snorm = 18,
    R16Snorm = 19,
    R8Snorm = 20,
    Rgba32i = 21,
    Rgba16i = 22,
    Rgba8i = 23,
    R32i = 24,
    Rg32i = 25,
    Rg16i = 26,
    Rg8i = 27,
    R16i = 28,
    R8i = 29,
    Rgba32ui = 30,
    Rgba16ui = 31,
    Rgba8ui = 32,
    R32ui = 33,
    Rgb10a2ui = 34,
    Rg32ui = 35,
    Rg16ui = 36,
    Rg8ui = 37,
    R16ui = 38,
    R8ui = 39,
    R64ui = 40,
    R64i = 41,
    Max = 0x7fffffff,
};

// keep the same with spirv-reflect SPV_REFLECT_MAX_ARRAY_DIMS
constexpr uint32 MAX_ARRAY_DIMS = 32;

// keep the same with spirv-reflect
enum class EDescriptorType : uint32
{
    Sampler                     =  0,        // = VK_DESCRIPTOR_TYPE_SAMPLER
    CombinedImageSampler        =  1,        // = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
    SampledImage                =  2,        // = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
    StorageImage                =  3,        // = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
    UniformTexelBuffer          =  4,        // = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
    StorageTexelBuffer          =  5,        // = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
    UniformBuffer               =  6,        // = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
    StorageBuffer               =  7,        // = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
    UniformBufferDynamic        =  8,        // = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
    StorageBufferDynamic        =  9,        // = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
    InputAttachment             = 10,        // = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
    Max                         = 0x7FFFFFFF
};

struct NumericTraits
{
    struct Scalar {
        uint32 Width;
        uint32 Signedness;
    } Scalar;

    struct Vector {
        uint32 ComponentCount;
    } Vector;

    struct Matrix {
        uint32 ColumnCount;
        uint32 RowCount;
        uint32 Stride; // Measured in bytes
    } Matrix;
};

struct ImageTraits
{
    EImageDim Dim;
    uint32 Depth;
    bool Arrayed;
    bool MultiSampled;
    bool Sampled;
    EImageFormat Format;
};

struct ArrayTraits
{
    uint32 DimsCount;
    uint32 Dims[MAX_ARRAY_DIMS];
};

struct BlockVariable
{
    std::string Name;
    uint32 Offset;
    uint32 AbsoluteOffset;
    uint32 Size;
    uint32 PaddedSize;

    // Traits
    NumericTraits         Numeric;
    ArrayTraits           Array;

    std::vector<BlockVariable> Members;
};

struct DescriptorSetLayoutBinding
{
    std::string Name;
    std::string TypeName;
    uint32 SetIndex;
    uint32 BindingIndex;
    EDescriptorType DescriptorType;
    
    // A binding can either be a block or an image, 
    // because Vulkan does't support uniform variables not in a block.
    BlockVariable Block;
    ImageTraits Image;
    ArrayTraits Array;

    bool bReadOnly;
    bool bWriteOnly;
};

struct DescriptorSetLayout
{
public:

    DescriptorSetLayoutBinding& operator[](uint32 Index)
    {
        return Bindings[Index];
    }

    const DescriptorSetLayoutBinding& operator[](uint32 Index) const
    {
        return Bindings.at(Index);
    }

    auto begin() const
    {
        return Bindings.begin();
    }

    auto end() const
    {
        return Bindings.end();
    }

    auto NumBindings() const
    {
        return Bindings.size();
    }

private:
    std::map<uint32, DescriptorSetLayoutBinding> Bindings;
};

struct DescriptorSetLayouts
{
public:

    DescriptorSetLayout& operator[](uint32 SetIndex)
    {
        return SetLayouts[SetIndex];
    }

    const DescriptorSetLayout& operator[](uint32 SetIndex) const
    {
        return SetLayouts.at(SetIndex);
    }

    auto begin() const
    {
        return SetLayouts.begin();
    }

    auto end() const
    {
        return SetLayouts.end();
    }

    bool contains(int32 SetIndex) const
    {
        return SetLayouts.find(SetIndex) != SetLayouts.end();
    }

    auto NumDescriptorSets() const
    {
        return SetLayouts.size();
    }

private:
    std::map<uint32, DescriptorSetLayout> SetLayouts;
};

bool ReflectShader(const std::string& ShaderCode, EShaderStage ShaderStage, DescriptorSetLayouts& OutLayouts, std::string& OutMessage);

} // namespace shader_reflection

bool ReflectShader(const std::string& ShaderCode, EShaderStage ShaderStage, std::unordered_map<uint32, TRefCountPtr<class RHIDescriptorSetLayout>>& OutLayouts, std::optional<class RHIPushConstantRange>& OutPushConstantRange, std::string& OutMessage);
bool ReflectShader(shaderc_compilation_result_t compile_result, std::unordered_map<uint32, TRefCountPtr<class RHIDescriptorSetLayout>>& OutLayouts, std::optional<class RHIPushConstantRange>& OutPushConstantRange, std::string& OutMessage);
} // namespace nilou