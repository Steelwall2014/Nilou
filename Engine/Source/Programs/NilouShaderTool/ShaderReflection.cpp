#include <functional>
#include <set>
#include <unordered_map>
#include <filesystem>
#include <slang.h>
#include <slang-com-ptr.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "ShaderReflection.h"
#include "utils.h"

namespace fs = std::filesystem;

const std::string GeneratedCppTemplate = 
R"(// Begin {NAMESPACE}{STRUCT_NAME}
{BEGIN_NAMESPACE_DECLARATIONS}
static RHIDescriptorSetLayoutRef CreateDescriptorSetLayout_{STRUCT_NAME}()
{
	std::vector<RHIDescriptorSetLayoutBinding> Bindings;

CREATE_DESCRIPTOR_SET_LAYOUT
	return RHICreateDescriptorSetLayout(Bindings);
}

static std::vector<FShaderParametersMetadata2::FMember> GetMembers_{STRUCT_NAME}()
{
	std::vector<FShaderParametersMetadata2::FMember> Members;

GET_MEMBERS
	return Members;
}

{END_NAMESPACE_DECLARATIONS}
template <>
SHADERBINDINGS_API FShaderParametersMetadata2* GetShaderParametersMetadata<{NAMESPACE}{STRUCT_NAME}>()
{
	static FShaderParametersMetadata2 Metadata(
		"{LOGICAL_NAMESPACE}{STRUCT_NAME}",
		"SOURCE_LOCATION_FILE_PATH",
		SOURCE_LOCATION_LINE,
		{NAMESPACE}CreateDescriptorSetLayout_{STRUCT_NAME}(),
		{NAMESPACE}GetMembers_{STRUCT_NAME}()
	);
	return &Metadata;
}

static FShaderParameterRegistry PREPROCESSOR_JOIN(UniqueRegister, __LINE__)(
	[]() -> std::pair<std::string, FShaderParametersMetadata2*>
	{
		return std::make_pair(
            "{LOGICAL_NAMESPACE}{STRUCT_NAME}", 
            GetShaderParametersMetadata<{NAMESPACE}{STRUCT_NAME}>()
        );
	}
);

// End {NAMESPACE}{STRUCT_NAME}
)";

const std::string GeneratedNonOpaqueCppStructTemplate =
R"(// Size in shader: TYPE_SIZE bytes, alignment in shader: TYPE_ALIGNMENT bytes
template<> struct alignas(TYPE_ALIGNMENT) FULL_STRUCT_NAME
{
NON_OPAQUE_FIELDS
    bool operator==(const FULL_STRUCT_NAME& Other) const = default;
    bool operator!=(const FULL_STRUCT_NAME& Other) const = default;
};
)";

const std::string GeneratedOpaqueCppStructTemplate =
R"(template<> struct STRUCT_NAME<OpaqueLayout>
{
OPAQUE_FIELDS
    bool operator==(const STRUCT_NAME<OpaqueLayout>& Other) const = default;
    bool operator!=(const STRUCT_NAME<OpaqueLayout>& Other) const = default;
};
)";

std::string SimpleMacroReplace(const std::string& Template, const std::string& Macro, const std::string& Value)
{
    std::string Output = Template;
    size_t Pos = 0;
    
    while ((Pos = Output.find(Macro, Pos)) != std::string::npos)
    {
        Output.replace(Pos, Macro.length(), Value);
        Pos += Value.length();
    }
    
    return Output;
}

// Returns the bare C++ type name for a shader field. Alignment is applied by the caller via
// `alignas(...)` on the member declaration; we deliberately do NOT wrap the type in TAlignedType
// here because applying `__declspec(align)` / `__attribute__((aligned))` through a typedef alias
// is silently dropped under the MSVC ABI (incl. clang-cl), which produced wrong member offsets
// (e.g. mie_extinction landing at 188 instead of 192 in AtmosphereParameters<Std140>).
// For TStaticArray, the per-element alignment is carried by its third template argument and
// applied inside the container itself, so its element type also stays bare.
std::string GetFieldTypeName(slang::TypeLayoutReflection* TypeLayout)
{
    slang::TypeReflection* Type = TypeLayout->getType();
    slang::TypeReflection::Kind Kind = Type->getKind();
    if (Kind == slang::TypeReflection::Kind::Array)
    {
        int ElementCount = Type->getElementCount();
        int Alignment = TypeLayout->getAlignment(slang::ParameterCategory::Uniform);
        slang::TypeLayoutReflection* ElementTypeLayout = TypeLayout->getElementTypeLayout();
        std::string TypeName = GetFieldTypeName(ElementTypeLayout);
        return std::format("TStaticArray<{}, {}, {}>", TypeName, ElementCount, Alignment);
    }
    else if (Kind == slang::TypeReflection::Kind::Vector)
    {
        int ColumnCount = Type->getColumnCount();
        slang::TypeReflection* ElementType = Type->getElementType();
        std::string TypeName = ElementType->getName();
        return std::format("{}{}", TypeName, ColumnCount);
    }
    else if (Kind == slang::TypeReflection::Kind::Matrix)
    {
        int RowCount = Type->getRowCount();
        int ColumnCount = Type->getColumnCount();
        slang::TypeReflection* ElementType = Type->getElementType();
        std::string TypeName = ElementType->getName();
        return std::format("{}{}x{}", TypeName, RowCount, ColumnCount);
    }
    else if (Kind == slang::TypeReflection::Kind::Scalar)
    {
        std::string TypeName = Type->getName();
        return TypeName;
    }
    else if (Kind == slang::TypeReflection::Kind::Struct)
    {
        std::string TypeName = Type->getName();
        return std::format("{}<EShaderDataLayout::Std140>", TypeName);
    }
    else if (Kind == slang::TypeReflection::Kind::Resource)
    {
        std::string TypeName = Type->getName();
        if ((Type->getResourceShape() & SLANG_RESOURCE_BASE_SHAPE_MASK) == SLANG_STRUCTURED_BUFFER)
        {
            return "RDGBuffer*";
        }
        else if ((Type->getResourceShape() & SLANG_RESOURCE_BASE_SHAPE_MASK) == SLANG_TEXTURE_1D || 
                 (Type->getResourceShape() & SLANG_RESOURCE_BASE_SHAPE_MASK) == SLANG_TEXTURE_2D ||
                 (Type->getResourceShape() & SLANG_RESOURCE_BASE_SHAPE_MASK) == SLANG_TEXTURE_3D ||
                 (Type->getResourceShape() & SLANG_RESOURCE_BASE_SHAPE_MASK) == SLANG_TEXTURE_CUBE)
        {
            if (Type->getResourceShape() & SLANG_TEXTURE_COMBINED_FLAG)
            {
                return "RDGCombinedTextureSampler";
            }
            else 
            {
                return "RDGTextureView*";
            }
        }
    }
    else if (Kind == slang::TypeReflection::Kind::SamplerState)
    {
        return "RHISamplerState*";
    }
    
    std::cerr << "Not supported type: " << Type->getName() << std::endl;
    assert(false);
    return Type->getName();
}

