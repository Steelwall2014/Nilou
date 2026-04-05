#include "ShaderParameter.h"

namespace nilou {

static std::unordered_map<std::string, FShaderParametersMetadata2*> StructNameToMetadata;
static std::vector<std::function<std::pair<std::string, FShaderParametersMetadata2*>()>> RegisterFunctions;

void FShaderParameterRegistry::RegisterTypes()
{
    for (auto& Func : RegisterFunctions)
    {
        auto [StructName, Metadata] = Func();
        if (StructNameToMetadata.contains(StructName))
        {
            NILOU_LOG(Fatal, "Struct {} already registered", StructName);
        }
        StructNameToMetadata[StructName] = Metadata;
    }
}

FShaderParameterRegistry::FShaderParameterRegistry(std::function<std::pair<std::string, FShaderParametersMetadata2*>()> GetStructNameAndMetadata)
{
    RegisterFunctions.push_back(GetStructNameAndMetadata);
}

FShaderParametersMetadata2* GetShaderParametersMetadata(const std::string& StructName)
{
    if (StructNameToMetadata.contains(StructName))
    {
        return StructNameToMetadata[StructName];
    }
    return nullptr;
}

}