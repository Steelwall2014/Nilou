#include "ShaderMap.h"

namespace nilou {

    // FGlobalShaderMap *GGlobalShaderMap = new FGlobalShaderMap();
    // std::unordered_map<FHashedName, FMaterialShaderMap *> *GMaterialShaderMap = new std::unordered_map<FHashedName, FMaterialShaderMap *>();
    
    // FGlobalShaderMap &GetGlobalShaderMap()
    // {
    //     return *GGlobalShaderMap;
    // }
    // std::unordered_map<FHashedName, FMaterialShaderMap *> &GetMaterialShaderMap()
    // {
    //     return *GMaterialShaderMap;
    // }
    // FShaderInstance *GetVertexShaderInstance(
    //     const FMaterialType &MaterialType,
    //     const FVertexFactoryType &VertexFactoryType,
    //     const FShaderPermutationParameters &PermutationParameters
    // )
    // {
    //     auto iter = GetMaterialShaderMap().find(MaterialType.GetHashedFileName());
    //     if (iter != GetMaterialShaderMap().end())
    //     {
    //         return iter->second->GetVertexShader(VertexFactoryType, PermutationParameters);
    //     }
    //     return nullptr;
    // }
    // FShaderInstance *GetPixelShaderInstance(
    //     const FMaterialType &MaterialType,
    //     const FShaderPermutationParameters &PermutationParameters
    // )
    // {
    //     auto iter = GetMaterialShaderMap().find(MaterialType.GetHashedFileName());
    //     if (iter != GetMaterialShaderMap().end())
    //     {
    //         return iter->second->GetPixelShader(PermutationParameters);
    //     }
    //     return nullptr;
    // }
    // FShaderInstance *GetGlobalShaderInstance(const FShaderPermutationParameters &PermutationParameters)
    // {
    //     return GetGlobalShaderMap().GetShader(*PermutationParameters.Type, PermutationParameters.PermutationId);
    // }


    // void AddVertexShaderInstance(
    //     FShaderInstanceRef ShaderInstance,
    //     const FMaterialType &MaterialType,
    //     const FVertexFactoryType &VertexFactoryType,
    //     const FShaderPermutationParameters &PermutationParameters
    // )
    // {
    //     auto &MaterialShaderMap = GetMaterialShaderMap();
    //     if (MaterialShaderMap.find(MaterialType.GetHashedFileName()) == GetMaterialShaderMap().end())
    //     {
    //         MaterialShaderMap[MaterialType.GetHashedFileName()] = new FMaterialShaderMap;
    //     }
    //     MaterialShaderMap[MaterialType.GetHashedFileName()]->AddVertexShader(VertexFactoryType, PermutationParameters, ShaderInstance);
    // }
    // void AddPixelShaderInstance(
    //     FShaderInstanceRef ShaderInstance,
    //     const FMaterialType &MaterialType,
    //     const FShaderPermutationParameters &PermutationParameters
    // )
    // {
    //     auto &MaterialShaderMap = GetMaterialShaderMap();
    //     if (MaterialShaderMap.find(MaterialType.GetHashedFileName()) == GetMaterialShaderMap().end())
    //     {
    //         MaterialShaderMap[MaterialType.GetHashedFileName()] = new FMaterialShaderMap;
    //     }
    //     MaterialShaderMap[MaterialType.GetHashedFileName()]->AddPixelShader(PermutationParameters, ShaderInstance);
    // }
    // void AddGlobalShaderInstance(
    //     FShaderInstanceRef ShaderInstance,
    //     const FShaderPermutationParameters &PermutationParameters
    // )
    // {
    //     GetGlobalShaderMap().AddShader(ShaderInstance, *PermutationParameters.Type, PermutationParameters.PermutationId);
    // }


    
    TShaderMap<FShaderPermutationParameters> &GetGlobalShaderMap2()
    {
        static TShaderMap<FShaderPermutationParameters> GlobalShaderMap;
        return GlobalShaderMap;
    }
    // TShaderMap<
    //     FVertexFactoryPermutationParameters, 
    //     FMaterial, 
    //     FShaderPermutationParameters> &GetVertexMaterialShaderMap2()
    // {
    //     static TShaderMap<
    //         FVertexFactoryPermutationParameters, 
    //         FMaterial, 
    //         FShaderPermutationParameters> VertexMaterialShaderMap;
    //     return VertexMaterialShaderMap;
    // }
    // TShaderMap<
    //     FMaterial, 
    //     FShaderPermutationParameters> &GetPixelMaterialShaderMap2()
    // {
    //     static TShaderMap<
    //         FMaterial, 
    //         FShaderPermutationParameters> PixelMaterialShaderMap;
    //     return PixelMaterialShaderMap;
    // }
    // FShaderInstance *GetVertexShaderInstance2(
    //     const FVertexFactoryPermutationParameters &VFParameters,
    //     FMaterial *Material,
    //     const FShaderPermutationParameters &ShaderParameters
    // )
    // {
    //     return GetVertexMaterialShaderMap2().GetShader(VFParameters, MaterialParameters, ShaderParameters);
    // }
    // FShaderInstance *GetPixelShaderInstance2(
    //     FMaterial *Material,
    //     const FShaderPermutationParameters &ShaderParameters
    // )
    // {
    //     return GetPixelMaterialShaderMap2().GetShader(MaterialParameters, ShaderParameters);
    // }
    FShaderInstance *GetGlobalShaderInstance2(
        const FShaderPermutationParameters &ShaderParameters
    )
    {
        return GetGlobalShaderMap2().GetShader(ShaderParameters);
    }
    
    // void AddVertexShaderInstance2(
    //     FShaderInstanceRef ShaderInstance,
    //     const FVertexFactoryPermutationParameters &VFParameters,
    //     FMaterial *Material,
    //     const FShaderPermutationParameters &ShaderParameters
    // )
    // {
    //     GetVertexMaterialShaderMap2().AddShader(ShaderInstance, VFParameters, MaterialParameters, ShaderParameters);
    // }
    // void AddPixelShaderInstance2(
    //     FShaderInstanceRef ShaderInstance,
    //     FMaterial *Material,
    //     const FShaderPermutationParameters &ShaderParameters
    // )
    // {
    //     GetPixelMaterialShaderMap2().AddShader(ShaderInstance, MaterialParameters, ShaderParameters);
    // }
    void AddGlobalShaderInstance2(
        FShaderInstanceRef ShaderInstance,
        const FShaderPermutationParameters &ShaderParameters
    )
    {
        GetGlobalShaderMap2().AddShader(ShaderInstance, ShaderParameters);
    }

}