#include <cassert>
#include <map>
#include <slang.h>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <slang-com-ptr.h>

enum class EShaderDataLayout
{
    Std140,
    Std430,
    Opaque,
};
inline std::string GetShaderDataLayoutName(EShaderDataLayout DataLayout)
{
    switch (DataLayout) 
    {
    case EShaderDataLayout::Std140:
        return "EShaderDataLayout::Std140";
    case EShaderDataLayout::Std430:
        return "EShaderDataLayout::Std430";
    case EShaderDataLayout::Opaque:
        return "EShaderDataLayout::Opaque";
    default:
        assert(false);
    }
    return "";
}

struct SlangTypeDeclaration
{
    std::string TypeName;
    slang::TypeReflection* Type;
    slang::SourceLocation SourceLocation;
    slang::IModule* Module; // The module that this type declaration is from
    slang::DeclReflection* Decl; // The declaration that this type declaration is from
    std::unordered_map<EShaderDataLayout, std::string> CppStructs;  // Generated cpp struct declarations
    std::string CppMetadata;
};

class SlangShaderReflectionSession
{
public:
    SlangShaderReflectionSession(slang::ISession* SlangSession) : SlangSession(SlangSession) {}

    bool LoadModule(const std::filesystem::path& SlangFilePath);
    void EmitCppStructs();

    // We postorder traverse the structure declarations, so that the member structure declarations of a structure are guaranteed to be traversed before it.
    // Therefore, we need to ensure the order of type declarations when storing them, we use vector instead of unordered_map.
    std::vector<SlangTypeDeclaration> TypeDeclarations;

    bool HasTypeDeclaration(slang::TypeReflection* Type) const
    {
        for (auto& TypeDecl : TypeDeclarations)
        {
            if (TypeDecl.Type == Type)
            {
                return true;
            }
        }
        return false;
    }
    
    SlangTypeDeclaration& GetTypeDeclaration(slang::TypeReflection* Type)
    {
        for (auto& TypeDecl : TypeDeclarations)
        {
            if (TypeDecl.Type == Type)
            {
                return TypeDecl;
            }
        }
        assert(false);
        return TypeDeclarations[0];
    }

private:
    slang::ISession* SlangSession;

    std::map<std::string, slang::IModule*> Modules;

    void CollectTypeDeclarations(slang::IModule* Module, slang::DeclReflection* Decl);

    void EnumerateStructTypeLayoutsRecursive(slang::TypeLayoutReflection* Container, slang::TypeLayoutReflection* TypeLayout);
    
};

void EmitCppStructs(
    std::vector<SlangTypeDeclaration>& OutTypeDeclarations, 
    slang::ISession* Session, 
    slang::IModule* Module);

void CollectTypeDeclarations(
    slang::ISession* Session, 
    slang::DeclReflection* Decl, 
    std::vector<SlangTypeDeclaration>& OutTypeDeclarations);
