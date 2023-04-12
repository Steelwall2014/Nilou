#include <fstream>

#include "World.h"
#include "BaseApplication.h"
#include "Common/Transform.h"

#include "StaticMeshResources.h"
#include "Common/Asset/AssetLoader.h"
#include "Common/Components/MeshComponent.h"
#include "Common/Components/ArrowComponent.h"
#include "PrimitiveUtils.h"
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
#include "Common/Actor/ReflectionProbe.h"

#include "Texture2D.h"

#include "Georeference.h"
#include "VirtualTexture2D.h"

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

        UMaterial* MirrorMaterial = GetContentManager()->GetMaterialByPath("/Materials/MirrorMaterial.nasset");
        UMaterial* ColoredMaterial = GetContentManager()->GetMaterialByPath("/Materials/ColoredMaterial.nasset");
        // MirrorMaterial->GetResource()->RasterizerState.CullMode = ERasterizerCullMode::CM_CCW;

        UStaticMesh *Mesh = GetContentManager()->GetStaticMeshByPath("Testgltf/WaterBottle.gltf_mesh_0.nasset");
        if (Mesh)
        {
            FTransform MeshTransform;
            MeshTransform.SetRotator(FRotator(0, 0, -90));
            for (int i = 1; i <= 1; i++)
            {
                for (int j = 1; j <= 1; j++)
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
        // DirectionalLightActor->LightComponent->SetLightType(ELightType::LT_Directional);
        // DirectionalLightActor->LightComponent->SetIntensity(10.f);

        // FTransform VHMTransform;
        // VHMTransform.SetScale3D(dvec3(0.5, 0.5, 1));
        // std::shared_ptr<AVirtualHeightfieldMeshActor> VHMActor = SpawnActor<AVirtualHeightfieldMeshActor>(VHMTransform, "test VHM");
        // VHMActor->VHMComponent->SetHeightfieldTexture(dynamic_cast<UVirtualTexture*>(GetContentManager()->GetContentByPath("/Textures/TestVirtualHeightfield.nasset")));
        // VHMActor->VHMComponent->SetMaterial(GetContentManager()->GetMaterialByPath("/Materials/ColoredMaterial.nasset"));
        
        // std::shared_ptr<AGeoreferenceActor> GeoreferenceActor = SpawnActor<AGeoreferenceActor>(FTransform::Identity, "test georeference");
        // GeoreferenceActor->SetGeoreferenceOrigin(84.77921, 45.65067, 604.42679);
        // // GeoreferenceActor->SetGeoreferenceOrigin(-75.612037, 40.043799, 123.340197);

        // std::shared_ptr<ACesiumTilesetActor> TilesetActor = SpawnActor<ACesiumTilesetActor>(FTransform::Identity, "test tileset");
        // TilesetActor->GetTilesetComponent()->SetURI(R"(E:\TuZiGou(20210608)\TuZiGou_3dtiles_cesiumlab\tileset.json)");
        // // TilesetActor->GetTilesetComponent()->SetURI(R"(E:\TuZiGou(20210608)\TuZiGou_3dtiles_cesiumlab\tileset.json)");
        // // TilesetActor->GetTilesetComponent()->SetURI(R"(E:\cesium-unreal\extern\cesium-native\Cesium3DTilesSelection\test\data\Tileset\tileset.json)");
        // // TilesetActor->GetTilesetComponent()->SetMaxScreenSpaceError(0);
        // // TilesetActor->GetTilesetComponent()->SetEnableFrustumCulling(false);
        // TilesetActor->GetTilesetComponent()->SetShowBoundingBox(true);

        // std::shared_ptr<ALineBatchActor> LineBatchActor = SpawnActor<ALineBatchActor>(FTransform::Identity, "test linebatch");
        // std::vector<FBatchedLine> lines;
        // lines.emplace_back(dvec3(0, 0, 0), dvec3(1, 1, 1));
        // LineBatchActor->LineBatchComponent->DrawLines(lines);

        // std::shared_ptr<AFFTOceanActor> FFTOceanActor = SpawnActor<AFFTOceanActor>(FTransform::Identity, "test ocean");

        FTransform ReflectionProbeTransform1;
        ReflectionProbeTransform1.SetTranslation(dvec3(-1, 1, 1));
        std::shared_ptr<AReflectionProbe> ReflectionProbe1 = SpawnActor<AReflectionProbe>(ReflectionProbeTransform1, "test ReflectionProbe1");

        /** To test Blend */
        ReflectionProbe1->ReflectionProbeComponent->SetExtent(dvec3(10));

        FTransform ReflectionProbeTransform2;
        ReflectionProbeTransform2.SetTranslation(dvec3(1, 1, 1));
        std::shared_ptr<AReflectionProbe> ReflectionProbe2 = SpawnActor<AReflectionProbe>(ReflectionProbeTransform2, "test ReflectionProbe2");

        /** To test Blend */
        ReflectionProbe2->ReflectionProbeComponent->SetExtent(dvec3(10));

        std::shared_ptr<AReflectionProbe> SkyboxReflectionProbe = SpawnActor<AReflectionProbe>(FTransform::Identity, "test SkyboxReflectionProbe");
        SkyboxReflectionProbe->ReflectionProbeComponent->SetExtent(dvec3(0));
        SkyboxReflectionProbe->ReflectionProbeComponent->ShowOnlyActorComponents(SphereActor.get());
        this->SkyboxReflectionProbe = SkyboxReflectionProbe.get();

        FTransform MirrorTransform;
        MirrorTransform.SetTranslation(dvec3(-1, 1, 1));
        MirrorTransform.SetScale3D(dvec3(0.2));
        std::shared_ptr<ASphereActor> MirrorActor = SpawnActor<ASphereActor>(MirrorTransform, "test MirrorActor");
        MirrorActor->SphereComponent->SetMaterial(MirrorMaterial);


        // GetContentManager()->ForEachContent([](UObject* Obj){
        //     if (Obj->IsA(UVirtualTexture::StaticClass()))
        //     {
        //         Obj->ContentEntry->bIsDirty = true;
        //     }
        // });

        // std::shared_ptr<FImage> img =GetAssetLoader()->SyncOpenAndReadImage(R"(E:\Downloads\ibl_brdf_lut.png)");
        // UTexture2D* LUT = GetContentManager()->CreateFile<UTexture2D>("/Textures/IBL_BRDF_LUT.nasset");
        // LUT->Name = "IBL_BRDF_LUT";
        // LUT->ImageData = img;
        // LUT->UpdateResource();

        // UMaterial* MirrorMaterial = GetContentManager()->CreateFile<UMaterial>("/Materials/MirrorMaterial.nasset");
        // MirrorMaterial->Name = "MirrorMaterial";
        // MirrorMaterial->SetShaderFileVirtualPath("/Shaders/Materials/MirrorMaterial_Mat.glsl");

    }

    void UWorld::Tick(double DeltaTime)
    {
        for (std::shared_ptr<AActor> Actor : Actors)
        {
            Actor->Tick(DeltaTime);
            std::vector<std::weak_ptr<UActorComponent>> Components;
            Actor->GetComponents(Components);
            for (auto Component : Components)
            {
                if (!Component.expired())
                    Component.lock()->TickComponent(DeltaTime);
            }
        }
    }
    
    void UWorld::SendAllEndOfFrameUpdates()
    {
        for (std::shared_ptr<AActor> Actor : Actors)
        {
            Actor->ForEachComponent<UActorComponent>([](std::shared_ptr<UActorComponent> Component) {
                Component->DoDeferredRenderUpdates();
            });
        }
    }

}