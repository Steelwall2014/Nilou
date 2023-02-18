#pragma once
#include <map>
#include "HashedName.h"
// #include "Material.h"
// #include "Shader.h"
// #include "VertexFactory.h"
#include "ShaderInstance.h"
#include "Thread.h"
#include "Common/Log.h"

namespace nilou {

    // class FGlobalShaderMap &GetGlobalShaderMap();
    // std::unordered_map<FHashedName, class FMaterialShaderMap *> &GetMaterialShaderMap();
    // FShaderInstance *GetVertexShaderInstance(
    //     const FMaterialType &MaterialType,
    //     const FVertexFactoryType &VertexFactoryType,
    //     const FShaderPermutationParameters &PermutationParameters
    // );
    // FShaderInstance *GetPixelShaderInstance(
    //     const FMaterialType &MaterialType,
    //     const FShaderPermutationParameters &PermutationParameters
    // );
    // FShaderInstance *GetGlobalShaderInstance(
    //     const FShaderPermutationParameters &PermutationParameters
    // );
    
    // void AddVertexShaderInstance(
    //     FShaderInstanceRef ShaderInstance,
    //     const FMaterialType &MaterialType,
    //     const FVertexFactoryType &VertexFactoryType,
    //     const FShaderPermutationParameters &PermutationParameters
    // );
    // void AddPixelShaderInstance(
    //     FShaderInstanceRef ShaderInstance,
    //     const FMaterialType &MaterialType,
    //     const FShaderPermutationParameters &PermutationParameters
    // );
    // void AddGlobalShaderInstance(
    //     FShaderInstanceRef ShaderInstance,
    //     const FShaderPermutationParameters &PermutationParameters
    // );

    template<class TPermutationParameters, class... Ts>
    class TShaderMap
    {  
    public:
        using Type = TShaderMap<TPermutationParameters, Ts...>;
        using ValueType = TShaderMap<Ts...>;

        TShaderMap() 
        {
            static_assert(
                std::is_same<TPermutationParameters, FShaderPermutationParameters>::value || 
                std::is_same<TPermutationParameters, FVertexFactoryPermutationParameters>::value,
                "TShaderMap template parameters MUST be FShaderPermutationParameters or FVertexFactoryPermutationParameters");
        }

        FShaderInstance *GetShader(const TPermutationParameters &InParameters, const Ts&... Args) const 
        {
            const FHashedName HashedName = InParameters.Type->GetHashedFileName();

            auto iter = Shaders.find(HashedName);
            if (iter != Shaders.end())
            {
                const std::vector<ValueType> &ShaderInstances = iter->second;
                return ShaderInstances[InParameters.PermutationId].GetShader(Args...);
            }
            return nullptr;
        }

        /** Add a FShaderInstanceRef to the shader map. If already exists, the existing FShaderInstanceRef will be replaced. */
        void AddShader(FShaderInstanceRef InShaderRHI, const TPermutationParameters &InParameters, const Ts&... Args)
        {

            const FHashedName HashedName = InParameters.Type->GetHashedFileName();
            if (Shaders.find(HashedName) == Shaders.end())
            {
                Shaders[HashedName] = std::vector<ValueType>(InParameters.Type->PermutationCount);
            }

            Shaders[HashedName][InParameters.PermutationId].AddShader(InShaderRHI, Args...);
        }

        void RemoveAllShaders()
        {
            if (!IsInRenderingThread())
                NILOU_LOG(Fatal, "FMaterialShaderMap::RemoveAllShaders MUST be called from rendering thread!")

            for (auto &[key, value] : Shaders)
            {
                Shaders[key].clear();
            }
        }

        std::unordered_map<FHashedName, std::vector<ValueType>> Shaders;
    };

    template<>
    class TShaderMap<FShaderPermutationParameters>
    {  
    public:

        FShaderInstance *GetShader(const FShaderPermutationParameters &InParameters) const 
        {
            const FHashedName HashedName = InParameters.Type->GetHashedFileName();

            auto iter = Shaders.find(HashedName);
            if (iter != Shaders.end())
            {
                const std::vector<FShaderInstanceRef> &ShaderInstances = iter->second;
                return ShaderInstances[InParameters.PermutationId].get();
            }
            return nullptr;
        }

        /** Add a FShaderInstanceRef to the shader map. If already exists, the existing FShaderInstanceRef will be replaced. */
        void AddShader(FShaderInstanceRef InShaderRHI, const FShaderPermutationParameters &InParameters)
        {

            const FHashedName HashedName = InParameters.Type->GetHashedFileName();
            if (Shaders.find(HashedName) == Shaders.end())
            {
                Shaders[HashedName] = std::vector<FShaderInstanceRef>(InParameters.Type->PermutationCount, nullptr);
            }

            Shaders[HashedName][InParameters.PermutationId] = InShaderRHI;
        }

        void RemoveAllShaders()
        {
            if (!IsInRenderingThread())
                NILOU_LOG(Fatal, "FMaterialShaderMap::RemoveAllShaders MUST be called from rendering thread!")

            for (auto &[key, value] : Shaders)
            {
                Shaders[key].clear();
            }
        }