static bool MapScalarUniformType(
    slang::TypeReflection* ScalarType,
    std::string& OutBaseType,
    std::string& OutPrecision)
{
    if (ScalarType->getKind() != slang::TypeReflection::Kind::Scalar)
        return false;
    switch (ScalarType->getScalarType())
    {
    case slang::TypeReflection::ScalarType::Float32:
        OutBaseType = "EUniformBufferBaseType2::Float32";
        OutPrecision = "EShaderPrecisionModifier2::Float";
        return true;
    case slang::TypeReflection::ScalarType::Float16:
        OutBaseType = "EUniformBufferBaseType2::Float32";
        OutPrecision = "EShaderPrecisionModifier2::Half";
        return true;
    case slang::TypeReflection::ScalarType::Int32:
    case slang::TypeReflection::ScalarType::Int16:
    case slang::TypeReflection::ScalarType::Int8:
        OutBaseType = "EUniformBufferBaseType2::Int32";
        OutPrecision = "EShaderPrecisionModifier2::Invalid";
        return true;
    case slang::TypeReflection::ScalarType::UInt32:
    case slang::TypeReflection::ScalarType::UInt16:
    case slang::TypeReflection::ScalarType::UInt8:
        OutBaseType = "EUniformBufferBaseType2::UInt32";
        OutPrecision = "EShaderPrecisionModifier2::Invalid";
        return true;
    case slang::TypeReflection::ScalarType::Bool:
        OutBaseType = "EUniformBufferBaseType2::Bool";
        OutPrecision = "EShaderPrecisionModifier2::Invalid";
        return true;
    case slang::TypeReflection::ScalarType::Float64:
        OutBaseType = "EUniformBufferBaseType2::Float64";
        OutPrecision = "EShaderPrecisionModifier2::Float";
        return true;
    default:
        std::cerr << "EmitMetadata: unsupported scalar in uniform: " << ScalarType->getName() << std::endl;
        return false;
    }
}

static void AppendUniformVectorOrMatrixPrecision(
    slang::TypeReflection* ElementScalarType,
    std::string& OutPrecision)
{
    if (ElementScalarType->getKind() == slang::TypeReflection::Kind::Scalar &&
        ElementScalarType->getScalarType() == slang::TypeReflection::ScalarType::Float16)
        OutPrecision = "EShaderPrecisionModifier2::Half";
    else
        OutPrecision = "EShaderPrecisionModifier2::Float";
}

static void EmitUniformFieldMetadataRecursive(
    std::string& GetMembers,
    slang::TypeLayoutReflection* StructLayout,
    int StructBaseOffsetInUB,
    const std::string& QualifiedPrefix);

static void EmitUniformTypeMetadataRecursive(
    std::string& GetMembers,
    slang::TypeLayoutReflection* TypeLayout,
    int AbsoluteOffsetInUB,
    const std::string& QualifiedName);

