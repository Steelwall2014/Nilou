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
            return MaterialRenderProxy;
        }

        void SetShaderFileVirtualPath(const std::string& VirtualPath);

        UMaterialInstance* CreateMaterialInstance();

        std::string GetMateiralCode() const { return Code; }

        virtual void PostSerialize(FArchive& Ar) override;

        virtual void PostDeserialize(FArchive& Ar) override;

        void InitializeResources();

        void ReleaseResources();

    protected:

        std::string Code;
        FMaterialRenderProxy* MaterialRenderProxy = nullptr;  // It's called "DefaultMaterialInstance" in UE5
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

        uint8 StencilRefValue = 0;

        RHIRasterizerStateRef RasterizerState = nullptr;

        RHIDepthStencilStateRef DepthStencilState = nullptr;

        RHIBlendStateRef BlendState = nullptr;

        EShadingModel ShadingModel = EShadingModel::SM_DefaultLit;

        UMaterial* Owner;

        void RenderThread_UpdateShader(const std::string& ShaderCode);

        /**
         * Updates a named parameter on the render thread.
         */
        template <typename ValueType>
        void RenderThread_UpdateParameter(const FMaterialParameterInfo& ParameterInfo, const ValueType& Value)
        {
            auto& ValueArray = GetValueArray<ValueType>();
            ValueArray[ParameterInfo.Name] = Value;
        }

        FMeshDrawShaderBindings GetShaderBindings() const;

    private:

        template <typename ValueType> std::map<std::string, ValueType>& GetValueArray() { return ScalarParameterArray; }
        template <typename ValueType> const std::map<std::string, ValueType>& GetValueArray() const { return ScalarParameterArray; }

        std::map<std::string, vec4> VectorParameterArray;
        std::map<std::string, float> ScalarParameterArray;
        // Note by Steelwall2014:
        // In UE5.5, the TextureParametersArray is a map of UTexture*, because they can check if the pointer is valid. (see FUniformExpressionSet::FillUniformBuffer)
        // So actually we have a risk of wild pointer here, but we haven't have a good way to solve this.
        std::map<std::string, UTexture*> TextureParameterArray;

        RDGBufferRef UniformBufferRDG = nullptr;
    };

    template <> inline std::map<std::string, float>& FMaterialRenderProxy::GetValueArray<float>() { return ScalarParameterArray; }
    template <> inline std::map<std::string, vec4>& FMaterialRenderProxy::GetValueArray<vec4>() { return VectorParameterArray; }
    template <> inline std::map<std::string, UTexture*>& FMaterialRenderProxy::GetValueArray<UTexture*>() { return TextureParameterArray; }
    template <> inline const std::map<std::string, float>& FMaterialRenderProxy::GetValueArray<float>() const { return ScalarParameterArray; }
    template <> inline const std::map<std::string, vec4>& FMaterialRenderProxy::GetValueArray<vec4>() const { return VectorParameterArray; }
    template <> inline const std::map<std::string, UTexture*>& FMaterialRenderProxy::GetValueArray<UTexture*>() const { return TextureParameterArray; }

    /** Finds a parameter by name from the game thread. */
    template <typename ParameterType>
    ParameterType* GameThread_FindParameterByName(std::vector<ParameterType>& Parameters, const FMaterialParameterInfo& ParameterInfo)
    {
        for (int32 ParameterIndex = 0; ParameterIndex < Parameters.size(); ParameterIndex++)
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
