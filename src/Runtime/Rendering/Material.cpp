#include "Common/Path.h"
#include "Material.h"
#include "Common/Asset/AssetLoader.h"
#include "RHIDefinitions.h"
#include "Templates/ObjectMacros.h"
#include "Common/Path.h"
#include "Common/ContentManager.h"
#include "RenderingThread.h"
#include <fstream>

namespace fs = std::filesystem;

namespace nilou {

    const std::filesystem::path MATERIAL_STATIC_PARENT_DIR = FPath::MaterialDir();

    void FMaterial::UpdateMaterialCode_RenderThread(const std::string &InCode, FDynamicRHI* RHICmdList)
    {
        std::string PreprocessResult = FShaderParser(InCode, MATERIAL_STATIC_PARENT_DIR).Parse();
        FShaderCompiler::CompileMaterialShader(this, PreprocessResult, RHICmdList);
        bShaderCompiled = true;
    }

    UMaterial *UMaterial::GetDefaultMaterial()
    {
        return GetContentManager()->GetMaterialByPath("/Materials/DefaultMaterial.nasset");
    }

    UMaterial::UMaterial()
        : DefaultMaterialInstance(nullptr)
        , UniformBlock(std::make_shared<FDynamicUniformBuffer>())
    {
        MaterialResource = new FMaterial;
        DefaultMaterialInstance = new FMaterialRenderProxy;
    }

    UMaterial::UMaterial(const UMaterial& Material)
        : Name(Material.Name)
        , Code(Material.Code)
        , Textures(Material.Textures)
        , ShadingModel(Material.ShadingModel)
        , StencilRefValue(Material.StencilRefValue)
        , BlendState(Material.BlendState)
        , DepthStencilState(Material.DepthStencilState)
        , RasterizerState(Material.RasterizerState)
        , RuntimeUniformBlocks(Material.RuntimeUniformBlocks)
    {
        UniformBlock = std::make_shared<FDynamicUniformBuffer>(*Material.UniformBlock);
        BeginInitResource(UniformBlock.get());
        SerializationPath = "";
        MaterialResource = new FMaterial(*Material.MaterialResource);
        DefaultMaterialInstance = new FMaterialRenderProxy(*Material.DefaultMaterialInstance);
    }

    void UMaterial::UpdateCode(const std::string &InCode, bool bRecompile)
    {
        Code = InCode;
        if (MaterialResource && bRecompile)
        {
            ENQUEUE_RENDER_COMMAND(UMaterial_UpdateCode)(
                [this](FDynamicRHI* RHICmdList)
                {
                    MaterialResource->UpdateMaterialCode_RenderThread(Code, RHICmdList);
                    DefaultMaterialInstance->ShaderMap = MaterialResource->ShaderMap;
                }
            );
        }
    }

    void UMaterial::SetShaderFileVirtualPath(const std::string& VirtualPath)
    {
        ShaderVirtualPath = VirtualPath;
        std::string ShaderAbsPath = GetShaderAbsolutePathFromVirtualPath(ShaderVirtualPath);
        Code = GetAssetLoader()->SyncOpenAndReadText(ShaderAbsPath.c_str());
        UpdateCode(Code);
    }

    void UMaterial::SetTextureParameterValue(const std::string &Name, UTexture *Texture)
    {
        Textures[Name] = Texture;
        ENQUEUE_RENDER_COMMAND(Material_SetParameterValue)(
            [Texture, Name, this](FDynamicRHI*) 
            {
                GetRenderProxy()->Textures[Name] = Texture;
            });
    }

    void UMaterial::SetScalarParameterValue(const std::string &Name, float Value)
    {
        UniformBlock->SetScalarParameterValue(Name, Value);
        ENQUEUE_RENDER_COMMAND(Material_SetParameterValue)(
            [Name, Value, this](FDynamicRHI*) 
            {
                UniformBlock->UpdateUniformBuffer();
                GetRenderProxy()->UniformBuffers["MAT_UNIFORM_BLOCK"] = UniformBlock.get();
            });
    }

    void UMaterial::SetParameterValue(const std::string &Name, FUniformBuffer *UniformBuffer)
    {
        RuntimeUniformBlocks[Name] = UniformBuffer;
        ENQUEUE_RENDER_COMMAND(Material_SetParameterValue)(
            [UniformBuffer, Name, this](FDynamicRHI*) 
            {
                GetRenderProxy()->UniformBuffers[Name] = UniformBuffer;
            });
    }

