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
        , UniformBuffers(Material.UniformBuffers)
        , Uniforms(Material.Uniforms)
    {
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

    void UMaterial::SetParameterValue(const std::string &Name, FUniformBuffer *UniformBuffer)
    {
        UniformBuffers[Name] = UniformBuffer;
        ENQUEUE_RENDER_COMMAND(Material_SetParameterValue)(
            [UniformBuffer, Name, this](FDynamicRHI*) 
            {
                GetRenderProxy()->UniformBuffers[Name] = UniformBuffer;
            });
    }

    void UMaterial::SetScalarParameterValue(const std::string &Name, FUniformValue Uniform)
    {
        Uniforms[Name] = Uniform;
        ENQUEUE_RENDER_COMMAND(Material_SetParameterValue)(
            [Uniform, Name, this](FDynamicRHI*) 
            {
                GetRenderProxy()->Uniforms[Name] = Uniform;
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
            [InBlendState, this](FDynamicRHI*) 
            {
                GetRenderProxy()->BlendState = InBlendState;
            });
    }

    void UMaterial::SetDepthStencilState(FDepthStencilStateInitializer InDepthStencilState)
    {
        DepthStencilState = InDepthStencilState;
        ENQUEUE_RENDER_COMMAND(Material_SetDepthStencilState)(
            [InDepthStencilState, this](FDynamicRHI*) 
            {
                GetRenderProxy()->DepthStencilState = InDepthStencilState;
            });
    }

    void UMaterial::SetRasterizerState(FRasterizerStateInitializer InRasterizerState)
    {
        RasterizerState = InRasterizerState;
        ENQUEUE_RENDER_COMMAND(Material_SetRasterizerState)(
            [InRasterizerState, this](FDynamicRHI*) 
            {
                GetRenderProxy()->RasterizerState = InRasterizerState;
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

    // void UMaterial::Serialize(FArchive &Ar)
    // {
    //     nlohmann::json &json = Ar.json;
    //     json["ClassName"] = "UMaterial";
    //     nlohmann::json &content = json["Content"];
    //     content["Name"] = Name;
    //     content["ShaderVirtualPath"] = ShaderVirtualPath.generic_string();
    //     content["ShadingModel"] = magic_enum::enum_name(ShadingModel);
    //     content["StencilRefValue"] = StencilRefValue;
    //     TStaticSerializer<FBlendStateInitializer>::Serialize(BlendState, content["BlendState"], Ar.OutBuffers);
    //     TStaticSerializer<FRasterizerStateInitializer>::Serialize(RasterizerState, content["RasterizerState"], Ar.OutBuffers);
    //     TStaticSerializer<FDepthStencilStateInitializer>::Serialize(DepthStencilState, content["DepthStencilState"], Ar.OutBuffers);
    //     nlohmann::json &textures = content["Textures"];
    //     for (auto &[Name, Texture] : Textures)
    //     {
    //         if (Texture)
    //         {
    //             textures[Name] = Texture->SerializationPath;
    //         }
    //     }
    // }

    // void UMaterial::MDeserialize(FArchive &Ar)
    // {
    //     nlohmann::json &json = Ar.Node;
    //     nlohmann::json &content = json["Content"];
    //     Name = content["Name"];
    //     MaterialResource->Name = Name;
    //     DefaultMaterialInstance->Name = Name;
    //     if (content.contains("ShadingModel"))
    //     {
    //         SetShadingModel(magic_enum::enum_cast<EShadingModel>(content["ShadingModel"].get<std::string>()).value());
    //     }
    //     SetStencilRefValue(content["StencilRefValue"]);
    //     FBlendStateInitializer BlendState;
    //     FRasterizerStateInitializer RasterizerState;
    //     FDepthStencilStateInitializer DepthStencilState;
    //     {
    //         FArchive local_Ar(content["BlendState"]["Content"], Ar);
    //         TStaticSerializer<FBlendStateInitializer>::Deserialize(BlendState, local_Ar);
    //     }
    //     {
    //         FArchive local_Ar(content["RasterizerState"]["Content"], Ar);
    //         TStaticSerializer<FRasterizerStateInitializer>::Deserialize(RasterizerState, local_Ar);
    //     }
    //     {
    //         FArchive local_Ar(content["DepthStencilState"]["Content"], Ar);
    //         TStaticSerializer<FDepthStencilStateInitializer>::Deserialize(DepthStencilState, local_Ar);
    //     }
    //     SetBlendState(BlendState);
    //     SetRasterizerState(RasterizerState);
    //     SetDepthStencilState(DepthStencilState);
        
    //     std::string ShaderVirtualPath = content["ShaderVirtualPath"];
    //     SetShaderFileVirtualPath(ShaderVirtualPath);
        
    //     nlohmann::json &textures = content["Textures"];
    //     for (auto &[sampler_name, texture] : textures.items())
    //     {
    //         fs::path texture_path = texture.get<std::string>();
    //         UTexture *Texture = GetContentManager()->GetTextureByPath(texture_path);
    //         SetTextureParameterValue(sampler_name, Texture);
    //     }
    // }

    UMaterialInstance* UMaterial::CreateMaterialInstance()
    {
        UMaterialInstance* MaterialInstance = new UMaterialInstance(this);
        return MaterialInstance;
    }

    void UMaterial::PostDeserialize()
    {
        SetShaderFileVirtualPath(ShaderVirtualPath);
        auto BlendState = this->BlendState;
        auto RasterizerState = this->RasterizerState;
        auto DepthStencilState = this->DepthStencilState;
        auto ShaderVirtualPath = this->ShaderVirtualPath;
        auto Textures = this->Textures;
        ENQUEUE_RENDER_COMMAND(Material_SetBlendState)(
            [=](FDynamicRHI*) 
            {
                GetRenderProxy()->BlendState = BlendState;
                GetRenderProxy()->RasterizerState = RasterizerState;
                GetRenderProxy()->DepthStencilState = DepthStencilState;
                for (auto &[Name, Texture] : Textures)
                    GetRenderProxy()->Textures[Name] = Texture;
            });
    }

    // void UMaterialInstance::Serialize(FArchive &Ar)
    // {
    //     nlohmann::json &json = Ar.json;
    //     UMaterial::Serialize(Ar);
    //     json["ClassName"] = "UMaterialInstance";
    // }

    FMaterialRenderProxy::FMaterialRenderProxy(const FMaterialRenderProxy& Other)
        : Name(Other.Name)
        , ShaderMap(Other.ShaderMap)
        , Textures(Other.Textures)
        , UniformBuffers(Other.UniformBuffers)
        , Uniforms(Other.Uniforms)
        , StencilRefValue(Other.StencilRefValue)
        , RasterizerState(Other.RasterizerState)
        , DepthStencilState(Other.DepthStencilState)
        , BlendState(Other.BlendState)
        , ShadingModel(Other.ShadingModel)
    {

    }

}