static bool GetUniformMemberShape(
    slang::TypeReflection* Type,
    std::string& OutBaseType,
    std::string& OutPrecision,
    unsigned& OutNumRows,
    unsigned& OutNumColumns)
{
    switch (Type->getKind())
    {
    case slang::TypeReflection::Kind::Scalar:
        OutNumRows = 1u;
        OutNumColumns = 1u;
        return MapScalarUniformType(Type, OutBaseType, OutPrecision);
    case slang::TypeReflection::Kind::Vector:
    {
        slang::TypeReflection* Elem = Type->getElementType();
        if (!MapScalarUniformType(Elem, OutBaseType, OutPrecision))
            return false;
        AppendUniformVectorOrMatrixPrecision(Elem, OutPrecision);
        OutNumRows = 1u;
        OutNumColumns = (unsigned)Type->getColumnCount();
        return true;
    }
    case slang::TypeReflection::Kind::Matrix:
    {
        slang::TypeReflection* Elem = Type->getElementType();
        if (!MapScalarUniformType(Elem, OutBaseType, OutPrecision))
            return false;
        AppendUniformVectorOrMatrixPrecision(Elem, OutPrecision);
        OutNumRows = (unsigned)Type->getRowCount();
        OutNumColumns = (unsigned)Type->getColumnCount();
        return true;
    }
    case slang::TypeReflection::Kind::Struct:
        OutBaseType = "EUniformBufferBaseType2::NestedStruct";
        OutPrecision = "EShaderPrecisionModifier2::Invalid";
        OutNumRows = 1u;
        OutNumColumns = 1u;
        return true;
    case slang::TypeReflection::Kind::Array:
        return GetUniformMemberShape(Type->getElementType(), OutBaseType, OutPrecision, OutNumRows, OutNumColumns);
    default:
        return false;
    }
}

static void EmitOneUniformMemberLine(
    std::string& GetMembers,
    const std::string& QualifiedName,
    int AbsoluteOffsetInUB,
    const std::string& BaseTypeStr,
    const std::string& PrecisionStr,
    unsigned NumRows,
    unsigned NumColumns,
    unsigned NumElements,
    unsigned ArrayStride = 0u)
{
    GetMembers += std::format(
        "\tMembers.push_back( {{0, \"{}\", {}, {}, {}, {}u, {}u, {}u, {}u}} );\n",
        QualifiedName,
        AbsoluteOffsetInUB,
        BaseTypeStr,
        PrecisionStr,
        NumRows,
        NumColumns,
        NumElements,
        ArrayStride);
}

static void EmitUniformTypeMetadataRecursive(
    std::string& GetMembers,
    slang::TypeLayoutReflection* TypeLayout,
    int AbsoluteOffsetInUB,
    const std::string& QualifiedName)
{
    if (!TypeLayout)
        return;

    slang::TypeReflection* Type = TypeLayout->getType();
    if (!Type)
        return;

    switch (Type->getKind())
    {
    case slang::TypeReflection::Kind::Resource:
    case slang::TypeReflection::Kind::SamplerState:
        break;
    case slang::TypeReflection::Kind::Scalar:
    case slang::TypeReflection::Kind::Vector:
    case slang::TypeReflection::Kind::Matrix:
    {
        std::string BaseStr;
        std::string PrecStr;
        unsigned NumRows = 1u;
        unsigned NumColumns = 1u;
        if (GetUniformMemberShape(Type, BaseStr, PrecStr, NumRows, NumColumns))
            EmitOneUniformMemberLine(GetMembers, QualifiedName, AbsoluteOffsetInUB, BaseStr, PrecStr, NumRows, NumColumns, 1u);
        break;
    }
    case slang::TypeReflection::Kind::Struct:
        EmitUniformFieldMetadataRecursive(GetMembers, TypeLayout, AbsoluteOffsetInUB, QualifiedName);
        break;
    case slang::TypeReflection::Kind::Array:
    {
        const int ElementCount = (int)Type->getElementCount();
        slang::TypeLayoutReflection* ElementTypeLayout = TypeLayout->getElementTypeLayout();
        if (ElementCount <= 0 || !ElementTypeLayout)
        {
            std::cerr << "EmitMetadata: unsupported uniform array field: " << QualifiedName << std::endl;
            break;
        }

        unsigned ArrayStride = (unsigned)ElementTypeLayout->getStride();
        if (ArrayStride == 0u)
            ArrayStride = (unsigned)ElementTypeLayout->getSize(slang::ParameterCategory::Uniform);
        if (ArrayStride == 0u && ElementCount > 0)
            ArrayStride = (unsigned)(TypeLayout->getSize(slang::ParameterCategory::Uniform) / ElementCount);
        std::string BaseStr;
        std::string PrecStr;
        unsigned NumRows = 1u;
        unsigned NumColumns = 1u;
        if (GetUniformMemberShape(Type->getElementType(), BaseStr, PrecStr, NumRows, NumColumns))
        {
            EmitOneUniformMemberLine(
                GetMembers,
                QualifiedName,
                AbsoluteOffsetInUB,
                BaseStr,
                PrecStr,
                NumRows,
                NumColumns,
                (unsigned)ElementCount,
                ArrayStride);
        }

        slang::TypeReflection* ElementType = Type->getElementType();
        if (ElementType && (ElementType->getKind() == slang::TypeReflection::Kind::Struct ||
                            ElementType->getKind() == slang::TypeReflection::Kind::Array))
        {
            for (int ElementIndex = 0; ElementIndex < ElementCount; ++ElementIndex)
            {
                EmitUniformTypeMetadataRecursive(
                    GetMembers,
                    ElementTypeLayout,
                    AbsoluteOffsetInUB + (int)(ElementIndex * ArrayStride),
                    std::format("{}[{}]", QualifiedName, ElementIndex));
            }
        }
        break;
    }
    default:
        break;
    }
}

