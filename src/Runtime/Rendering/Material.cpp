#include <glslang/Public/ResourceLimits.h>
#include <glslang/MachineIndependent/localintermediate.h>
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

    static std::string GlslangTypeToString(const glslang::TType* Type)
    {
        if (Type->getBasicType() == glslang::TBasicType::EbtFloat)
        {
            if (Type->getVectorSize() == 1)
                return "float";
            else if (Type->getVectorSize() == 2)
                return "vec2";
            else if (Type->getVectorSize() == 3)
                return "vec3";
            else if (Type->getVectorSize() == 4)
                return "vec4";
        }

        return "";
    }


    /** Begin FRenderResource Interface */
    void FMaterialUniformBuffer::InitRHI()
    {
        if (Data != nullptr)
        {
            FRenderResource::InitRHI();
            UniformBufferRHI = FDynamicRHI::GetDynamicRHI()->RHICreateUniformBuffer(Size, Usage, Data.get());
        }
    }
    void FMaterialUniformBuffer::ReleaseRHI()
    {
        UniformBufferRHI = nullptr;
        FRenderResource::ReleaseRHI();
    }
    /** End FRenderResource Interface */

    void FMaterialUniformBuffer::UpdateUniformBuffer()
    {
        if (UniformBufferRHI != nullptr && Data != nullptr)
            FDynamicRHI::GetDynamicRHI()->RHIUpdateUniformBuffer(UniformBufferRHI, Data.get());
    }

    void FMaterialUniformBuffer::Serialize(FArchive& Ar)
    {
        nlohmann::json& json = Ar.Node;
        for (auto& [Name, Field] : Fields)
        {
            nlohmann::json value_json; 
            if (Field.Type == "float")
            {
                value_json = _GetField<float>(Field.Offset);
            }
            else if (Field.Type == "vec2")
            {
                vec2 value = _GetField<vec2>(Field.Offset);
                for (int i = 0; i < value.length(); i++)
                    value_json.push_back(value[i]);
            }
            else if (Field.Type == "vec3")
            {
                vec3 value = _GetField<vec3>(Field.Offset);
                for (int i = 0; i < value.length(); i++)
                    value_json.push_back(value[i]);
            }
            else if (Field.Type == "vec4")
            {
                vec4 value = _GetField<vec4>(Field.Offset);
                for (int i = 0; i < value.length(); i++)
                    value_json.push_back(value[i]);
            }
            json["Fields"].push_back(std::map<std::string, nlohmann::json>{
                {"Name", Field.Name},
                {"Offset", Field.Offset},
                {"Type", Field.Type},
                {"Value", value_json}
            });
        }
        
    }

    void FMaterialUniformBuffer::Deserialize(FArchive& Ar)
    {
        nlohmann::json& json = Ar.Node;
        if (json.contains("Fields"))
            Ncheck(json["Fields"].is_array());
        for (auto& field : json["Fields"])
        {
            Field Field;
            Field.Name = field["Name"];
            Field.Type = field["Type"];
            Ncheck(Field.Type == this->Fields[Field.Name].Type);
            // DON'T SET THE METADATA HERE
            // Because the metadata comes from the shader reflection, NOT the serialized data.
            // We only need to set the value here.
            // Fields[Field.Name] = Field;
            if (Field.Type == "float")
            {
                float value;
                value = field["Value"].get<float>();
                SetScalarParameterValue(Field.Name, value);
            }
            else if (Field.Type == "vec2")
            {
                vec2 value;
                value[0] = field["Value"][0].get<float>();
                value[1] = field["Value"][1].get<float>();
                SetVectorParameterValue(Field.Name, value);
            }
            else if (Field.Type == "vec3")
            {
                vec3 value;
                value[0] = field["Value"][0].get<float>();
                value[1] = field["Value"][1].get<float>();
                value[2] = field["Value"][2].get<float>();
                SetVectorParameterValue(Field.Name, value);
            }
            else if (Field.Type == "vec4")
            {
                vec4 value;
                value[0] = field["Value"][0].get<float>();
                value[1] = field["Value"][1].get<float>();
                value[2] = field["Value"][2].get<float>();
                value[3] = field["Value"][3].get<float>();
                SetVectorParameterValue(Field.Name, value);
            }
        }
    }

    UMaterial *UMaterial::GetDefaultMaterial()
    {
        return GetContentManager()->GetMaterialByPath("/Materials/DefaultMaterial.nasset");
    }

    UMaterial::UMaterial()
    {
        MaterialShaderMap = std::make_shared<FMaterialShaderMap>();
        MaterialRenderProxy = new FMaterialRenderProxy(this);
        MaterialUniformBlock = new FMaterialUniformBuffer();
        
        MaterialRenderProxy->UniformBuffer = MaterialUniformBlock;
        MaterialRenderProxy->ShaderMap = MaterialShaderMap.get();
    }

    UMaterial::~UMaterial()
    {
        ENQUEUE_RENDER_COMMAND(Material_ReleaseResources)(
            [
                ToDelete_proxy = MaterialRenderProxy,
                ToDelete_uniform = MaterialUniformBlock] (FDynamicRHI*)
            {
                delete ToDelete_proxy;
                delete ToDelete_uniform;
            });
        MaterialRenderProxy = nullptr;
        MaterialUniformBlock = nullptr;
    }

    static std::unique_ptr<glslang::TShader> CompileShader(const std::string& Code, EPipelineStage PipelineStage)
    {
        const char *code_c_str = Code.c_str();
        glslang::EShClient client = glslang::EShClientOpenGL;
        glslang::EShTargetClientVersion version = glslang::EShTargetOpenGL_450;
        if (FDynamicRHI::StaticGetCurrentGraphicsAPI() == EGraphicsAPI::Vulkan)
        {
            client = glslang::EShClientVulkan;
            version = glslang::EShTargetVulkan_1_3;
        }
        std::unique_ptr<glslang::TShader> ShaderGlsl;
        switch (PipelineStage) 
        {
            case EPipelineStage::PS_Vertex:
                ShaderGlsl = std::make_unique<glslang::TShader>(EShLanguage::EShLangVertex);
                ShaderGlsl->setEnvInput(glslang::EShSourceGlsl , EShLanguage::EShLangVertex,  client, 0);
                break;
            case EPipelineStage::PS_Pixel:
                ShaderGlsl = std::make_unique<glslang::TShader>(EShLanguage::EShLangFragment);
                ShaderGlsl->setEnvInput(glslang::EShSourceGlsl , EShLanguage::EShLangFragment,  client, 0);
                break;
            case EPipelineStage::PS_Compute:
                ShaderGlsl = std::make_unique<glslang::TShader>(EShLanguage::EShLangCompute);
                ShaderGlsl->setEnvInput(glslang::EShSourceGlsl , EShLanguage::EShLangCompute,  client, 0);
                break;
        }
        ShaderGlsl->setEnvClient(client, version);
        ShaderGlsl->setEnvTarget(glslang::EShTargetNone, glslang::EShTargetLanguageVersion(0));
        ShaderGlsl->setStrings(&code_c_str, 1);
        std::string preprocess;
        glslang::TShader::ForbidIncluder includer;
        bool res = ShaderGlsl->preprocess(GetResources(), 460, EProfile::ECoreProfile, false, false, EShMsgDefault, &preprocess, includer);
        res &= ShaderGlsl->parse(GetResources(), 460, false, EShMsgDefault);
        if (!res)
        {
            std::string info = ShaderGlsl->getInfoLog();
            std::string debuginfo = ShaderGlsl->getInfoDebugLog();
            NILOU_LOG(Error, "Shader parse error: {}\n{}", 
                info,
                debuginfo);
            NILOU_LOG(Error, "Shader code: \n{}", Code);
            ShaderGlsl = nullptr;
        }
        return ShaderGlsl;
    }

    static std::unique_ptr<glslang::TProgram> BuildProgram(const std::string& VS, const std::string& FS)
    {
        auto VertexShader = CompileShader(VS, PS_Vertex);
        auto PixelShader = CompileShader(FS, PS_Pixel);
        std::unique_ptr<glslang::TProgram> ProgramGlsl = std::make_unique<glslang::TProgram>();
        ProgramGlsl->addShader(VertexShader.get());
        ProgramGlsl->addShader(PixelShader.get());
        ProgramGlsl->link(EShMsgDefault);
        ProgramGlsl->buildReflection();
        return ProgramGlsl;
    }

    void UMaterial::UpdateCode(const std::string &InCode)
    {
        Code = InCode;
        Ncheck(MaterialUniformBlock != nullptr);

        // Build reflection, then we can set uniforms by name.
        // This is only used for reflection, NOT the actual shader compilation.
        std::string PreprocessResult = FShaderParser(InCode, MATERIAL_STATIC_PARENT_DIR).Parse();
        std::string VertexShaderCode = "#version 460\nvoid main() { }";
        std::string PixelShaderCode = 
            "#version 460\n"
            "layout (location = 0) out vec4 FragColor;\n" +  
            PreprocessResult + 
            R"(
                void main()
                {
                    VS_Out vs_out;
                    FragColor = MaterialGetBaseColor(vs_out) + 
                                vec4(MaterialGetEmissive(vs_out), 0) + 
                                vec4(MaterialGetWorldSpaceNormal(vs_out), 0) + 
                                vec4(MaterialGetRoughness(vs_out)) + 
                                vec4(MaterialGetMetallic(vs_out)) + 
                                vec4(MaterialGetWorldSpaceOffset(vs_out), 0);
                }
            )";
        auto ProgramGlsl = BuildProgram(VertexShaderCode, PixelShaderCode);

        int NumUniformBlock = ProgramGlsl->getNumUniformBlocks();
        std::map<std::string, FMaterialUniformBuffer::Field>& Fields = MaterialUniformBlock->Fields;
        Fields.clear();
        for (int i = 0; i < NumUniformBlock; i++)
        {
            const glslang::TObjectReflection &refl = ProgramGlsl->getUniformBlock(i);
            if (refl.name == "MAT_UNIFORM_BLOCK")
            {
                MaterialUniformBlock->Size = refl.size;
                MaterialUniformBlock->Data = std::make_unique<uint8[]>(refl.size);
                const glslang::TTypeList* struct_refl = refl.getType()->getStruct();
                if (struct_refl)
                {
                    for (auto& type : *struct_refl)
                    {
                        std::string name = type.type->getFieldName().c_str();
                        Fields.insert({name, FMaterialUniformBuffer::Field{name}});
                    }
                }
                break;
            }
        }

        int uniform_num = ProgramGlsl->getNumUniformVariables();
        for (int i = 0; i < uniform_num; i++)
        {
            std::string name = ProgramGlsl->getUniformName(i);
            const glslang::TObjectReflection& refl = ProgramGlsl->getUniform(i);
            if (Fields.contains(name))
            {
                FMaterialUniformBuffer::Field& Field = Fields[name];
                Field.Name = name;
                Field.Offset = refl.offset;
                Field.Type = GlslangTypeToString(refl.getType());
                if (Field.Type == "")
                    NILOU_LOG(Error, "\"{}\" in material \"{}\" is invalid because its type is not supported", name, this->Name);
            }
        }

        ENQUEUE_RENDER_COMMAND(UMaterial_UpdateCode)(
            [this, PreprocessResult](FDynamicRHI* RHICmdList)
            {
                MaterialUniformBlock->InitResource();
                FShaderCompiler::CompileMaterialShader(MaterialShaderMap.get(), PreprocessResult, RHICmdList);
            }
        );
    }

    void UMaterial::SetShaderFileVirtualPath(const std::string& VirtualPath)
    {
        ShaderVirtualPath = VirtualPath;
        std::string ShaderAbsPath = FPath::VirtualPathToAbsPath(ShaderVirtualPath).generic_string();
        Code = GetAssetLoader()->SyncOpenAndReadText(ShaderAbsPath.c_str());
        UpdateCode(Code);
    }

    void UMaterial::SetTextureParameterValue(const std::string &Name, UTexture *Texture)
    {
        Textures[Name] = Texture;
        ENQUEUE_RENDER_COMMAND(Material_SetTextureParameterValue)(
            [Texture, Name, this](FDynamicRHI*) 
            {
                GetRenderProxy()->Textures[Name] = Texture->GetResource();
            });
    }

    void UMaterial::SetScalarParameterValue(const std::string &Name, float Value)
    {
        MaterialUniformBlock->SetScalarParameterValue(Name, Value);
        ENQUEUE_RENDER_COMMAND(Material_SetScalarParameterValue)(
            [Name, Value, this](FDynamicRHI*) 
            {
                MaterialUniformBlock->UpdateUniformBuffer();
            });
    }

    void UMaterial::SetVectorParameterValue(const std::string &Name, const vec2 &Value)
    {
        MaterialUniformBlock->SetVectorParameterValue(Name, Value);
        ENQUEUE_RENDER_COMMAND(Material_SetVectorParameterValue)(
            [Name, Value, this](FDynamicRHI*) 
            {
                MaterialUniformBlock->UpdateUniformBuffer();
            });
    }

    void UMaterial::SetVectorParameterValue(const std::string &Name, const vec3 &Value)
    {
        MaterialUniformBlock->SetVectorParameterValue(Name, Value);
        ENQUEUE_RENDER_COMMAND(Material_SetVectorParameterValue)(
            [Name, Value, this](FDynamicRHI*) 
            {
                MaterialUniformBlock->UpdateUniformBuffer();
            });
    }

    void UMaterial::SetVectorParameterValue(const std::string &Name, const vec4 &Value)
    {
        MaterialUniformBlock->SetVectorParameterValue(Name, Value);
        ENQUEUE_RENDER_COMMAND(Material_SetVectorParameterValue)(
            [Name, Value, this](FDynamicRHI*) 
            {
                MaterialUniformBlock->UpdateUniformBuffer();
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
        std::string parent_path = std::filesystem::path(GetVirtualPath()).parent_path().generic_string();
        UMaterialInstance* MaterialInstance = GetContentManager()->CreateAsset<UMaterialInstance>(Name+"_Instance", parent_path);

        // Copy some properties
        MaterialInstance->Code = this->Code;
        MaterialInstance->Textures = this->Textures;
        MaterialInstance->ShadingModel = this->ShadingModel;
        MaterialInstance->StencilRefValue = this->StencilRefValue;
        MaterialInstance->BlendState = this->BlendState;
        MaterialInstance->DepthStencilState = this->DepthStencilState;
        MaterialInstance->RasterizerState = this->RasterizerState;
        MaterialInstance->MaterialShaderMap = this->MaterialShaderMap;

        // Copy the uniform buffer
        auto NewUniformBuffer = MaterialInstance->MaterialUniformBlock;
        NewUniformBuffer->Size = this->MaterialUniformBlock->Size;
        NewUniformBuffer->Fields = this->MaterialUniformBlock->Fields;
        NewUniformBuffer->Data = std::make_unique<uint8[]>(MaterialUniformBlock->Size);
        memcpy(NewUniformBuffer->Data.get(), this->MaterialUniformBlock->Data.get(), this->MaterialUniformBlock->Size);

        // Copy the render proxy
        auto NewRenderProxy = MaterialInstance->MaterialRenderProxy;
        NewRenderProxy->ShaderMap = this->MaterialRenderProxy->ShaderMap;
        NewRenderProxy->Textures = this->MaterialRenderProxy->Textures;
        NewRenderProxy->UniformBuffer = this->MaterialRenderProxy->UniformBuffer;
        NewRenderProxy->StencilRefValue = this->MaterialRenderProxy->StencilRefValue;
        NewRenderProxy->RasterizerState = this->MaterialRenderProxy->RasterizerState;
        NewRenderProxy->DepthStencilState = this->MaterialRenderProxy->DepthStencilState;
        NewRenderProxy->BlendState = this->MaterialRenderProxy->BlendState;
        NewRenderProxy->ShadingModel = this->MaterialRenderProxy->ShadingModel;

        ENQUEUE_RENDER_COMMAND(UMaterial_CreateMaterialInstance)(
            [NewUniformBuffer, NewRenderProxy](FDynamicRHI* RHICmdList) 
            {
                NewUniformBuffer->InitResource();
                NewRenderProxy->UniformBuffer = NewUniformBuffer;
            });

        return MaterialInstance;
    }

    void UMaterial::PostSerialize(FArchive& Ar)
    {
        nlohmann::json& json = Ar.Node["Content"]["MaterialUniformBlock"];
        FArchive local_Ar(json, Ar);
        MaterialUniformBlock->Serialize(local_Ar);
    }

    void UMaterial::PostDeserialize(FArchive& Ar)
    {
        SetShaderFileVirtualPath(ShaderVirtualPath);
        if (Ar.Node["Content"].contains("MaterialUniformBlock"))
        {
            // The deserialization of MaterialUniformBlock MUST be done after the shader compilation.
            nlohmann::json& json = Ar.Node["Content"]["MaterialUniformBlock"];
            FArchive local_Ar(json, Ar);
            MaterialUniformBlock->Deserialize(local_Ar);
        }
        auto BlendState = this->BlendState;
        auto RasterizerState = this->RasterizerState;
        auto DepthStencilState = this->DepthStencilState;
        auto ShaderVirtualPath = this->ShaderVirtualPath;
        auto ShadingModel = this->ShadingModel;
        auto Textures = this->Textures;
        ENQUEUE_RENDER_COMMAND(Material_PostDeserialize)(
            [=](FDynamicRHI* RHICmdList) 
            {
                MaterialUniformBlock->UpdateUniformBuffer();
                GetRenderProxy()->UniformBuffer = MaterialUniformBlock;
                GetRenderProxy()->BlendState = RHICmdList->RHICreateBlendState(BlendState);
                GetRenderProxy()->RasterizerState = RHICmdList->RHICreateRasterizerState(RasterizerState);
                GetRenderProxy()->DepthStencilState = RHICmdList->RHICreateDepthStencilState(DepthStencilState);
                GetRenderProxy()->ShadingModel = ShadingModel;
                for (auto &[Name, Texture] : Textures)
                {
                    if (Texture->GetResource() == nullptr)
                        Texture->UpdateResource();
                    GetRenderProxy()->Textures[Name] = Texture->GetResource();
                }
            });
    }

    FMaterialRenderProxy::FMaterialRenderProxy(UMaterial* InMaterial)
        : Material(InMaterial)
    {
        ENQUEUE_RENDER_COMMAND(FMaterialRenderProxy_Constructor)(
            [InBlendState=InMaterial->BlendState,
             InRasterizerState=InMaterial->RasterizerState,
             InDepthStencilState=InMaterial->DepthStencilState,
             InShadingModel=InMaterial->ShadingModel,
             this](FDynamicRHI* RHICmdList) 
            {
                BlendState = RHICmdList->RHICreateBlendState(InBlendState);
                RasterizerState = RHICmdList->RHICreateRasterizerState(InRasterizerState);
                DepthStencilState = RHICmdList->RHICreateDepthStencilState(InDepthStencilState);
                ShadingModel = InShadingModel;
            });
    }

}