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

            Shaders.clear();
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

            Shaders.clear();
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

}