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
        SphereActor->SphereComponent->SetMaterial(GetContentManager()->GetGlobalMaterial("SkyAtmosphereMaterial"));
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