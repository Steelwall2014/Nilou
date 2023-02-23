#include <fstream>

#include "World.h"
#include "Common/BaseApplication.h"
#include "Common/Transform.h"

#include "Common/StaticMeshResources.h"
#include "Common/AssetLoader.h"
#include "Common/Components/MeshComponent.h"
#include "Common/Components/ArrowComponent.h"
#include "Common/PrimitiveUtils.h"
#include "Common/ContentManager.h"

#include "Common/Actor/StaticMeshActor.h"
#include "Common/Actor/CameraActor.h"
#include "Common/Actor/LightActor.h"
#include "Common/Actor/ArrowActor.h"
#include "Common/Actor/SphereActor.h"
#include "Common/Actor/SkyAtmosphereActor.h"
#include "Common/Actor/GeoreferenceActor.h"
#include "Common/Actor/Cesium3DTilesetActor.h"
#include "Common/Actor/LineBatchActor.h"

#include "Georeference.h"

#include <ogrsf_frmts.h>

namespace nilou {
    // static void CreateMaterials()
    // {
    //     std::unique_ptr<UMaterial> DefaultMaterial = std::make_unique<UMaterial>("DefaultMaterial");
    //     // GetContentManager()->AddGlobalMaterial(DefaultMaterial->Name, DefaultMaterial);
    //     DefaultMaterial->UpdateCode(
    //     R"(
    //         #include "../include/BasePassCommon.glsl"
    //         vec4 MaterialGetBaseColor(VS_Out vs_out)
    //         {
    //             return vec4(0, 0, 0, 1);
    //         }
    //         vec3 MaterialGetEmissive(VS_Out vs_out)
    //         {
    //             return vec3(0);
    //         }
    //         vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
    //         {
    //             return normalize(vs_out.TBN * vec3(0, 0, 1));
    //         }
    //         float MaterialGetRoughness(VS_Out vs_out)
    //         {
    //             return 0.5;
    //         }
    //         float MaterialGetMetallic(VS_Out vs_out)
    //         {
    //             return 0.5;
    //         }
    //         vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
    //         {
    //             return vec3(0);
    //         }
    //     )", false);
    //     GetContentManager()->CreateFile("/DefaultMaterial.json", std::move(DefaultMaterial));

    //     std::unique_ptr<UMaterial> ColoredMaterial = std::make_unique<UMaterial>("ColoredMaterial");
    //     // GetContentManager()->AddGlobalMaterial(ColoredMaterial->Name, ColoredMaterial);
    //     ColoredMaterial->GetResource()->RasterizerState.CullMode = CM_None;
    //     ColoredMaterial->UpdateCode(
    //     R"(
    //         #include "../include/BasePassCommon.glsl"
    //         vec4 MaterialGetBaseColor(VS_Out vs_out)
    //         {
    //             return vs_out.Color;
    //         }
    //         vec3 MaterialGetEmissive(VS_Out vs_out)
    //         {
    //             return vec3(0);
    //         }
    //         vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
    //         {
    //             return normalize(vs_out.TBN * vec3(0, 0, 1));
    //         }
    //         float MaterialGetRoughness(VS_Out vs_out)
    //         {
    //             return 0.5;
    //         }
    //         float MaterialGetMetallic(VS_Out vs_out)
    //         {
    //             return 0.5;
    //         }
    //         vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
    //         {
    //             return vec3(0);
    //         }
    //     )", false);
    //     GetContentManager()->CreateFile("/ColoredMaterial.json", std::move(ColoredMaterial));