        std::unordered_map<FHashedName, std::vector<FShaderInstanceRef>> Shaders;
    };

    class FMaterialShaderMap
    {
        friend class FShaderCompiler;
    public:
        FShaderInstance *GetShader(
            const FVertexFactoryPermutationParameters &VFParameter, 
            const FShaderPermutationParameters &ShaderParameter)
        {
            return VertexShaderMap.GetShader(VFParameter, ShaderParameter);
        }
        FShaderInstance *GetShader(const FShaderPermutationParameters &ShaderParameter)
        {
            return PixelShaderMap.GetShader(ShaderParameter);
        }

        void AddShader(
            FShaderInstanceRef Shader,
            const FVertexFactoryPermutationParameters &VFParameter, 
            const FShaderPermutationParameters &ShaderParameter)
        {
            VertexShaderMap.AddShader(Shader, VFParameter, ShaderParameter);
        }
        void AddShader(
            FShaderInstanceRef Shader,
            const FShaderPermutationParameters &ShaderParameter)
        {
            PixelShaderMap.AddShader(Shader, ShaderParameter);
        }

        void RemoveAllShaders()
        {
            if (!IsInRenderingThread())
                NILOU_LOG(Fatal, "FMaterialShaderMap::RemoveAllShaders MUST be called from rendering thread!")
            VertexShaderMap.RemoveAllShaders();
            PixelShaderMap.RemoveAllShaders();
        }
    
    private:
        TShaderMap<FVertexFactoryPermutationParameters, FShaderPermutationParameters> VertexShaderMap;
        TShaderMap<FShaderPermutationParameters> PixelShaderMap;

    };

    // class FShaderMapContent
    // {
    // public:
    //     inline FShaderInstance *GetShader(const FShaderType &InType, int32 PermutationId = 0) const 
    //     {
    //         const FHashedName HashedName = InType.GetHashedFileName();

    //         auto iter = ShaderMapSections.find(HashedName);
    //         if (iter != ShaderMapSections.end())
    //         {
    //             return iter->second.GetShader(PermutationId);
    //         }
    //         return nullptr;
    //     }

    //     /** Add a FShaderInstanceRef to the shader map. If already exists, the existing FShaderInstanceRef will be replaced. */
    //     inline void AddShader(FShaderInstanceRef InShaderRHI, const FShaderType &InType, int32 PermutationId = 0)
    //     {
    //         const FHashedName HashedName = InType.GetHashedFileName();

    //         // auto iter = GlobalShaderMapSections.find(HashedName);
    //         // if (iter == GlobalShaderMapSections.end())
    //         //     GlobalShaderMapSections[HashedName] = FShaderMapSection(InType);

    //         ShaderMapSections[HashedName].AddShader(InShaderRHI, PermutationId);
    //     }

    // protected:

    //     class FShaderMapSection
    //     {
    //     public:
    //         FShaderMapSection() { }
    //         FShaderMapSection(const FShaderType &InType)
    //             : ShaderType(&InType)
    //         { }
    //         inline FShaderInstance *GetShader(int32 PermutationId = 0) const 
    //         {
    //             if (Shaders.count(PermutationId))
    //             {
    //                 return Shaders.find(PermutationId)->second.get();
    //             }
    //             return nullptr;
    //         }

    //         inline void AddShader(FShaderInstanceRef InShaderRHI, int32 PermutationId = 0)
    //         {
    //             Shaders[PermutationId] = InShaderRHI;
    //         }
        
    //         const FShaderType *ShaderType;
    //     private:
    //                     /** PermutationId */
    //         std::unordered_map<int32, FShaderInstanceRef> Shaders;
    //     }; 

    //     std::unordered_map<FHashedName, FShaderMapSection> ShaderMapSections;
    // };

    // class FGlobalShaderMap
    // {
    // public:
    //     const FShaderMapContent *GetContent() const { return &Content; }
    //     inline FShaderInstance *GetShader(const FShaderType &InType, int32 PermutationId = 0) const 
    //     {
    //         return GetContent()->GetShader(InType, PermutationId);
    //     }

    //     /** Add a FShaderInstanceRef to the shader map. If already exists, the existing FShaderInstanceRef will be replaced. */
    //     inline void AddShader(FShaderInstanceRef InShaderRHI, const FShaderType &InType, int32 PermutationId = 0)
    //     {
    //         GetContent()->AddShader(InShaderRHI, InType, PermutationId);
    //     }
    // private:
    //     FShaderMapContent *GetContent() { return &Content; }
    //     FShaderMapContent Content;
    // };

    // class FMaterialShaderMap
    // {
    // public:
    //     const FShaderMapContent *GetContent(const FVertexFactoryType &InType) const 
    //     { 
    //         const FHashedName HashedName = InType.GetHashedFileName();

    //         auto iter = ContentsVS.find(HashedName);
    //         if (iter != ContentsVS.end())
    //             return &iter->second;
    //         else
    //             return nullptr;
    //     }
        