static void EmitUniformFieldMetadataRecursive(
    std::string& GetMembers,
    slang::TypeLayoutReflection* StructLayout,
    int StructBaseOffsetInUB,
    const std::string& QualifiedPrefix)
{
    const int NumFields = StructLayout->getFieldCount();
    for (int FieldIndex = 0; FieldIndex < NumFields; ++FieldIndex)
    {
        slang::VariableLayoutReflection* FieldLayout = StructLayout->getFieldByIndex(FieldIndex);
        slang::TypeLayoutReflection* FieldTypeLayout = FieldLayout->getTypeLayout();
        const std::string FieldName = FieldLayout->getName();
        const std::string Qualified =
            QualifiedPrefix.empty() ? FieldName : (QualifiedPrefix + "." + FieldName);
        const int AbsOffset =
            StructBaseOffsetInUB + (int)FieldLayout->getOffset(slang::ParameterCategory::Uniform);

        EmitUniformTypeMetadataRecursive(GetMembers, FieldTypeLayout, AbsOffset, Qualified);
    }
}

void EmitCppStructForThisType(SlangTypeDeclaration& TypeDecl, slang::TypeLayoutReflection* Container, slang::TypeLayoutReflection* TypeLayout)
{
    std::unordered_map<EShaderDataLayout, std::string>& OutCppStructs = TypeDecl.CppStructs;
    EShaderDataLayout NonOpaqueDataLayout = EShaderDataLayout::Std140;
    if (Container)
    {
        if (Container->getKind() == slang::TypeReflection::Kind::Resource &&
            (Container->getResourceShape() & SLANG_RESOURCE_BASE_SHAPE_MASK) == SLANG_STRUCTURED_BUFFER)
        {
            NonOpaqueDataLayout = EShaderDataLayout::Std430;
        }
    }

    const std::string StructName = TypeLayout->getType()->getName();
    const std::string FullStructName = std::format("{}<{}>", StructName, GetShaderDataLayoutName(NonOpaqueDataLayout));
    const int TypeSize = TypeLayout->getSize(slang::ParameterCategory::Uniform);
    const int TypeAlignment = TypeLayout->getAlignment(slang::ParameterCategory::Uniform);

    // Collect field strings for both layouts in a single pass
    std::string NonOpaqueFields;
    std::string OpaqueFields;
    bool bHasNonOpaqueField = false;
    bool bHasOpaqueField = false;

    const int NumFields = TypeLayout->getFieldCount();
    for (int FieldIndex = 0; FieldIndex < NumFields; FieldIndex++)
    {
        slang::VariableLayoutReflection* FieldLayout = TypeLayout->getFieldByIndex(FieldIndex);
        slang::TypeReflection* FieldType = FieldLayout->getType();
        slang::TypeLayoutReflection* FieldTypeLayout = FieldLayout->getTypeLayout();
        const std::string FieldName = FieldLayout->getName();

        switch (FieldType->getKind())
        {
        case slang::TypeReflection::Kind::Scalar:
        case slang::TypeReflection::Kind::Vector:
        case slang::TypeReflection::Kind::Matrix:
        case slang::TypeReflection::Kind::Array:
        case slang::TypeReflection::Kind::Struct:
        {
            const std::string FieldTypeName = GetFieldTypeName(FieldTypeLayout);
            const int FieldOffset = FieldLayout->getOffset(slang::ParameterCategory::Uniform);
            const int FieldSize = FieldTypeLayout->getSize(slang::ParameterCategory::Uniform);
            const int FieldAlignment = FieldTypeLayout->getAlignment(slang::ParameterCategory::Uniform);
            // Apply alignment directly on the member declaration. `alignas` on a non-static data
            // member is fully honored by MSVC/clang-cl/clang/GCC, unlike the previous TAlignedType
            // typedef trick which was silently dropped on type aliases.
            NonOpaqueFields += std::format("    alignas({}) {} {};    // offset in shader: {} bytes, size in shader: {} bytes\n", FieldAlignment, FieldTypeName, FieldName, FieldOffset, FieldSize);
            bHasNonOpaqueField = true;
            break;
        }
        default:
        {
            const std::string FieldTypeName = GetFieldTypeName(FieldTypeLayout);
            OpaqueFields += std::format("    {} {}{{}};\n", FieldTypeName, FieldName);
            bHasOpaqueField = true;
            break;
        }
        }
    }

    // Build NonOpaque struct from template
    if (bHasNonOpaqueField)
    {
        std::string NonOpaqueDecl = GeneratedNonOpaqueCppStructTemplate;
        NonOpaqueDecl = SimpleMacroReplace(NonOpaqueDecl, "TYPE_SIZE",        std::to_string(TypeSize));
        NonOpaqueDecl = SimpleMacroReplace(NonOpaqueDecl, "TYPE_ALIGNMENT",   std::to_string(TypeAlignment));
        NonOpaqueDecl = SimpleMacroReplace(NonOpaqueDecl, "FULL_STRUCT_NAME", FullStructName);
        NonOpaqueDecl = SimpleMacroReplace(NonOpaqueDecl, "NON_OPAQUE_FIELDS", NonOpaqueFields);

        if (OutCppStructs.contains(NonOpaqueDataLayout) && OutCppStructs[NonOpaqueDataLayout] != NonOpaqueDecl)
        {
            std::cerr << "Ambiguous struct: " << StructName << std::endl;
            std::cerr << "Current: " << NonOpaqueDecl << std::endl;
            std::cerr << "Existing: " << OutCppStructs[NonOpaqueDataLayout] << std::endl;
            __debugbreak();
        }
        OutCppStructs[NonOpaqueDataLayout] = NonOpaqueDecl;
    }

    // Build Opaque struct from template
    if (bHasNonOpaqueField || bHasOpaqueField)
    {
        const std::string AutoUBField = bHasNonOpaqueField
            ? "    RDGBuffer* AutomaticallyIntroducedUniformBuffer{};\n"
            : "";
        OpaqueFields = AutoUBField + OpaqueFields;

        std::string OpaqueDecl = GeneratedOpaqueCppStructTemplate;
        OpaqueDecl = SimpleMacroReplace(OpaqueDecl, "STRUCT_NAME",             StructName);
        OpaqueDecl = SimpleMacroReplace(OpaqueDecl, "OPAQUE_FIELDS",           OpaqueFields);

        if (OutCppStructs.contains(EShaderDataLayout::Opaque) && OutCppStructs[EShaderDataLayout::Opaque] != OpaqueDecl)
        {
            std::cout << "Ambiguous struct: " << StructName << std::endl;
            std::cout << "Current: " << OpaqueDecl << std::endl;
            std::cout << "Existing: " << OutCppStructs[EShaderDataLayout::Opaque] << std::endl;
            assert(false);
        }
        OutCppStructs[EShaderDataLayout::Opaque] = OpaqueDecl;
    }
}