    //     std::unique_ptr<UMaterial> SkyAtmosphereMaterial = std::make_unique<UMaterial>("SkyAtmosphereMaterial");
    //     // GetContentManager()->AddGlobalMaterial(SkyAtmosphereMaterial->Name, SkyAtmosphereMaterial);
    //     FMaterial *SkyAtmosphereMaterialResource = SkyAtmosphereMaterial->GetResource();
    //     SkyAtmosphereMaterialResource->RasterizerState.CullMode = CM_None;
    //     SkyAtmosphereMaterialResource->DepthStencilState.bEnableFrontFaceStencil = true;
    //     SkyAtmosphereMaterialResource->DepthStencilState.FrontFaceStencilTest = ECompareFunction::CF_Always;
    //     SkyAtmosphereMaterialResource->DepthStencilState.FrontFacePassStencilOp = EStencilOp::SO_Replace;
    //     SkyAtmosphereMaterialResource->DepthStencilState.bEnableBackFaceStencil = true;
    //     SkyAtmosphereMaterialResource->DepthStencilState.BackFaceStencilTest = ECompareFunction::CF_Always;
    //     SkyAtmosphereMaterialResource->DepthStencilState.BackFacePassStencilOp = EStencilOp::SO_Replace;
    //     SkyAtmosphereMaterialResource->StencilRefValue = 255;
    //     SkyAtmosphereMaterial->UpdateCode(
    //     R"(
    //         #include "../include/BasePassCommon.glsl"
    //         vec4 MaterialGetBaseColor(VS_Out vs_out)
    //         {
    //             return vec4(0);
    //         }
    //         vec3 MaterialGetEmissive(VS_Out vs_out)
    //         {
    //             return vec3(0);
    //         }
    //         vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
    //         {
    //             return normalize(vs_out.TBN * vec3(0, 0, 1));
    //         }
    //         float MaterialGetRoughness(VS_Out vs_out)
    //         {
    //             return 0.5;
    //         }
    //         float MaterialGetMetallic(VS_Out vs_out)
    //         {
    //             return 0.5;
    //         }
    //         vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
    //         {
    //             return vec3(0);
    //         }
    //     )", false);
    //     GetContentManager()->CreateFile("/SkyAtmosphereMaterial.json", std::move(SkyAtmosphereMaterial));
        
    //     std::unique_ptr<UMaterial> WireframeMaterial = std::make_unique<UMaterial>("WireframeMaterial");
    //     // GetContentManager()->AddGlobalMaterial(WireframeMaterial->Name, WireframeMaterial);
    //     WireframeMaterial->UpdateCode(
    //     R"(
    //         #include "../include/BasePassCommon.glsl"
    //         vec4 MaterialGetBaseColor(VS_Out vs_out)
    //         {
    //             return vec4(0, 0, 0, 1);
    //         }
    //         vec3 MaterialGetEmissive(VS_Out vs_out)
    //         {
    //             return vec3(0);
    //         }
    //         vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
    //         {
    //             return normalize(vs_out.TBN * vec3(0, 0, 1));
    //         }
    //         float MaterialGetRoughness(VS_Out vs_out)
    //         {
    //             return 0.5;
    //         }
    //         float MaterialGetMetallic(VS_Out vs_out)
    //         {
    //             return 0.5;
    //         }
    //         vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
    //         {
    //             return vec3(0);
    //         }
    //     )", false);
    //     WireframeMaterial->GetResource()->RasterizerState.FillMode = ERasterizerFillMode::FM_Wireframe;
    //     GetContentManager()->CreateFile("/WireframeMaterial.json", std::move(WireframeMaterial));
        
    //     std::unique_ptr<UMaterial> GLTFMaterial = std::make_unique<UMaterial>("GLTFMaterial");
    //     // GetContentManager()->AddGlobalMaterial(GLTFMaterial->Name, GLTFMaterial);
    //     GLTFMaterial->UpdateCode(
    //     R"(
    //         #include "../include/BasePassCommon.glsl"

    //         uniform sampler2D baseColorTexture;
    //         uniform sampler2D metallicRoughnessTexture;
    //         uniform sampler2D emissiveTexture;
    //         // uniform sampler2D normalTexture;

    //         layout (std140) uniform FGLTFMaterialBlock {
    //             vec4 baseColorFactor;
    //             vec3 emissiveFactor;
    //             float metallicFactor;
    //             float roughnessFactor;
    //         };

