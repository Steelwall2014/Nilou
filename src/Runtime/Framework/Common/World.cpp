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
#include "Common/Actor/VirtualHeightfieldMeshActor.h"
#include "Common/Actor/FFTOceanActor.h"

#include "Georeference.h"

// #include <ogrsf_frmts.h>

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

        UStaticMesh *Mesh = GetContentManager()->GetStaticMeshByPath("Testgltf/WaterBottle.gltf_mesh_0.nasset");
        if (Mesh)
        {
            FTransform MeshTransform;
            MeshTransform.SetRotator(FRotator(0, 0, -90));
            for (int i = -1; i <= 1; i++)
            {
                for (int j = -1; j <= 1; j++)
                {
                    MeshTransform.SetTranslation(glm::vec3(i, j, 1));
                    std::shared_ptr<AStaticMeshActor> StaticMeshActor = SpawnActor<AStaticMeshActor>(MeshTransform, "test mesh_" + std::to_string(i) + "_" + std::to_string(j));
                    StaticMeshActor->SetStaticMesh(Mesh);
                }
            }
        }

        // std::vector<FDynamicMeshVertex> OutVerts;
        // std::vector<uint32> OutIndices;
        // BuildCuboidVerts(1, 1, 1, OutVerts, OutIndices);
        // UStaticMesh *cube = GetContentManager()->CreateFile<UStaticMesh>("StaticMeshes/Cube.nasset");
        // cube->LocalBoundingBox = FBoundingBox(dvec3(-0.5), dvec3(0.5));
        // cube->MaterialSlots.push_back(GetContentManager()->GetMaterialByPath("/Materials/ColoredMaterial.nasset"));
        // std::unique_ptr<FStaticMeshLODResources> resources = std::make_unique<FStaticMeshLODResources>();
        // std::unique_ptr<FStaticMeshSection> section = std::make_unique<FStaticMeshSection>();
        // section->VertexBuffers.InitFromDynamicVertex(&section->VertexFactory, OutVerts);
        // section->IndexBuffer.Init(OutIndices);
        // resources->Sections.push_back(std::move(section));
        // cube->RenderData->LODResources.push_back(std::move(resources));
        // cube->Name = "Cube";

        FTransform CubeTransform;
        CubeTransform.SetScale3D(vec3(15, 15, 0.05));
        UStaticMesh *Cube = GetContentManager()->GetStaticMeshByPath("StaticMeshes/Cube.nasset");
        std::shared_ptr<AStaticMeshActor> StaticMeshActor = SpawnActor<AStaticMeshActor>(CubeTransform, "test cube");
        StaticMeshActor->SetStaticMesh(Cube);
        
        UMaterial* SkyAtmosphereMaterial = GetContentManager()->GetMaterialByPath("/Materials/SkyAtmosphereMaterial.nasset");
        SkyAtmosphereMaterial->SetShadingModel(EShadingModel::SM_SkyAtmosphere);
        std::shared_ptr<ASphereActor> SphereActor = SpawnActor<ASphereActor>(FTransform::Identity, "test sky sphere");
        SphereActor->SphereComponent->SetCastShadow(false);
        SphereActor->SphereComponent->SetRelativeScale3D(vec3(4000));
        SphereActor->SphereComponent->SetMaterial(SkyAtmosphereMaterial);
        std::shared_ptr<ASkyAtmosphereActor> SkyAtmosphereActor = SpawnActor<ASkyAtmosphereActor>(FTransform::Identity, "test atmosphere");
        std::shared_ptr<AArrowActor> ArrorActor = SpawnActor<AArrowActor>(FTransform::Identity, "test arrow");

        FTransform CameraActorTransform;
        CameraActorTransform.SetTranslation(glm::vec3(0, -2, 0));
        CameraActorTransform.SetRotator(FRotator(0, 90, 0));
        std::shared_ptr<ACameraActor> CameraActor = SpawnActor<ACameraActor>(CameraActorTransform, "test camera");
        CameraActor->SetCameraResolution(ivec2(GetAppication()->GetConfiguration().screenWidth, GetAppication()->GetConfiguration().screenHeight));
        CameraActor->SetMoveSpeed(100);

        FTransform LightActorTransform;
        // LightActorTransform.SetTranslation(glm::vec3(10, 10, 10));
        LightActorTransform.SetRotator(FRotator(-45, -45, 0));
        std::shared_ptr<ALightActor> DirectionalLightActor = SpawnActor<ALightActor>(LightActorTransform, "test directional light");
        DirectionalLightActor->LightComponent->SetLightType(ELightType::LT_Directional);
        // DirectionalLightActor->LightComponent->SetIntensity(10.f);

        // FTransform VHMTransform;
        // VHMTransform.SetScale3D(dvec3(0.1, 0.1, 1));
        // std::shared_ptr<AVirtualHeightfieldMeshActor> VHMActor = SpawnActor<AVirtualHeightfieldMeshActor>(VHMTransform, "test VHM");
        // VHMActor->VHMComponent->SetHeightfieldTexture(dynamic_cast<UVirtualTexture*>(GetContentManager()->GetContentByPath("/Textures/Karelia_VirtualTexture.nasset")));
        // VHMActor->VHMComponent->SetMaterial(GetContentManager()->GetMaterialByPath("/Materials/ColoredMaterial.nasset"));
        
        // std::shared_ptr<AGeoreferenceActor> GeoreferenceActor = SpawnActor<AGeoreferenceActor>(FTransform::Identity, "test georeference");
        // GeoreferenceActor->SetGeoreferenceOrigin(84.77921, 45.65067, 604.42679);
        // GeoreferenceActor->SetGeoreferenceOrigin(-75.612037, 40.043799, 123.340197);

        // std::shared_ptr<ACesiumTilesetActor> TilesetActor = SpawnActor<ACesiumTilesetActor>(FTransform::Identity, "test tileset");
        // // TilesetActor->GetTilesetComponent()->SetURI(R"(E:\TuZiGou(20210608)\TuZiGou_3dtiles_cesiumlab\tileset.json)");
        // // TilesetActor->GetTilesetComponent()->SetURI(R"(E:\cesium-unreal\extern\cesium-native\Cesium3DTilesSelection\test\data\Tileset\tileset.json)");
        // // TilesetActor->GetTilesetComponent()->SetMaxScreenSpaceError(0);
        // TilesetActor->GetTilesetComponent()->SetEnableFrustumCulling(false);
        // TilesetActor->GetTilesetComponent()->SetShowBoundingBox(true);

        std::shared_ptr<ALineBatchActor> LineBatchActor = SpawnActor<ALineBatchActor>(FTransform::Identity, "test linebatch");
        std::vector<FBatchedLine> lines;
        lines.emplace_back(dvec3(0, 0, 0), dvec3(1, 1, 1));
        LineBatchActor->LineBatchComponent->DrawLines(lines);

        std::shared_ptr<AFFTOceanActor> FFTOceanActor = SpawnActor<AFFTOceanActor>(FTransform::Identity, "test ocean");

        // GetContentManager()->ForEachContent([](UObject* Obj){
        //     if (Obj->IsA(UTexture::StaticClass()))
        //     {
        //         UTexture* tex = (UTexture*)Obj;
        //         if (tex->GetMinFilter() == ETextureFilters::TF_Linear)
        //         {
        //             tex->GetResource()->GetSamplerRHI()->Params.Min_Filter = ETextureFilters::TF_Linear_Mipmap_Linear;
        //         Obj->ContentEntry->bIsDirty = true;

        //         }
        //     }
        // });

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