void EmitMetadataForThisType(SlangTypeDeclaration& TypeDecl, slang::TypeLayoutReflection* Container, slang::TypeLayoutReflection* TypeLayout)
{
    std::string StructName = TypeLayout->getType()->getName();
    std::string Namespace = "shader::";
    std::string LogicalNamespace;  // Slang-side namespaces only, no "shader::" prefix
    std::string BeginNamespaceDeclarations = "namespace shader {\n";
    std::string EndNamespaceDeclarations;
    for (auto NamespaceDecl : TypeDecl.NamespaceDecls)
    {
        Namespace += std::string(NamespaceDecl->getName()) + "::";
        LogicalNamespace += std::string(NamespaceDecl->getName()) + "::";
        BeginNamespaceDeclarations += std::format("namespace {} {{\n", NamespaceDecl->getName());
        EndNamespaceDeclarations += std::format("}} // End of namespace {}\n", NamespaceDecl->getName());
    }
    EndNamespaceDeclarations += "} // namespace shader\n";

    int BindingIndex = 0;
    std::string CreateDescriptorSetLayout;
    std::string GetMembers;

    auto GetResourcePrecisionString = [](slang::TypeReflection* ResourceType) -> std::string
    {
        slang::TypeReflection* ResultType = ResourceType->getResourceResultType();
        if (!ResultType)
            return "EShaderPrecisionModifier2::Float";
        slang::TypeReflection* ScalarType = ResultType;
        while (ScalarType->getKind() == slang::TypeReflection::Kind::Vector ||
               ScalarType->getKind() == slang::TypeReflection::Kind::Matrix)
        {
            ScalarType = ScalarType->getElementType();
        }
        if (ScalarType->getKind() == slang::TypeReflection::Kind::Scalar)
        {
            if (ScalarType->getScalarType() == slang::TypeReflection::ScalarType::Float16)
                return "EShaderPrecisionModifier2::Half";
        }
        return "EShaderPrecisionModifier2::Float";
    };

    auto AppendBinding = [&](slang::BindingType BindingType, int DescriptorCount = 1)
    {
        std::string DescriptorTypeString = MapSlangBindingTypeToEDescriptorType(BindingType);
        CreateDescriptorSetLayout += "\t{\n";
        CreateDescriptorSetLayout += "\t\tRHIDescriptorSetLayoutBinding Binding;\n";
        CreateDescriptorSetLayout += std::format("\t\tBinding.BindingIndex = {};\n", BindingIndex);
        CreateDescriptorSetLayout += std::format("\t\tBinding.DescriptorType = {};\n", DescriptorTypeString);
        CreateDescriptorSetLayout += std::format("\t\tBinding.DescriptorCount = {};\n", DescriptorCount);
        switch (BindingType) 
        {
        case slang::BindingType::TypedBuffer:
        case slang::BindingType::Texture:
        case slang::BindingType::CombinedTextureSampler:
            CreateDescriptorSetLayout += "\t\tBinding.Access = ERHIAccess::ShaderSampledRead;\n";
            break;
        case slang::BindingType::RawBuffer:
            CreateDescriptorSetLayout += "\t\tBinding.Access = ERHIAccess::ShaderStorageRead;\n";
            break;
        case slang::BindingType::MutableTypedBuffer:
        case slang::BindingType::MutableTexture:
        case slang::BindingType::MutableRawBuffer:
            CreateDescriptorSetLayout += "\t\tBinding.Access = ERHIAccess::ShaderStorageReadWrite;\n";
            break;
        case slang::BindingType::ConstantBuffer:
            CreateDescriptorSetLayout += "\t\tBinding.Access = ERHIAccess::UniformRead;\n";
            break;
        case slang::BindingType::Sampler:
            CreateDescriptorSetLayout += "\t\tBinding.Access = ERHIAccess::ShaderSampledRead;\n";
            break;
        default:
            std::cerr << "Not supported binding type: " << (int)BindingType << std::endl;
            __debugbreak();
            break;
        }
        CreateDescriptorSetLayout += "\t\tBindings.push_back(Binding);\n";
        CreateDescriptorSetLayout += "\t}\n";
        BindingIndex++;
    };
    if (TypeLayout->getSize(slang::ParameterCategory::Uniform) > 0)
    {
        const int AutoUBBindingIndex = BindingIndex;
        AppendBinding(slang::BindingType::ConstantBuffer);
        GetMembers += std::format(
            "\tMembers.push_back( {{{}, \"AutomaticallyIntroducedUniformBuffer\", "
            "offsetof({}<EShaderDataLayout::Opaque>, AutomaticallyIntroducedUniformBuffer), "
            "EUniformBufferBaseType2::Buffer, EShaderPrecisionModifier2::Invalid, 1u, 1u, 1u, 0u}} );\n",
            AutoUBBindingIndex, StructName);
    }
    int relativeSetIndex = 0;
    int rangeCount = TypeLayout->getDescriptorSetDescriptorRangeCount(relativeSetIndex);
    for (int rangeIndex = 0; rangeIndex < rangeCount; ++rangeIndex)
    {
        slang::BindingType bindingType =
            TypeLayout->getDescriptorSetDescriptorRangeType(relativeSetIndex, rangeIndex);
        auto descriptorCount = TypeLayout->getDescriptorSetDescriptorRangeDescriptorCount(
            relativeSetIndex,
            rangeIndex);

        // Some Ranges Need to Be Skipped
        // ------------------------------
        //
        switch (bindingType)
        {
        default:
            break;

        case slang::BindingType::PushConstant:
            return;
        }

        AppendBinding(bindingType, descriptorCount);
    }

    int NumFields = TypeLayout->getFieldCount();
    for (int FieldIndex = 0; FieldIndex < NumFields; FieldIndex++)
    {
        slang::VariableLayoutReflection* FieldLayout = TypeLayout->getFieldByIndex(FieldIndex);
        slang::TypeReflection* FieldType = FieldLayout->getType();
        slang::TypeLayoutReflection* FieldTypeLayout = FieldLayout->getTypeLayout();
        slang::TypeReflection::Kind FieldKind = FieldTypeLayout->getKind();
        std::string FieldName = FieldLayout->getName();
        SlangInt bindingRangeIdx = TypeLayout->getFieldBindingRangeOffset(FieldIndex);
        SlangInt setIndex        = TypeLayout->getBindingRangeDescriptorSetIndex(bindingRangeIdx);
        SlangInt descRangeIdx    = TypeLayout->getBindingRangeFirstDescriptorRangeIndex(bindingRangeIdx);
        int FieldBindingIndex    = (int)TypeLayout->getDescriptorSetDescriptorRangeIndexOffset(setIndex, descRangeIdx);
        std::string BaseTypeStr;
        std::string PrecisionStr;
        switch (FieldKind)
        {
        case slang::TypeReflection::Kind::Resource:
        {
            const SlangResourceShape BaseShape = static_cast<SlangResourceShape>(FieldType->getResourceShape() & SLANG_RESOURCE_BASE_SHAPE_MASK);
            const bool bIsCombined = (FieldType->getResourceShape() & SLANG_TEXTURE_COMBINED_FLAG) != 0;
            if (BaseShape == SLANG_STRUCTURED_BUFFER)
            {
                BaseTypeStr  = "EUniformBufferBaseType2::Buffer";
                PrecisionStr = "EShaderPrecisionModifier2::Invalid";
            }
            else if (BaseShape == SLANG_TEXTURE_1D ||
                     BaseShape == SLANG_TEXTURE_2D ||
                     BaseShape == SLANG_TEXTURE_3D ||
                     BaseShape == SLANG_TEXTURE_CUBE)
            {
                if (bIsCombined)
                {
                    BaseTypeStr  = "EUniformBufferBaseType2::TextureSampler";
                    PrecisionStr = GetResourcePrecisionString(FieldType);
                }
                else
                {
                    BaseTypeStr  = "EUniformBufferBaseType2::Texture";
                    PrecisionStr = GetResourcePrecisionString(FieldType);
                }
            }
            break;
        }
        case slang::TypeReflection::Kind::SamplerState:
        {
            BaseTypeStr  = "EUniformBufferBaseType2::Sampler";
            PrecisionStr = "EShaderPrecisionModifier2::Invalid";
            break;
        }
        default:
            break;
        }

        if (!BaseTypeStr.empty())
        {
            GetMembers += std::format(
                "\tMembers.push_back( {{{}, \"{}\", offsetof({}<EShaderDataLayout::Opaque>, {}), {}, {}, 1u, 1u, 1u, 0u}} );\n",
                FieldBindingIndex, FieldName, StructName, FieldName, BaseTypeStr, PrecisionStr);
        }
    }

    // Std140/Std430 uniform block: per-field byte offsets (BindingIndex 0; distinguish by BaseType at runtime).
    if (TypeLayout->getSize(slang::ParameterCategory::Uniform) > 0)
    {
        EmitUniformFieldMetadataRecursive(GetMembers, TypeLayout, 0, "");
    }

    std::string SourceLocationFilePath = fs::path(TypeDecl.SourceLocation.filePath).generic_string();
    std::string SourceLocationLine = std::to_string(TypeDecl.SourceLocation.line);

    std::string OutMetadata = GeneratedCppTemplate;
    OutMetadata = SimpleMacroReplace(OutMetadata, "{NAMESPACE}", Namespace);
    OutMetadata = SimpleMacroReplace(OutMetadata, "{LOGICAL_NAMESPACE}", LogicalNamespace);
    OutMetadata = SimpleMacroReplace(OutMetadata, "{BEGIN_NAMESPACE_DECLARATIONS}", BeginNamespaceDeclarations);
    OutMetadata = SimpleMacroReplace(OutMetadata, "{END_NAMESPACE_DECLARATIONS}", EndNamespaceDeclarations);
    OutMetadata = SimpleMacroReplace(OutMetadata, "CREATE_DESCRIPTOR_SET_LAYOUT", CreateDescriptorSetLayout);
    OutMetadata = SimpleMacroReplace(OutMetadata, "GET_MEMBERS", GetMembers);
    OutMetadata = SimpleMacroReplace(OutMetadata, "{STRUCT_NAME}", StructName);
    OutMetadata = SimpleMacroReplace(OutMetadata, "SOURCE_LOCATION_FILE_PATH", SourceLocationFilePath);
    OutMetadata = SimpleMacroReplace(OutMetadata, "SOURCE_LOCATION_LINE", SourceLocationLine);

    if (TypeDecl.CppMetadata != "" && TypeDecl.CppMetadata != OutMetadata)
    {
        std::cerr << "Ambiguous Metadata" << StructName << std::endl;
        std::cerr << "Current: " << TypeDecl.CppMetadata << std::endl;
        std::cerr << "Existing: " << OutMetadata << std::endl;
        __debugbreak();
    }
    TypeDecl.CppMetadata = OutMetadata;
}