    void UMaterial::SetShadingModel(EShadingModel InShadingModel)
    {
        ShadingModel = InShadingModel;
        ENQUEUE_RENDER_COMMAND(Material_SetShadingModel)(
            [InShadingModel, this](FDynamicRHI*) 
            {
                GetRenderProxy()->ShadingModel = InShadingModel;
            });
    }

    void UMaterial::SetBlendState(FBlendStateInitializer InBlendState)
    {
        BlendState = InBlendState;
        ENQUEUE_RENDER_COMMAND(Material_SetBlendState)(
            [InBlendState, this](FDynamicRHI* RHICmdList) 
            {
                GetRenderProxy()->BlendState = RHICmdList->RHICreateBlendState(InBlendState);
            });
    }

    void UMaterial::SetDepthStencilState(FDepthStencilStateInitializer InDepthStencilState)
    {
        DepthStencilState = InDepthStencilState;
        ENQUEUE_RENDER_COMMAND(Material_SetDepthStencilState)(
            [InDepthStencilState, this](FDynamicRHI* RHICmdList) 
            {
                GetRenderProxy()->DepthStencilState = RHICmdList->RHICreateDepthStencilState(InDepthStencilState);
            });
    }

    void UMaterial::SetRasterizerState(FRasterizerStateInitializer InRasterizerState)
    {
        RasterizerState = InRasterizerState;
        ENQUEUE_RENDER_COMMAND(Material_SetRasterizerState)(
            [InRasterizerState, this](FDynamicRHI* RHICmdList) 
            {
                GetRenderProxy()->RasterizerState = RHICmdList->RHICreateRasterizerState(InRasterizerState);
            });
    }

    void UMaterial::SetStencilRefValue(uint8 InStencilRefValue)
    {
        StencilRefValue = InStencilRefValue;
        ENQUEUE_RENDER_COMMAND(Material_SetStencilRefValue)(
            [InStencilRefValue, this](FDynamicRHI*) 
            {
                GetRenderProxy()->StencilRefValue = InStencilRefValue;
            });
    }

    UMaterialInstance* UMaterial::CreateMaterialInstance()
    {
        UMaterialInstance* MaterialInstance = new UMaterialInstance(this);
        return MaterialInstance;
    }

    void UMaterial::PostSerialize(FArchive& Ar)
    {
        nlohmann::json& json = Ar.Node["Content"]["UniformBlock"];
        FArchive local_Ar(json, Ar);
        UniformBlock->Serialize(local_Ar);
    }

    void UMaterial::PostDeserialize(FArchive& Ar)
    {
        if (Ar.Node["Content"].contains("UniformBlock"))
        {
            nlohmann::json& json = Ar.Node["Content"]["UniformBlock"];
            FArchive local_Ar(json, Ar);
            UniformBlock->Deserialize(local_Ar);
        }
        SetShaderFileVirtualPath(ShaderVirtualPath);
        auto BlendState = this->BlendState;
        auto RasterizerState = this->RasterizerState;
        auto DepthStencilState = this->DepthStencilState;
        auto ShaderVirtualPath = this->ShaderVirtualPath;
        auto ShadingModel = this->ShadingModel;
        auto Textures = this->Textures;
        ENQUEUE_RENDER_COMMAND(Material_SetBlendState)(
            [=](FDynamicRHI* RHICmdList) 
            {
                UniformBlock->InitResource();
                GetRenderProxy()->UniformBuffers["MAT_UNIFORM_BLOCK"] = UniformBlock.get();
                GetRenderProxy()->BlendState = RHICmdList->RHICreateBlendState(BlendState);
                GetRenderProxy()->RasterizerState = RHICmdList->RHICreateRasterizerState(RasterizerState);
                GetRenderProxy()->DepthStencilState = RHICmdList->RHICreateDepthStencilState(DepthStencilState);
                GetRenderProxy()->ShadingModel = ShadingModel;
                for (auto &[Name, Texture] : Textures)
                    GetRenderProxy()->Textures[Name] = Texture;
            });
    }

    FMaterialRenderProxy::FMaterialRenderProxy(const FMaterialRenderProxy& Other)
        : Name(Other.Name)
        , ShaderMap(Other.ShaderMap)
        , Textures(Other.Textures)
        , UniformBuffers(Other.UniformBuffers)
        , StencilRefValue(Other.StencilRefValue)
        , RasterizerState(Other.RasterizerState)
        , DepthStencilState(Other.DepthStencilState)
        , BlendState(Other.BlendState)
        , ShadingModel(Other.ShadingModel)
    {

    }

}