    //     inline FShaderInstance *GetShader(const FVertexFactoryType &InVertexFactoryType, const FShaderType &InType, int32 PermutationId = 0) const 
    //     {
    //         const FShaderMapContent *Content = GetContent(InVertexFactoryType);
    //         if (Content)
    //             return Content->GetShader(InType, PermutationId);
    //         else
    //             return nullptr;
    //     }
        
    //     inline FShaderInstance *GetShader(const FShaderType &InType, int32 PermutationId = 0) const 
    //     {
    //         const FShaderMapContent *Content = nullptr;
    //         Content = &ContentsPS;
    //         return Content->GetShader(InType, PermutationId);
    //     }

    //     /** Add a FShaderInstanceRef to the shader map. If a FShaderInstanceRef already exists, the existing FShaderInstanceRef will be replaced. */
    //     inline void AddShader(FShaderInstanceRef InShaderRHI, const FVertexFactoryType &InVertexFactoryType, const FShaderType &InType, int32 PermutationId = 0)
    //     {
    //         const FHashedName HashedName = InVertexFactoryType.GetHashedFileName();
    //         ContentsVS[HashedName].AddShader(InShaderRHI, InType, PermutationId);
    //     }
        
    //     inline void AddShader(FShaderInstanceRef InShaderRHI, const FShaderType &InType, int32 PermutationId = 0)
    //     {
    //         ContentsPS.AddShader(InShaderRHI, InType, PermutationId);
    //     }

    //     FShaderInstance *GetVertexShader(const FVertexFactoryType &InVertexFactoryType, const FShaderPermutationParameters &Parameters) const
    //     {
    //         check(Parameters.Type->ShaderFrequency == EShaderFrequency::SF_Vertex);
    //         return GetShader(InVertexFactoryType, *Parameters.Type, Parameters.PermutationId);
    //     }
    
    //     FShaderInstance *GetPixelShader(const FShaderPermutationParameters &Parameters) const
    //     {
    //         check(Parameters.Type->ShaderFrequency == EShaderFrequency::SF_Pixel);
    //         return GetShader(*Parameters.Type, Parameters.PermutationId);
    //     }

    //     void AddVertexShader(const FVertexFactoryType &InVertexFactoryType, const FShaderPermutationParameters &Parameters, FShaderInstanceRef ShaderInstance)
    //     {
    //         check(Parameters.Type->ShaderFrequency == EShaderFrequency::SF_Vertex);
    //         AddShader(ShaderInstance, InVertexFactoryType, *Parameters.Type, Parameters.PermutationId);
    //     }
    
    //     void AddPixelShader(const FShaderPermutationParameters &Parameters, FShaderInstanceRef ShaderInstance)
    //     {
    //         check(Parameters.Type->ShaderFrequency == EShaderFrequency::SF_Pixel);
    //         AddShader(ShaderInstance, *Parameters.Type, Parameters.PermutationId);
    //     }
    // private:
    //     // FShaderMapContent *GetContent(const FVertexFactoryType &InType)
    //     // { 
    //     //     const FHashedName HashedName = InType.GetHashedFileName();

    //     //     auto iter = ContentsVS.find(HashedName);
    //     //     if (iter != ContentsVS.end())
    //     //         return &iter->second;
    //     //     else
    //     //         return nullptr;
    //     // }
        
    //     std::unordered_map<FHashedName, FShaderMapContent> ContentsVS;
    //     FShaderMapContent ContentsPS;
    // };



    

    // TShaderMap<FShaderPermutationParameters> &GetGlobalShaderMap2();
    // TShaderMap<
    //     FVertexFactoryPermutationParameters, 
    //     FMaterialPermutationParameters, 
    //     FShaderPermutationParameters> &GetVertexMaterialShaderMap2();
    // TShaderMap<
    //     FMaterialPermutationParameters, 
    //     FShaderPermutationParameters> &GetPixelMaterialShaderMap2();
    // FShaderInstance *GetVertexShaderInstance2(
    //     const FVertexFactoryPermutationParameters &VFParameters,
    //     FMaterial *Material,
    //     const FShaderPermutationParameters &ShaderParameters
    // );
    // FShaderInstance *GetPixelShaderInstance2(
    //     FMaterial *Material,
    //     const FShaderPermutationParameters &ShaderParameters
    // );
    FShaderInstance *GetGlobalShaderInstance2(
        const FShaderPermutationParameters &ShaderParameters
    );
    
    // void AddVertexShaderInstance2(
    //     FShaderInstanceRef ShaderInstance,
    //     const FVertexFactoryPermutationParameters &VFParameters,
    //     FMaterial *Material,
    //     const FShaderPermutationParameters &ShaderParameters
    // );
    // void AddPixelShaderInstance2(
    //     FShaderInstanceRef ShaderInstance,
    //     FMaterial *Material,
    //     const FShaderPermutationParameters &ShaderParameters
    // );
    void AddGlobalShaderInstance2(
        FShaderInstanceRef ShaderInstance,
        const FShaderPermutationParameters &ShaderParameters
    );


}