bool SlangShaderReflectionSession::LoadModule(const std::filesystem::path& SlangFilePath)
{
    std::string SourceString;
    if (!LoadFileToString(SourceString, SlangFilePath))
    {
        std::cout << "Failed to load file: " << SlangFilePath.generic_string() << std::endl;
        return false;
    }

    std::string ModuleName = SlangFilePath.stem().string();
    if (Modules.contains(ModuleName))
    {
        std::cout << "Module already loaded: " << ModuleName << std::endl;
        return false;
    }

    slang::IModule* Module;
    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        Module = SlangSession->loadModuleFromSourceString(
            ModuleName.c_str(), 
            SlangFilePath.generic_string().c_str(), 
            SourceString.c_str(), 
            diagnosticsBlob.writeRef());
        diagnoseIfNeeded(diagnosticsBlob);
    }

    // Note: slang 支持隐式加载模块，所以这里需要检查所有模块。
    for (SlangInt i = 0; i < SlangSession->getLoadedModuleCount(); ++i)
    {
        slang::IModule* LocalModule = SlangSession->getLoadedModule(i);
        // 使用路径 import 一个模块、而且这个模块被隐式加载时，
        // 对它调用 getName 获取的是路径而不是 slang 文件中定义的模块名。
        // 所以**Nilou 项目要求 slang 文件中定义的模块名与文件名相同**。
        std::string LocalModuleName = fs::path(LocalModule->getName()).stem().string();
        if (LocalModule)
        {
            if (!Modules.contains(LocalModuleName))
            {
                Modules[LocalModuleName] = LocalModule;
                std::vector<slang::DeclReflection*> NamespaceDecls;
                CollectTypeDeclarations(LocalModule, NamespaceDecls, LocalModule->getModuleReflection());
            }
            else 
            {
                slang::IModule* ExistingModule = Modules[LocalModuleName];
                if (ExistingModule != LocalModule)
                {
                    std::cerr << "Duplicated shader module found: " << LocalModuleName;
                    __debugbreak();
                }
            }
        }
    }

    return true;
}

