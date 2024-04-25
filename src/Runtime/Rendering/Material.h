#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "UniformBuffer.h"
#include "MeshPassProcessor.h"
#include "RHI.h"
#include "RHIResources.h"
#include "RHIStaticStates.h"
#include "Shader.h"
#include "ShaderType.h"
#include "ShaderMap.h"
#include "Templates/ObjectMacros.h"
#include "Texture.h"
#include "VertexFactory.h"
#include "ShaderParameter.h"
#include "Common/ContentManager.h"

namespace nilou {

    enum class EShadingModel : uint32
    {
        SM_Unlit = 0,
        SM_DefaultLit,
        SM_OceanSubsurface,
        SM_SkyAtmosphere,

        SM_ShadingModelNum
    };

    
    class FMaterialUniformBuffer : public FUniformBuffer
    {
    public:
        friend class UMaterial;

        FMaterialUniformBuffer() { }

        struct Field
        {
            std::string Name;
            std::string Type;   // only "float","vec2","vec3","vec4" are supported
            uint32 Offset;
        };

        void SetScalarParameterValue(const std::string& Name, float Value)
        {
            auto Found = Fields.find(Name);
            if (Found != Fields.end())
            {
                float* ptr = (float*)(Data.get() + Found->second.Offset);
                *ptr = Value;
            }
        }

        template<typename T>
        void SetVectorParameterValue(const std::string& Name, const T& Value)
        {
            static_assert(
                std::is_same_v<T, vec2> || 
                std::is_same_v<T, vec3> || 
                std::is_same_v<T, vec4>, "FMaterialUniformBuffer::SetVectorParameterValue T must be vec2,vec3 or vec4");
            auto Found = Fields.find(Name);
            if (Found != Fields.end())
            {
                T* ptr = (T*)(Data.get() + Found->second.Offset);
                *ptr = Value;
            }
        }

        template<typename T>
        T GetField(const std::string& Name)
        {
            auto Found = Fields.find(Name);
            if (Found != Fields.end())
            {
                return _GetField<T>(Found->second.Offset);
            }
            return T();
        }

        /** Begin FRenderResource Interface */
        virtual void InitRHI() override;
        virtual void ReleaseRHI() override;
        /** End FRenderResource Interface */

        void UpdateUniformBuffer();

        void Serialize(FArchive& Ar);
        void Deserialize(FArchive& Ar);

    protected:

        std::unique_ptr<uint8[]> Data = nullptr;
        uint32 Size = 0;
        EUniformBufferUsage Usage = EUniformBufferUsage::UniformBuffer_MultiFrame;
        std::map<std::string, Field> Fields;


        template<typename T>
        T _GetField(int32 Offset) 
        { 
            static_assert(
                std::is_same_v<T, float> || 
                std::is_same_v<T, vec2> || 
                std::is_same_v<T, vec3> || 
                std::is_same_v<T, vec4>, "FMaterialUniformBuffer::GetField T must be float,vec2,vec3 or vec4");
            T out = *(T*)(Data.get() + Offset);
            return out;
        }
    };

    class NCLASS UMaterial : public NAsset
    {
        friend class UMaterialInstance;
        friend class FMaterialRenderProxy;
        GENERATED_BODY()
    public:

        UMaterial();

        virtual ~UMaterial();

        static UMaterial *GetDefaultMaterial();

        NPROPERTY()
        std::string ShaderVirtualPath;

        NPROPERTY()
        std::map<std::string, UTexture *> Textures;

        NPROPERTY()
        FRasterizerStateInitializer RasterizerState;

        NPROPERTY()
        FDepthStencilStateInitializer DepthStencilState;

        NPROPERTY()
        FBlendStateInitializer BlendState;

        NPROPERTY()
        EShadingModel ShadingModel = EShadingModel::SM_DefaultLit;

        NPROPERTY()
        uint8 StencilRefValue = 0;

        void UpdateCode(const std::string &InCode);

        void SetTextureParameterValue(const std::string &Name, UTexture *Texture);

        void SetScalarParameterValue(const std::string &Name, float Value);

        void SetVectorParameterValue(const std::string& Name, const vec2& Value);

        void SetVectorParameterValue(const std::string& Name, const vec3& Value);

        void SetVectorParameterValue(const std::string& Name, const vec4& Value);

        void SetShadingModel(EShadingModel InShadingModel);

        void SetBlendState(FBlendStateInitializer InBlendState);

        void SetDepthStencilState(FDepthStencilStateInitializer InDepthStencilState);

        void SetRasterizerState(FRasterizerStateInitializer InRasterizerState);

        void SetStencilRefValue(uint8 InStencilRefValue);
        
        FMaterialRenderProxy* GetRenderProxy() const
        {
            return MaterialRenderProxy;
        }

        void SetShaderFileVirtualPath(const std::string& VirtualPath);

        UMaterialInstance* CreateMaterialInstance();

        std::string GetMateiralCode() const { return Code; }

        virtual void PostSerialize(FArchive& Ar) override;

        virtual void PostDeserialize(FArchive& Ar) override;

        void ReleaseResources()
        {
        }

    protected:

        std::string Code;

        // Since FMaterialShaderMap is the shaders for a material, so it may be SHARED by multiple materials e.g. material instances
        std::shared_ptr<FMaterialShaderMap> MaterialShaderMap = nullptr;
        // While FMaterialRenderProxy contains uniform buffers, so it is UNIQUE for each material/material instance
        // However, std::function requires the functor to be copyable, so we use naked pointer here.
        // So, be careful when you use this pointer, it may be invalid if the material is destroyed.
        FMaterialRenderProxy* MaterialRenderProxy = nullptr;  // It's called "DefaultMaterialInstance" in UE5
        // Same as above
        FMaterialUniformBuffer* MaterialUniformBlock = nullptr;
        
    };

    class NCLASS UMaterialInstance : public UMaterial
    {
        GENERATED_BODY()
    public:
        UMaterialInstance() { }
    };

    class FMaterialRenderProxy
    {
        friend class UMaterial;
        friend class UMaterialInstance;
    public:
        FMaterialRenderProxy(UMaterial* InMaterial);

        FShaderInstance *GetShader(
            const FVertexFactoryPermutationParameters VFParameter, 
            const FShaderPermutationParameters &ShaderParameter) const
        {
            return ShaderMap->GetShader(VFParameter, ShaderParameter);
        }
        FShaderInstance *GetShader(const FShaderPermutationParameters &ShaderParameter) const
        {
            return ShaderMap->GetShader(ShaderParameter);
        }

        void FillShaderBindings(FInputShaderBindings &OutBindings)
        { 
            for (auto &[Name, Texture] : Textures)
                OutBindings.SetElementShaderBinding(Name, Texture->GetSamplerRHI());
            if (UniformBuffer)
                OutBindings.SetElementShaderBinding("MAT_UNIFORM_BLOCK", UniformBuffer->GetRHI());
        }

        FMaterialShaderMap* ShaderMap;

        std::map<std::string, FTextureResource *> Textures;

        FMaterialUniformBuffer* UniformBuffer = nullptr;

        uint8 StencilRefValue = 0;

        RHIRasterizerStateRef RasterizerState = nullptr;

        RHIDepthStencilStateRef DepthStencilState = nullptr;

        RHIBlendStateRef BlendState = nullptr;

        EShadingModel ShadingModel;

        UMaterial* Material;

    };

}