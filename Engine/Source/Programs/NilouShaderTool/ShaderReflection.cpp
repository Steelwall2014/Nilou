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
R"(static RHIDescriptorSetLayoutRef Z_CreateDescriptorSetLayout_STRUCT_NAME()
{
	std::vector<RHIDescriptorSetLayoutBinding> Bindings;

CREATE_DESCRIPTOR_SET_LAYOUT
	return RHICreateDescriptorSetLayout(Bindings);
}

static std::vector<FShaderParametersMetadata2::FOpaqueResource> Z_GetOpaqueResources_STRUCT_NAME()
{
	std::vector<FShaderParametersMetadata2::FOpaqueResource> OpaqueResources;

GET_OPAQUE_RESOURCES
	return OpaqueResources;
}

template <>
FShaderParametersMetadata2* GetShaderParametersMetadata<STRUCT_NAME>()
{
	static FShaderParametersMetadata2 Metadata(
		"STRUCT_NAME",
		"SOURCE_LOCATION_FILE_PATH",
		SOURCE_LOCATION_LINE,
		Z_CreateDescriptorSetLayout_STRUCT_NAME(),
		Z_GetOpaqueResources_STRUCT_NAME()
	);
	return &Metadata;
}

static FShaderParameterRegistry UniqueRegister_STRUCT_NAME(
	[]() -> std::pair<std::string, FShaderParametersMetadata2*>
	{
		return std::make_pair("STRUCT_NAME", GetShaderParametersMetadata<STRUCT_NAME>());
	}
);
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
        int Alignment = TypeLayout->getAlignment(slang::ParameterCategory::Uniform);
        slang::TypeReflection* ElementType = Type->getElementType();
        std::string TypeName = ElementType->getName();
        return std::format("TAlignedType<{}{}, {}>", TypeName, ColumnCount, Alignment);
    }
    else if (Kind == slang::TypeReflection::Kind::Matrix)
    {
        int RowCount = Type->getRowCount();
        int ColumnCount = Type->getColumnCount();
        int Alignment = TypeLayout->getAlignment(slang::ParameterCategory::Uniform);
        slang::TypeReflection* ElementType = Type->getElementType();
        std::string TypeName = ElementType->getName();
        return std::format("TAlignedType<{}{}x{}, {}>", TypeName, RowCount, ColumnCount, Alignment);
    }
    else if (Kind == slang::TypeReflection::Kind::Scalar)
    {
        int Alignment = TypeLayout->getAlignment(slang::ParameterCategory::Uniform);
        std::string TypeName = Type->getName();
        return std::format("TAlignedType<{}, {}>", TypeName, Alignment);
    }
    else if (Kind == slang::TypeReflection::Kind::Struct)
    {
        int Alignment = TypeLayout->getAlignment(slang::ParameterCategory::Uniform);
        std::string TypeName = Type->getName();
        return std::format("TAlignedType<{}<EShaderDataLayout::Std140>, {}>", TypeName, Alignment);
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
            NonOpaqueFields += std::format("    {} {};    // offset in shader: {} bytes, size in shader: {} bytes\n", FieldTypeName, FieldName, FieldOffset, FieldSize);
            bHasNonOpaqueField = true;
            break;
        }
        default:
        {
            const std::string FieldTypeName = GetFieldTypeName(FieldTypeLayout);
            OpaqueFields += std::format("    {} {};\n", FieldTypeName, FieldName);
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
            std::cout << "Ambiguous struct: " << StructName << std::endl;
            std::cout << "Current: " << NonOpaqueDecl << std::endl;
            std::cout << "Existing: " << OutCppStructs[NonOpaqueDataLayout] << std::endl;
            __debugbreak();
        }
        OutCppStructs[NonOpaqueDataLayout] = NonOpaqueDecl;
    }

    // Build Opaque struct from template
    if (bHasNonOpaqueField || bHasOpaqueField)
    {
        const std::string AutoUBField = bHasNonOpaqueField
            ? "    RDGBuffer* AutomaticallyIntroducedUniformBuffer;\n"
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

    int BindingIndex = 0;
    std::string CreateDescriptorSetLayout;
    std::string GetOpaqueResources;
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
        case slang::BindingType::RawBuffer:
            CreateDescriptorSetLayout += "\t\tBinding.Access = ERHIAccess::ShaderResourceRead;\n";
            break;
        case slang::BindingType::MutableTypedBuffer:
        case slang::BindingType::MutableTexture:
        case slang::BindingType::MutableRawBuffer:
            CreateDescriptorSetLayout += "\t\tBinding.Access = ERHIAccess::ShaderResourceReadWrite;\n";
            break;
        case slang::BindingType::ConstantBuffer:
        case slang::BindingType::CombinedTextureSampler:
            CreateDescriptorSetLayout += "\t\tBinding.Access = ERHIAccess::UniformRead;\n";
            break;
        case slang::BindingType::Sampler:
            CreateDescriptorSetLayout += "\t\tBinding.Access = ERHIAccess::ShaderResourceRead;\n";
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
        AppendBinding(slang::BindingType::ConstantBuffer);
        GetOpaqueResources += std::format("\tOpaqueResources.push_back( {{{}, offsetof({}<EShaderDataLayout::Opaque>, {}), {}}} );\n", 0, StructName, "AutomaticallyIntroducedUniformBuffer", "FShaderParametersMetadata2::EOpaqueResourceType::Buffer");
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
        int FieldBindingIndex = TypeLayout->getFieldBindingRangeOffset(FieldIndex);
        std::string OpaqueResourceType;
        switch (FieldKind)
        {
        case slang::TypeReflection::Kind::Resource:
        {
            if ((FieldType->getResourceShape() & SLANG_RESOURCE_BASE_SHAPE_MASK) == SLANG_STRUCTURED_BUFFER)
            {
                OpaqueResourceType = "FShaderParametersMetadata2::EOpaqueResourceType::Buffer";
            }
            else if ((FieldType->getResourceShape() & SLANG_RESOURCE_BASE_SHAPE_MASK) == SLANG_TEXTURE_1D || 
                     (FieldType->getResourceShape() & SLANG_RESOURCE_BASE_SHAPE_MASK) == SLANG_TEXTURE_2D ||
                     (FieldType->getResourceShape() & SLANG_RESOURCE_BASE_SHAPE_MASK) == SLANG_TEXTURE_3D ||
                     (FieldType->getResourceShape() & SLANG_RESOURCE_BASE_SHAPE_MASK) == SLANG_TEXTURE_CUBE)
            {
                if ((FieldType->getResourceShape() & SLANG_RESOURCE_BASE_SHAPE_MASK) == SLANG_TEXTURE_COMBINED_FLAG)
                {
                    OpaqueResourceType = "FShaderParametersMetadata2::EOpaqueResourceType::CombinedTextureSampler";
                }
                else 
                {
                    OpaqueResourceType = "FShaderParametersMetadata2::EOpaqueResourceType::TextureView";
                }
            }
            break;
        }
        case slang::TypeReflection::Kind::SamplerState:
        {
            OpaqueResourceType = "FShaderParametersMetadata2::EOpaqueResourceType::SamplerState";
        }
        default:
            break;
        }
        
        if (OpaqueResourceType != "")
        {
            GetOpaqueResources += std::format("\tOpaqueResources.push_back( {{{}, offsetof({}<EShaderDataLayout::Opaque>, {}), {}}} );\n", FieldBindingIndex, StructName, FieldName, OpaqueResourceType);
        }
    }

    std::string SourceLocationFilePath = fs::path(TypeDecl.SourceLocation.filePath).generic_string();
    std::string SourceLocationLine = std::to_string(TypeDecl.SourceLocation.line);

    std::string OutMetadata = GeneratedCppTemplate;
    OutMetadata = SimpleMacroReplace(OutMetadata, "CREATE_DESCRIPTOR_SET_LAYOUT", CreateDescriptorSetLayout);
    OutMetadata = SimpleMacroReplace(OutMetadata, "GET_OPAQUE_RESOURCES", GetOpaqueResources);
    OutMetadata = SimpleMacroReplace(OutMetadata, "STRUCT_NAME", StructName);
    OutMetadata = SimpleMacroReplace(OutMetadata, "SOURCE_LOCATION_FILE_PATH", SourceLocationFilePath);
    OutMetadata = SimpleMacroReplace(OutMetadata, "SOURCE_LOCATION_LINE", SourceLocationLine);

    if (TypeDecl.CppMetadata != "" && TypeDecl.CppMetadata != OutMetadata)
    {
        std::cout << "Ambiguous Metadata" << StructName << std::endl;
        std::cout << "Current: " << TypeDecl.CppMetadata << std::endl;
        std::cout << "Existing: " << OutMetadata << std::endl;
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
    Modules[ModuleName] = Module;

    CollectTypeDeclarations(Module, Module->getModuleReflection());
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

void SlangShaderReflectionSession::CollectTypeDeclarations(slang::IModule* Module, slang::DeclReflection* Decl)
{
    if (Decl == nullptr)
    {
        return;
    }

    // Check if this is a struct declaration
    if (Decl->getKind() == slang::DeclReflection::Kind::Struct)
    {
        const char* TypeName = Decl->getName(); 
        slang::ISession::SourceLocation loc = SlangSession->getDeclSourceLocation(Decl);
        if (TypeName != nullptr && TypeName[0] != '\0')
        {
            // If the type declaration already exists, check if it is from the same module and declaration
            if (HasTypeDeclaration(TypeName))
            {
                SlangTypeDeclaration& ExistingTypeDecl = GetTypeDeclaration(TypeName);
                if (ExistingTypeDecl.Module != Module || ExistingTypeDecl.Decl != Decl)
                {
                    std::cout << "Type declaration already exists but is from a different module or declaration" << std::endl;
                    std::cout << "Existing: " << ExistingTypeDecl.Module->getName() << " " << ExistingTypeDecl.Decl->getName() << std::endl;
                    std::cout << "New: " << Module->getName() << " " << Decl->getName() << std::endl;
                    assert(false);
                }
            }
            else 
            {
                SlangTypeDeclaration NewTypeDecl;
                NewTypeDecl.TypeName = std::string(TypeName);
                NewTypeDecl.SourceLocation = loc;
                NewTypeDecl.Module = Module;
                NewTypeDecl.Decl = Decl;
                TypeDeclarations.push_back(NewTypeDecl);
            }
        }
    }

    // Recursively process children
    unsigned int ChildCount = Decl->getChildrenCount();
    for (unsigned int i = 0; i < ChildCount; i++)
    {
        slang::DeclReflection* Child = Decl->getChild(i);
        CollectTypeDeclarations(Module, Child);
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
        if (HasTypeDeclaration(TypeName))
        {
            auto& TypeDecl = GetTypeDeclaration(TypeName);
            EmitCppStructForThisType(TypeDecl, Container, TypeLayout);
            EmitMetadataForThisType(TypeDecl, Container, TypeLayout);
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