void SlangShaderReflectionSession::EmitCppStructs()
{
    for (auto& [ModuleName, Module] : Modules)
    {
        slang::ProgramLayout* ProgramLayout;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            ProgramLayout = Module->getLayout(0, diagnosticsBlob.writeRef());
            diagnoseIfNeeded(diagnosticsBlob);
        }
        
        // Enumerate all global parameters
        uint32_t ParameterCount = ProgramLayout->getParameterCount();
        for (uint32_t i = 0; i < ParameterCount; i++)
        {
            slang::VariableLayoutReflection* VarLayout = ProgramLayout->getParameterByIndex(i);
            if (VarLayout == nullptr)
                continue;
            
            EnumerateStructTypeLayoutsRecursive(nullptr, VarLayout->getTypeLayout());
        }
        
        // Enumerate global parameters variable layout (if exists)
        slang::VariableLayoutReflection* GlobalParamsVarLayout = ProgramLayout->getGlobalParamsVarLayout();
        if (GlobalParamsVarLayout != nullptr)
        {
            slang::TypeLayoutReflection* GlobalParamsTypeLayout = GlobalParamsVarLayout->getTypeLayout();
            if (GlobalParamsTypeLayout != nullptr)
            {
                int fieldCount = GlobalParamsTypeLayout->getFieldCount();

                for (int f = 0; f < fieldCount; f++)
                {
                    slang::VariableLayoutReflection* FieldLayout = GlobalParamsTypeLayout->getFieldByIndex(f);
                    EnumerateStructTypeLayoutsRecursive(nullptr, FieldLayout->getTypeLayout());
                }
            }
        }
        
        // Enumerate entry point parameters
        SlangUInt EntryPointCount = ProgramLayout->getEntryPointCount();
        for (SlangUInt i = 0; i < EntryPointCount; i++)
        {
            slang::EntryPointReflection* EntryPoint = ProgramLayout->getEntryPointByIndex(i);
            if (EntryPoint == nullptr)
                continue;
            
            // Enumerate parameters of entry point
            uint32_t EntryPointParameterCount = EntryPoint->getParameterCount();
            for (uint32_t j = 0; j < EntryPointParameterCount; j++)
            {
                slang::VariableLayoutReflection* ParamLayout = EntryPoint->getParameterByIndex(j);
                if (ParamLayout == nullptr)
                    continue;
                
                EnumerateStructTypeLayoutsRecursive(nullptr, ParamLayout->getTypeLayout());
            }
        }
    }
}