    //         vec4 MaterialGetBaseColor(VS_Out vs_out)
    //         {
    //             return texture(baseColorTexture, vs_out.TexCoords) * baseColorFactor;
    //         }
    //         vec3 MaterialGetEmissive(VS_Out vs_out)
    //         {
    //             return texture(emissiveTexture, vs_out.TexCoords).rgb * emissiveFactor;
    //         }
    //         vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
    //         {
    //             // vec3 tangent_normal = texture(normalTexture, vs_out.TexCoords).rgb;
    //             vec3 tangent_normal = vec3(0.5, 0.5, 1.0);
    //             tangent_normal = normalize(tangent_normal * 2.0f - 1.0f);
    //             return normalize(vs_out.TBN * tangent_normal);
    //         }
    //         float MaterialGetRoughness(VS_Out vs_out)
    //         {
    //             return texture(metallicRoughnessTexture, vs_out.TexCoords).g;
    //         }
    //         float MaterialGetMetallic(VS_Out vs_out)
    //         {
    //             return texture(metallicRoughnessTexture, vs_out.TexCoords).b;
    //         }
    //         vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
    //         {
    //             return vec3(0);
    //         }
    //     )", false);
    //     RHITextureParams texParams;
    //     texParams.Mag_Filter = ETextureFilters::TF_Nearest;
    //     texParams.Min_Filter = ETextureFilters::TF_Nearest;
    //     texParams.Wrap_S = ETextureWrapModes::TW_Clamp;
    //     texParams.Wrap_T = ETextureWrapModes::TW_Clamp;
    //     std::shared_ptr<FImage> NoColorImg = std::make_shared<FImage>();
    //     NoColorImg->Width = 1; NoColorImg->Height = 1; NoColorImg->Channel = 4; NoColorImg->data_size = 4;
    //     NoColorImg->PixelFormat = EPixelFormat::PF_R8G8B8A8; NoColorImg->data = new uint8[4];
    //     NoColorImg->data[0] = 255; NoColorImg->data[1] = 255; NoColorImg->data[2] = 255; NoColorImg->data[3] = 255;
    //     std::unique_ptr<UTexture> NoColorTexture = std::make_unique<UTexture>("NoColorTexture", 1, NoColorImg);
    //     NoColorTexture->GetResource()->SetSamplerParams(texParams);
    //     GetContentManager()->CreateFile("/NoColorTexture.json", std::move(NoColorTexture));

    //     std::shared_ptr<FImage> NoMetallicRoughnessImg = std::make_shared<FImage>();
    //     NoMetallicRoughnessImg->Width = 1; NoMetallicRoughnessImg->Height = 1; NoMetallicRoughnessImg->Channel = 4; NoMetallicRoughnessImg->data_size = 4;
    //     NoMetallicRoughnessImg->PixelFormat = EPixelFormat::PF_R8G8B8A8; NoMetallicRoughnessImg->data = new uint8[4];
    //     NoMetallicRoughnessImg->data[0] = 0; NoMetallicRoughnessImg->data[1] = 255; NoMetallicRoughnessImg->data[2] = 255; NoMetallicRoughnessImg->data[3] = 255;
    //     std::unique_ptr<UTexture> NoMetallicRoughnessTexture = std::make_unique<UTexture>("NoMetallicRoughnessTexture", 1, NoMetallicRoughnessImg);
    //     NoMetallicRoughnessTexture->GetResource()->SetSamplerParams(texParams);
    //     GetContentManager()->CreateFile("/NoMetallicRoughnessTexture.json", std::move(NoMetallicRoughnessTexture));

    //     std::shared_ptr<FImage> NoEmissiveImg = std::make_shared<FImage>();
    //     NoEmissiveImg->Width = 1; NoEmissiveImg->Height = 1; NoEmissiveImg->Channel = 4; NoEmissiveImg->data_size = 4;
    //     NoEmissiveImg->PixelFormat = EPixelFormat::PF_R8G8B8A8; NoEmissiveImg->data = new uint8[4];
    //     NoEmissiveImg->data[0] = 0; NoEmissiveImg->data[1] = 0; NoEmissiveImg->data[2] = 0; NoEmissiveImg->data[3] = 255;
    //     std::unique_ptr<UTexture> NoEmissiveTexture = std::make_unique<UTexture>("NoEmissiveTexture", 1, NoEmissiveImg);
    //     NoEmissiveTexture->GetResource()->SetSamplerParams(texParams);
    //     GetContentManager()->CreateFile("/NoEmissiveTexture.json", std::move(NoEmissiveTexture));

    //     std::shared_ptr<FImage> NoNormalImg = std::make_shared<FImage>();
    //     NoNormalImg->Width = 1; NoNormalImg->Height = 1; NoNormalImg->Channel = 4; NoNormalImg->data_size = 4;
    //     NoNormalImg->PixelFormat = EPixelFormat::PF_R8G8B8A8; NoNormalImg->data = new uint8[4];
    //     NoNormalImg->data[0] = 127; NoNormalImg->data[1] = 127; NoNormalImg->data[2] = 255; NoNormalImg->data[3] = 255;
    //     std::unique_ptr<UTexture> NoNormalTexture = std::make_unique<UTexture>("NoNormalTexture", 1, NoNormalImg);
    //     NoNormalTexture->GetResource()->SetSamplerParams(texParams);
    //     GetContentManager()->CreateFile("/NoNormalTexture.json", std::move(NoNormalTexture));

