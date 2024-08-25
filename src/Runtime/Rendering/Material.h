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

// The nlohmann::json here is used as a placeholder for any type of material parameter value 
namespace nilou {

    enum class EShadingModel : uint32
    {
        SM_Unlit = 0,
        SM_DefaultLit,
        SM_OceanSubsurface,
        SM_SkyAtmosphere,

        SM_ShadingModelNum
    };

    /** Defines the domain of a material. */
    enum EMaterialDomain : int
    {
        /** The material's attributes describe a 3d surface. */
        MD_Surface,
        /** The material's attributes describe a deferred decal, and will be mapped onto the decal's frustum. */
        MD_DeferredDecal,
        /** The material's attributes describe a light's distribution. */
        MD_LightFunction,
        /** The material's attributes describe a 3d volume. */
        MD_Volume,
        /** The material will be used in a custom post process pass. */
        MD_PostProcess,
        /** The material will be used for UMG or Slate UI */
        MD_UI,
        /** The material will be used for runtime virtual texture (Deprecated). */
        MD_RuntimeVirtualTexture,

        MD_MAX
    };

    struct NSTRUCT FMaterialParameterInfo
    {
        GENERATED_STRUCT_BODY()

        NPROPERTY()
        std::string Name;

        bool operator==(const FMaterialParameterInfo& Other) const = default;

        FMaterialParameterInfo() { }

        FMaterialParameterInfo(const std::string& InName) : Name(InName) { }
    };

    struct NSTRUCT FScalarParameterValue
    {
        GENERATED_STRUCT_BODY()

        using ValueType = float;
        static ValueType GetValue(const FScalarParameterValue& Parameter) { return Parameter.ParameterValue; }

        NPROPERTY()
        FMaterialParameterInfo ParameterInfo;

        NPROPERTY()
        float ParameterValue;
    };

    struct NSTRUCT FVectorParameterValue
    {
        GENERATED_STRUCT_BODY()

        using ValueType = vec4;
        static ValueType GetValue(const FVectorParameterValue& Parameter) { return Parameter.ParameterValue; }

        NPROPERTY()
        FMaterialParameterInfo ParameterInfo;

        NPROPERTY()
        vec4 ParameterValue;
    };

    struct NSTRUCT FTextureParameterValue
    {
        GENERATED_STRUCT_BODY()

        using ValueType = UTexture*;
        static ValueType GetValue(const FTextureParameterValue& Parameter) { return Parameter.ParameterValue; }

        NPROPERTY()
        FMaterialParameterInfo ParameterInfo;

        NPROPERTY()
        UTexture* ParameterValue;
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
        EMaterialDomain MaterialDomain = MD_Surface;

        NPROPERTY()
        std::vector<FScalarParameterValue> ScalarParameterValues;

        NPROPERTY()
        std::vector<FVectorParameterValue> VectorParameterValues;

        NPROPERTY()
        std::vector<FTextureParameterValue> TextureParameterValues;

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

        void SetScalarParameterValue(const std::string& Name, float Value);

        void SetVectorParameterValue(const std::string& Name, const vec4& Value);

        void SetTextureParameterValue(const std::string& Name, UTexture *Texture);

        bool SetScalarParameterValueByIndex(int32 ParameterIndex, float Value);

        bool SetVectorParameterValueByIndex(int32 ParameterIndex, const vec4& Value);

        bool SetTextureParameterValueByIndex(int32 ParameterIndex, UTexture *Texture);

        void SetShadingModel(EShadingModel InShadingModel);

        void SetBlendState(FBlendStateInitializer InBlendState);

        void SetDepthStencilState(FDepthStencilStateInitializer InDepthStencilState);

        void SetRasterizerState(FRasterizerStateInitializer InRasterizerState);

        void SetStencilRefValue(uint8 InStencilRefValue);
        