std::string SlangShaderReflectionSession::GetCppStructDeclaration(slang::TypeReflection* Type)
{
    auto TypeDecl = GetTypeDeclaration(Type);
    if (TypeDecl == nullptr)
    {
        return "";
    }
    const std::string TypeName = TypeDecl->TypeName;
    std::string Result;
    for  (auto NamespaceDecl : TypeDecl->NamespaceDecls)
    {
        Result += std::format("namespace {} {{\n", NamespaceDecl->getName());
    }
    Result += "\n";
    Result += std::format("// Begin {}\n", TypeName);
    Result += std::format("template <EShaderDataLayout DataLayout> struct {} {{}};\n", TypeName);
    for (auto& [DataLayout, CppStruct] : TypeDecl->CppStructs)
    {
        Result += CppStruct;
    }
    Result += std::format("// End {}\n", TypeName);
    Result += "\n";
    for (auto NamespaceDecl : TypeDecl->NamespaceDecls)
    {
        Result += std::format("}} // End of namespace {}\n", NamespaceDecl->getName());
    }
    return Result;
}

std::string SlangShaderReflectionSession::GetCppStructDefinition(slang::TypeReflection* Type)
{
    auto TypeDecl = GetTypeDeclaration(Type);
    if (TypeDecl == nullptr)
    {
        return "";
    }
    return TypeDecl->CppMetadata + "\n\n";
}

void SlangShaderReflectionSession::CollectTypeDeclarations(slang::IModule* Module, std::vector<slang::DeclReflection*>& NamespaceDecls, slang::DeclReflection* Decl)
{
    if (Decl == nullptr)
    {
        return;
    }

    // Check if this is a struct declaration
    auto Kind = Decl->getKind();
    if (Kind == slang::DeclReflection::Kind::Struct)
    {
        const char* TypeName = Decl->getName(); 
        slang::TypeReflection* Type = Decl->getType();
        slang::SourceLocation SourceLocation;
        SlangSession->getDeclSourceLocation(Decl, &SourceLocation);
        SlangTypeDeclaration NewTypeDecl;
        NewTypeDecl.TypeName = std::string(TypeName);
        NewTypeDecl.Type = Type;
        NewTypeDecl.SourceLocation = SourceLocation;
        NewTypeDecl.Module = Module;
        NewTypeDecl.Decl = Decl;
        NewTypeDecl.NamespaceDecls = NamespaceDecls;
        TypeDeclarations.push_back(NewTypeDecl);
    }

    if (Kind == slang::DeclReflection::Kind::Namespace)
    {
        NamespaceDecls.push_back(Decl);
    }

    // Recursively process children
    unsigned int ChildCount = Decl->getChildrenCount();
    for (unsigned int i = 0; i < ChildCount; i++)
    {
        slang::DeclReflection* Child = Decl->getChild(i);
        CollectTypeDeclarations(Module, NamespaceDecls, Child);
    }

    if (Kind == slang::DeclReflection::Kind::Namespace)
    {
        NamespaceDecls.pop_back();
    }
}

void SlangShaderReflectionSession::EnumerateStructTypeLayoutsRecursive(slang::TypeLayoutReflection* Container, slang::TypeLayoutReflection* TypeLayout)
{
    if (TypeLayout == nullptr) return;
    const char* DebugTypeName = TypeLayout->getName();
    switch (TypeLayout->getKind())
    {
    case slang::TypeReflection::Kind::Struct:
    {
        int fieldCount = TypeLayout->getFieldCount();
        for (int f = 0; f < fieldCount; f++)
        {
            slang::VariableLayoutReflection* FieldLayout = TypeLayout->getFieldByIndex(f);
            EnumerateStructTypeLayoutsRecursive(Container, FieldLayout->getTypeLayout());
        }

        const std::string TypeName = TypeLayout->getName();
        slang::TypeReflection* Type = TypeLayout->getType();
        if (auto TypeDecl = GetTypeDeclaration(Type))
        {
            TypeDecl->bUsedInParameterBlock = true;
            EmitCppStructForThisType(*TypeDecl, Container, TypeLayout);
            EmitMetadataForThisType(*TypeDecl, Container, TypeLayout);
        }

        break;
    }

    case slang::TypeReflection::Kind::Array:
    {
        EnumerateStructTypeLayoutsRecursive(TypeLayout, TypeLayout->getElementTypeLayout());
        break;
    }
    case slang::TypeReflection::Kind::TextureBuffer:
    case slang::TypeReflection::Kind::ShaderStorageBuffer:
    case slang::TypeReflection::Kind::ParameterBlock:
    case slang::TypeReflection::Kind::ConstantBuffer:
    {
        EnumerateStructTypeLayoutsRecursive(TypeLayout, TypeLayout->getElementTypeLayout());
        break;
    }
    case slang::TypeReflection::Kind::Resource:
    {
        if ((TypeLayout->getResourceShape() & SLANG_RESOURCE_BASE_SHAPE_MASK) == SLANG_STRUCTURED_BUFFER)
        {
            EnumerateStructTypeLayoutsRecursive(TypeLayout, TypeLayout->getElementTypeLayout());
        }
        break;
    }

    default:
        break;
    }
}