    //     GLTFMaterial->GetResource()->SetParameterValue("baseColorTexture", NoColorTexture.get());
    //     GLTFMaterial->GetResource()->SetParameterValue("metallicRoughnessTexture", NoMetallicRoughnessTexture.get());
    //     GLTFMaterial->GetResource()->SetParameterValue("emissiveTexture", NoEmissiveTexture.get());
    //     GLTFMaterial->GetResource()->SetParameterValue("normalTexture", NoNormalTexture.get());
    //     GetContentManager()->CreateFile("/GLTFMaterial.json", std::move(GLTFMaterial));
    //     GetContentManager()->Flush();
    // }
    UWorld::UWorld()
    {
        
    }

    void UWorld::InitWorld()
    {

    }

    void UWorld::InitializeActorsForPlay() 
    { 
        // TODO: 这里暂时是空，以后反序列化出来的Actor会在这里注册和初始化
    }

    void UWorld::BeginPlay()
    {
        bHasBegunPlay = true;

        UStaticMesh *Mesh = dynamic_cast<UStaticMesh*>(GetContentManager()->GetContentByPath("Testgltf/WaterBottle.gltf_mesh_0.json"));
        if (Mesh)
        {
            FTransform MeshTransform;
            MeshTransform.SetTranslation(glm::vec3(1, 1, 1));
            std::shared_ptr<AStaticMeshActor> StaticMeshActor = SpawnActor<AStaticMeshActor>(MeshTransform, "test mesh");
            StaticMeshActor->SetStaticMesh(Mesh);
        }
        // CreateMaterials();
        // std::shared_ptr<FImage> NoMetallicRoughnessImg = std::make_shared<FImage>();
        // NoMetallicRoughnessImg->Width = 1; NoMetallicRoughnessImg->Height = 1; NoMetallicRoughnessImg->Channel = 4; NoMetallicRoughnessImg->data_size = 4;
        // NoMetallicRoughnessImg->PixelFormat = EPixelFormat::PF_R8G8B8A8; NoMetallicRoughnessImg->data = new uint8[4];
        // NoMetallicRoughnessImg->data[0] = 0; NoMetallicRoughnessImg->data[1] = 255; NoMetallicRoughnessImg->data[2] = 255; NoMetallicRoughnessImg->data[3] = 255;
        // std::unique_ptr<UTexture> NoMetallicRoughnessTexture = std::make_unique<UTexture>("NoMetallicRoughnessTexture", 1, NoMetallicRoughnessImg);
        // BeginInitResource(NoMetallicRoughnessTexture->GetResource());
        // GetContentManager()->CreateDirectory("/Testgltf/0_WaterBottle.gltf_texture_0.json");
        // GetContentManager()->CreateFile("/NoMetallicRoughnessTexture.json", std::move(NoMetallicRoughnessTexture));
        // GetContentManager()->Flush();
        // std::shared_ptr<tinygltf::Model> Model = GetAssetLoader()->SyncReadGLTFModel(R"(D:\Nilou\Assets\Models\WaterBottle.gltf)");
        // GLTFParseResult Mesh = GameStatics::ParseToStaticMeshes(*Model, false);
        // nlohmann::json json;
        // std::ifstream in("D:\\Nilou\\Content\\Textures\\test.json");
        // in >> json;
        // Mesh.Textures[0]->Serialize(json);
        // std::string s = json.dump();
        // std::ofstream out("D:\\Nilou\\Content\\Textures\\test.json");
        // out << s;

        // Mesh.Materials[0]->Serialize(json);
        // std::ofstream out("D:\\Nilou\\Content\\Textures\\test_material.json");
        // std::string s = json.dump();
        // out << s;

        // Mesh.UniformBuffer->Serialize(json);
        // std::ofstream out("D:\\Nilou\\Content\\Textures\\test_ubo.json");
        // std::string s = json.dump();
        // out << s;

        // nlohmann::json json;
        // std::ifstream in("D:\\Nilou\\Content\\Testgltf\\GLTFMaterial.json");
        // in >> json;
        // std::shared_ptr<UMaterial> material = std::make_shared<UMaterial>();
        // material->Path = "/Content/Testgltf/GLTFMaterial.json";
        // material->Deserialize(json);
        
        // Mesh.Textures[0]->Deserialize(json);
        // for (auto &Texture : Mesh.Textures)
        //     GetContentManager()->AddGlobalTexture(Texture->Name, Texture);
        // for (auto &Material : Mesh.Materials)
        //     GetContentManager()->AddGlobalMaterial(Material->Name, Material);
        // for (auto &StaticMesh : Mesh.StaticMeshes)
        //     GetContentManager()->AddGlobalStaticMesh("WaterBottle.gltf", StaticMesh);
        // GetContentManager()->AddGlobalUniformBuffer("WaterBottle_ubo", Mesh.UniformBuffer);
        // Mesh.InitResource();
        // FTransform MeshTransform;
        // MeshTransform.SetTranslation(glm::vec3(1, 1, 1));
        // std::shared_ptr<AStaticMeshActor> StaticMeshActor = SpawnActor<AStaticMeshActor>(MeshTransform, "test mesh");
        // StaticMeshActor->SetStaticMesh(Mesh.StaticMeshes[0]);
        
        std::shared_ptr<ASphereActor> SphereActor = SpawnActor<ASphereActor>(FTransform::Identity, "test sky sphere");
        SphereActor->SphereComponent->SetRelativeScale3D(vec3(4000));
        SphereActor->SphereComponent->SetMaterial(dynamic_cast<UMaterial*>(GetContentManager()->GetContentByPath("/SkyAtmosphereMaterial.json")));
        std::shared_ptr<ASkyAtmosphereActor> SkyAtmosphereActor = SpawnActor<ASkyAtmosphereActor>(FTransform::Identity, "test atmosphere");
        std::shared_ptr<AArrowActor> ArrorActor = SpawnActor<AArrowActor>(FTransform::Identity, "test arrow");

        FTransform CameraActorTransform;
        CameraActorTransform.SetTranslation(glm::vec3(0, -2, 0));
        CameraActorTransform.SetRotator(FRotator(0, 90, 0));
        std::shared_ptr<ACameraActor> CameraActor = SpawnActor<ACameraActor>(CameraActorTransform, "test camera");
        CameraActor->SetCameraResolution(ivec2(GetAppication()->GetConfiguration().screenWidth, GetAppication()->GetConfiguration().screenHeight));
        CameraActor->SetMoveSpeed(100);

        FTransform LightActorTransform;
        LightActorTransform.SetTranslation(glm::vec3(10, 10, 10));
        LightActorTransform.SetRotator(FRotator(-45, 0, 0));
        std::shared_ptr<ALightActor> DirectionalLightActor = SpawnActor<ALightActor>(LightActorTransform, "test directional light");
        DirectionalLightActor->LightComponent->SetLightType(ELightType::LT_Directional);
        DirectionalLightActor->LightComponent->SetIntensity(10.f);

        
        std::shared_ptr<AGeoreferenceActor> GeoreferenceActor = SpawnActor<AGeoreferenceActor>(FTransform::Identity, "test georeference");
        GeoreferenceActor->SetGeoreferenceOrigin(84.77921, 45.65067, 604.42679);

        std::shared_ptr<ACesiumTilesetActor> TilesetActor = SpawnActor<ACesiumTilesetActor>(FTransform::Identity, "test tileset");
        TilesetActor->GetTilesetComponent()->SetURI(R"(E:\TuZiGou(20210608)\TuZiGou_3dtiles_cesiumlab\tileset.json)");
        // TilesetActor->GetTilesetComponent()->SetMaxScreenSpaceError(0);
        // TilesetActor->GetTilesetComponent()->SetEnableFrustumCulling(false);
        TilesetActor->GetTilesetComponent()->SetShowBoundingBox(true);

        // std::shared_ptr<ALineBatchActor> LineBatchActor = SpawnActor<ALineBatchActor>(FTransform::Identity, "test linebatch");
        // std::vector<FBatchedLine> lines;
        // lines.emplace_back(dvec3(0, 0, 0), dvec3(1, 1, 1));
        // LineBatchActor->LineBatchComponent->DrawLines(lines);

    }

    void UWorld::Tick(double DeltaTime)
    {
        for (std::shared_ptr<AActor> Actor : Actors)
        {
            Actor->Tick(DeltaTime);
            std::vector<UActorComponent *> Components;
            Actor->GetComponents(Components);
            for (UActorComponent *Component : Components)
            {
                Component->TickComponent(DeltaTime);
            }
        }
    }
    
    void UWorld::SendAllEndOfFrameUpdates()
    {
        for (std::shared_ptr<AActor> Actor : Actors)
        {
            Actor->ForEachComponent<UActorComponent>([](UActorComponent *Component) {
                Component->DoDeferredRenderUpdates();
            });
        }
    }

}