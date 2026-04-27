#pragma once
#include <map>
#include "Serialization/Archive.h"
// #include "Materials/Material.h"
#include "Shader.h"
// #include "VertexFactory.h"
#include "ShaderInstance.h"
#include "HAL/Thread.h"
#include "Logging/LogMacros.h"

namespace nilou {

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

        RHIShader *GetShader(const TPermutationParameters &InParameters, const Ts&... Args) const 
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

        /** Add a RHIShaderRef to the shader map. If already exists, the existing RHIShaderRef will be replaced. */
        void AddShader(RHIShaderRef InShaderRHI, const TPermutationParameters &InParameters, const Ts&... Args)
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

            Shaders.clear();
        }

        std::unordered_map<FHashedName, std::vector<ValueType>> Shaders;
    };

    template<>
    class TShaderMap<FShaderPermutationParameters>
    {  
    public:

        RHIShader *GetShader(const FShaderPermutationParameters &InParameters) const 
        {
            const FHashedName HashedName = InParameters.Type->GetHashedFileName();

            auto iter = Shaders.find(HashedName);
            if (iter != Shaders.end())
            {
                const std::vector<RHIShaderRef> &ShaderInstances = iter->second;
                return ShaderInstances[InParameters.PermutationId].GetReference();
            }
            return nullptr;
        }

        /** Add a RHIShaderRef to the shader map. If already exists, the existing RHIShaderRef will be replaced. */
        void AddShader(RHIShaderRef InShaderRHI, const FShaderPermutationParameters &InParameters)
        {

            const FHashedName HashedName = InParameters.Type->GetHashedFileName();
            if (Shaders.find(HashedName) == Shaders.end())
            {
                Shaders[HashedName] = std::vector<RHIShaderRef>(InParameters.Type->PermutationCount, nullptr);
            }

            Shaders[HashedName][InParameters.PermutationId] = InShaderRHI;
        }

        void RemoveAllShaders()
        {
            if (!IsInRenderingThread())
                NILOU_LOG(Fatal, "FMaterialShaderMap::RemoveAllShaders MUST be called from rendering thread!")

            Shaders.clear();
        }

        std::unordered_map<FHashedName, std::vector<RHIShaderRef>> Shaders;
    };

    template<class TPermutationParameters, class... Ts>
    class TPipelineMap
    {  
    public:
        using Type = TPipelineMap<TPermutationParameters, Ts...>;
        using ValueType = TPipelineMap<Ts...>;

        TPipelineMap() 
        {
            static_assert(
                std::is_same<TPermutationParameters, FShaderPermutationParameters>::value || 
                std::is_same<TPermutationParameters, FVertexFactoryPermutationParameters>::value,
                "TPipelineMap template parameters MUST be FShaderPermutationParameters or FVertexFactoryPermutationParameters");
        }

        RHIShader *GetShader(const TPermutationParameters &InParameters, const Ts&... Args) const 
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

        /** Add a RHIShaderRef to the shader map. If already exists, the existing RHIShaderRef will be replaced. */
        void AddShader(RHIShaderRef InShaderRHI, const TPermutationParameters &InParameters, const Ts&... Args)
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

            Shaders.clear();
        }

        std::unordered_map<FHashedName, std::vector<ValueType>> Shaders;
    };

    template<>
    class TPipelineMap<FShaderPermutationParameters>
    {  
    public:

        RHIShader *GetShader(const FShaderPermutationParameters &InParameters) const 
        {
            const FHashedName HashedName = InParameters.Type->GetHashedFileName();

            auto iter = Shaders.find(HashedName);
            if (iter != Shaders.end())
            {
                const std::vector<RHIShaderRef> &ShaderInstances = iter->second;
                return ShaderInstances[InParameters.PermutationId].GetReference();
            }
            return nullptr;
        }

        /** Add a RHIShaderRef to the shader map. If already exists, the existing RHIShaderRef will be replaced. */
        void AddShader(RHIShaderRef InShaderRHI, const FShaderPermutationParameters &InParameters)
        {

            const FHashedName HashedName = InParameters.Type->GetHashedFileName();
            if (Shaders.find(HashedName) == Shaders.end())
            {
                Shaders[HashedName] = std::vector<RHIShaderRef>(InParameters.Type->PermutationCount, nullptr);
            }

            Shaders[HashedName][InParameters.PermutationId] = InShaderRHI;
        }

        void RemoveAllShaders()
        {
            if (!IsInRenderingThread())
                NILOU_LOG(Fatal, "FMaterialShaderMap::RemoveAllShaders MUST be called from rendering thread!")

            Shaders.clear();
        }

        std::unordered_map<FHashedName, std::vector<RHIShaderRef>> Shaders;
    };

    /**
     * Stores compiled material graphics pipelines (VS+PS pairs) keyed by:
     *   Pipeline type → [Pipeline permutation] → VF type → [VF permutation] → RHIGraphicsPipelineShaders
     */
    class FMaterialPipelineMap
    {
        friend class FShaderCompiler;
    public:
        void AddPipeline(
            const FGraphicsPipelinePermutationParameters& PipelineParams,
            const FVertexFactoryPermutationParameters& VFParams,
            const RHIGraphicsPipelineShaders& Shaders)
        {
            const FHashedName PipelineKey = PipelineParams.Type->HashedName;
            const FHashedName VFKey = VFParams.Type->GetHashedFileName();

            auto& PermVec = Pipelines[PipelineKey];
            if ((int32)PermVec.size() < PipelineParams.Type->PermutationCount)
                PermVec.resize(PipelineParams.Type->PermutationCount);

            auto& VFMap = PermVec[PipelineParams.PermutationId];
            auto& VFPermVec = VFMap[VFKey];
            if ((int32)VFPermVec.size() < VFParams.Type->PermutationCount)
                VFPermVec.resize(VFParams.Type->PermutationCount);

            VFPermVec[VFParams.PermutationId] = Shaders;
        }

        RHIGraphicsPipelineShaders* GetPipelineShaders(
            const FGraphicsPipelinePermutationParameters& PipelineParams,
            const FVertexFactoryPermutationParameters& VFParams)
        {
            const FHashedName PipelineKey = PipelineParams.Type->HashedName;
            const FHashedName VFKey = VFParams.Type->GetHashedFileName();

            auto pIter = Pipelines.find(PipelineKey);
            if (pIter == Pipelines.end())
                return nullptr;

            auto& VFMap = pIter->second[PipelineParams.PermutationId];
            auto vfIter = VFMap.find(VFKey);
            if (vfIter == VFMap.end())
                return nullptr;

            return &vfIter->second[VFParams.PermutationId];
        }

        void RemoveAllPipelines()
        {
            if (!IsInRenderingThread())
                NILOU_LOG(Fatal, "FMaterialPipelineMap::RemoveAllPipelines MUST be called from rendering thread!")
            Pipelines.clear();
        }

    private:
        // Pipeline HashedName → [Pipeline perm] → VF HashedName → [VF perm] → RHIGraphicsPipelineShaders
        std::unordered_map<FHashedName,
            std::vector<
                std::unordered_map<FHashedName,
                    std::vector<RHIGraphicsPipelineShaders>
                >
            >
        > Pipelines;
    };

    class FMaterialShaderMap
    {
        friend class FShaderCompiler;
    public:
        RHIGraphicsPipelineShaders* GetPipelineShaders(
            const FGraphicsPipelinePermutationParameters& PipelineParams,
            const FVertexFactoryPermutationParameters& VFParams)
        {
            return PipelineMap.GetPipelineShaders(PipelineParams, VFParams);
        }

        void AddPipeline(
            const FGraphicsPipelinePermutationParameters& PipelineParams,
            const FVertexFactoryPermutationParameters& VFParams,
            const RHIGraphicsPipelineShaders& Shaders)
        {
            PipelineMap.AddPipeline(PipelineParams, VFParams, Shaders);
        }

        void RemoveAllShaders()
        {
            if (!IsInRenderingThread())
                NILOU_LOG(Fatal, "FMaterialShaderMap::RemoveAllShaders MUST be called from rendering thread!")
            PipelineMap.RemoveAllPipelines();
        }

    private:
        FMaterialPipelineMap PipelineMap;

    };

}