        FMaterialRenderProxy* GetRenderProxy() const
        {
            return MaterialRenderProxy.get();
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
        // While FMaterialRenderProxy contains uniform buffers, so it is UNIQUE for each material/material instance
        std::unique_ptr<FMaterialRenderProxy> MaterialRenderProxy = nullptr;  // It's called "DefaultMaterialInstance" in UE5
    };

    class NCLASS UMaterialInstance : public UMaterial
    {
        GENERATED_BODY()
    public:
        UMaterialInstance() { }

        NPROPERTY()
        UMaterial* Parent;
    };

    #define UNIFORMBUFFER_KEY(SetIndex, BindingIndex) (((uint64)(SetIndex) << 32) | (BindingIndex))

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

        // Since FMaterialShaderMap is the shaders for a material, so it may be SHARED by multiple materials e.g. material instances
        std::shared_ptr<FMaterialShaderMap> ShaderMap = nullptr;

        std::map<uint64, FTextureResource*> Textures;

        std::map<uint64, RDGBufferRef> UniformBuffers;

        struct ParameterPosition
        {
            uint32 SetIndex;
            uint32 BindingIndex;
            uint32 Offset;
        };
        std::map<std::string, ParameterPosition> ParameterNameToPosition;

        std::map<uint32, RDGDescriptorSetRef> DescriptorSets;

        uint8 StencilRefValue = 0;

        RHIRasterizerStateRef RasterizerState = nullptr;

        RHIDepthStencilStateRef DepthStencilState = nullptr;

        RHIBlendStateRef BlendState = nullptr;

        EShadingModel ShadingModel;

        UMaterial* Material;

        void RenderThread_UpdateShader(const std::string& ShaderCode);

        /**
         * Updates a named parameter on the render thread.
         */
        template <typename ValueType>
        void RenderThread_UpdateParameter(const FMaterialParameterInfo& ParameterInfo, const ValueType& Value)
        {
            auto Found = ParameterNameToPosition.find(ParameterInfo.Name);
            if (Found != ParameterNameToPosition.end())
            {
                ParameterPosition Position = Found->second;
                uint64 Key = UNIFORMBUFFER_KEY(Position.SetIndex, Position.BindingIndex);
                if constexpr (std::is_same_v<ValueType, UTexture*>)
                {
                    if (DescriptorSets.count(Position.SetIndex) != 0)
                    {
                        Textures[Key] = Value->GetResource();
                        DescriptorSets[Position.SetIndex]->SetSampler(Position.BindingIndex, Textures[Key]->GetSamplerState(), Textures[Key]->GetTextureRDG());
                    }
                }
                else 
                {
                    if (UniformBuffers.count(Key) != 0)
                        UniformBuffers[Key]->SetData(Value, Position.Offset);
                }
            }
        }

    };

    /** Finds a parameter by name from the game thread. */
    template <typename ParameterType>
    ParameterType* GameThread_FindParameterByName(std::vector<ParameterType>& Parameters, const FMaterialParameterInfo& ParameterInfo)
    {
        for (int32 ParameterIndex = 0; ParameterIndex < Parameters.Num(); ParameterIndex++)
        {
            ParameterType* Parameter = &Parameters[ParameterIndex];
            if (Parameter->ParameterInfo == ParameterInfo)
            {
                return Parameter;
            }
        }
        return nullptr;
    }

    /** Finds a parameter by index from the game thread. */
    template <typename ParameterType>
    ParameterType* GameThread_FindParameterByIndex(std::vector<ParameterType>& Parameters, int32 Index)
    {
        if (0 <= Index && Index < Parameters.size())
        {
            return &Parameters[Index];
        }

        return nullptr;
    }

}

// TStaticSerializer的用法是这样的：
// 有时候我们需要序列化一个类，这个类可能是来自库的，我们不能修改它的代码，
// 那我们可以把模板的特化写在头文件里，特化的实现写在cpp文件里，
// header tool生成的代码能看得到TStaticSerializer。