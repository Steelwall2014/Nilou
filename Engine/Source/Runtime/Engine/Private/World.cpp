#include <fstream>

#include "AssetImporter.h"
#include "Engine/World.h"
#include "Math/Transform.h"

#include "NObject/Package.h"
#include "StaticMeshResources.h"
#include "Components/MeshComponent.h"
#include "Components/ArrowComponent.h"
#include "PrimitiveUtils.h"

#include "GameFramework/StaticMeshActor.h"
#include "GameFramework/CameraActor.h"
#include "GameFramework/LightActor.h"
#include "GameFramework/ArrowActor.h"
#include "GameFramework/SphereActor.h"
#include "GameFramework/SkyAtmosphereActor.h"
#include "GameFramework/GeoreferenceActor.h"
// #include "GameFramework/Cesium3DTilesetActor.h"
// #include "Cesium3DTileset.h"
#include "GameFramework/LineBatchActor.h"
#include "GameFramework/VirtualHeightfieldMeshActor.h"
#include "GameFramework/FFTOceanActor.h"
#include "GameFramework/ReflectionProbe.h"
#include "Test.h"
#include "Test2.h"

#include "Engine/Texture2D.h"

#include "Geospatial/Georeference.h"
#include "Engine/VirtualTexture2D.h"
#include "MaterialUniformBlocks.h"

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

    static void LoadPBRExibition(UWorld* World)
    {
        // UStaticMesh* Mesh = GetContentManager()->GetStaticMeshByPath("Testgltf/WaterBottle.gltf_mesh_0.nasset");
        // if (Mesh)
        // {
        //     FTransform MeshTransform;
        //     MeshTransform.SetRotator(FRotator(0, 0, -90));
        //     for (int i = 1; i <= 1; i++)
        //     {
        //         for (int j = 1; j <= 1; j++)
        //         {
        //             MeshTransform.SetTranslation(vec3(i, j, 1));
        //             std::shared_ptr<AStaticMeshActor> StaticMeshActor = World->SpawnActor<AStaticMeshActor>(MeshTransform, "test mesh_" + std::to_string(i) + "_" + std::to_string(j));
        //             StaticMeshActor->SetStaticMesh(Mesh);
        //         }
        //     }
        // }
        

        std::vector<ASphereActor*> PBRSpheres;
        {
            NPackage* Package = CreatePackage("/Engine/Materials/PBRExhibitionMaterial");
            UMaterial* PBRExhibitionMaterial = FindObject<UMaterial>(Package, "PBRExhibitionMaterial");
            if (!PBRExhibitionMaterial)
            {
                PBRExhibitionMaterial = NewObject<UMaterial>(Package, "PBRExhibitionMaterial");
                PBRExhibitionMaterial->SetShaderFileVirtualPath("/Shaders/Private/Materials/PBRExhibition_Mat.slang");
                PBRExhibitionMaterial->SetScalarParameterValue("Red", 1.f);
                PBRExhibitionMaterial->SetScalarParameterValue("Green", 1.f);
                PBRExhibitionMaterial->SetScalarParameterValue("Blue", 1.f);
                PBRExhibitionMaterial->SetScalarParameterValue("Metallic", 0.f);
                PBRExhibitionMaterial->SetScalarParameterValue("Roughness", 0.8f);
                PBRExhibitionMaterial->MarkPackageDirty();
                NPackage::SavePackage(Package);
            }
            for (int i = 1; i <= 5; i++)
            {
                for (int j = 0; j <= 4; j++)
                {
                    UMaterialInstance* mat = PBRExhibitionMaterial->CreateMaterialInstance(Package, "test sphere_" + std::to_string(i) + "_" + std::to_string(j));
                    mat->SetScalarParameterValue("Red", 1.f);
                    mat->SetScalarParameterValue("Green", 1.f);
                    mat->SetScalarParameterValue("Blue", 1.f);
                    mat->SetScalarParameterValue("Metallic", (i-1)*0.25f);
                    mat->SetScalarParameterValue("Roughness", (j)*0.25f);
                    FTransform SphereTransform;
                    SphereTransform.SetTranslation(FVector(-2, j*0.3, i*0.3));
                    SphereTransform.SetScale3D(FVector(0.1));
                    ASphereActor* Sphere = World->SpawnActor<ASphereActor>(SphereTransform, "test sphere_" + std::to_string(i) + "_" + std::to_string(j));
                    Sphere->SphereComponent->SetMaterial(mat);
                    PBRSpheres.push_back(Sphere);
                }
            }
        }

        FTransform CubeTransform;
        CubeTransform.SetScale3D(FVector(15, 15, 0.05));
        NPackage* CubePackage = CreatePackage("/Engine/Meshes/Cube");
        UStaticMesh* Cube = FindObject<UStaticMesh>(CubePackage, "Cube");
        if (!Cube)
        {
            Cube = NewObject<UStaticMesh>(CubePackage, "Cube");
            std::vector<FDynamicMeshVertex> OutVerts;
            std::vector<uint32> OutIndices;
            BuildCuboidVerts(1, 1, 1, OutVerts, OutIndices);
            Cube->MaterialSlots.Add(GetColoredMaterial());
            FStaticMeshLODResources* resources = new FStaticMeshLODResources();
            FStaticMeshSection* section = new FStaticMeshSection();
            section->VertexBuffers.InitFromDynamicVertex(&section->VertexFactory, OutVerts);
            section->IndexBuffer.Init(OutIndices);
            resources->Sections.push_back(section);
            Cube->RenderData->LODResources.push_back(resources);
        }
        AStaticMeshActor* StaticMeshActor = World->SpawnActor<AStaticMeshActor>(CubeTransform, "test cube");
        StaticMeshActor->SetStaticMesh(Cube);
        StaticMeshActor->StaticMeshComponent->SetReflectionProbeBlendMode(RPBM_Off);

        AArrowActor* ArrowActor = World->SpawnActor<AArrowActor>(FTransform::Identity, "test arrow");
        
        FTransform ReflectionProbeTransform1;
        ReflectionProbeTransform1.SetTranslation(FVector(-2, 0, 1.5));
        AReflectionProbe* ReflectionProbe1 = World->SpawnActor<AReflectionProbe>(ReflectionProbeTransform1, "test ReflectionProbe1");
        for (auto Sphere : PBRSpheres)
            ReflectionProbe1->ReflectionProbeComponent->HideActorComponents(Sphere);
        ReflectionProbe1->ReflectionProbeComponent->SetExtent(FVector(1, 4, 4));

        FTransform ReflectionProbeTransform2;
        ReflectionProbeTransform2.SetTranslation(FVector(1, 1, 1));
        AReflectionProbe* ReflectionProbe2 = World->SpawnActor<AReflectionProbe>(ReflectionProbeTransform2, "test ReflectionProbe2");
    }

    static void LoadDamagedHelmet(UWorld* World)
    {
        NPackage* Package = CreatePackage("/Engine/Testgltf/DamagedHelmet");
        UStaticMesh* Helmet = FindObject<UStaticMesh>(Package, "DamagedHelmet");
        if (Helmet == nullptr)
        {
            std::vector<UTexture2D*> Textures;
            std::vector<UMaterial*> Materials;
            std::vector<UStaticMesh*> StaticMeshes;
            FGLTFImporter::Import(R"(D:\Nilou\Assets\Models\DamagedHelmet.gltf)", "/Testgltf", Textures, Materials, StaticMeshes);
            Helmet = StaticMeshes[0];
        }
        FTransform MeshTransform;
        AStaticMeshActor* StaticMeshActor = World->SpawnActor<AStaticMeshActor>(MeshTransform, "damaged helmet");
        StaticMeshActor->SetStaticMesh(Helmet);
        StaticMeshActor->StaticMeshComponent->SetReflectionProbeBlendMode(RPBM_Simple);
    }

    static void LoadSkyAtmosphere(UWorld* World)
    {
        NPackage* Package = CreatePackage("/Engine/Materials/SkyAtmosphereMaterial");
        UMaterial* SkyAtmosphereMaterial = FindObject<UMaterial>(Package, "SkyAtmosphereMaterial");
        if (!SkyAtmosphereMaterial)
        {
            SkyAtmosphereMaterial = NewObject<UMaterial>(Package, "SkyAtmosphereMaterial");
            SkyAtmosphereMaterial->InitializeResources();
            SkyAtmosphereMaterial->SetRasterizerState(FRasterizerStateInitializer(FM_Solid, CM_None));
            SkyAtmosphereMaterial->SetShadingModel(EShadingModel::SM_SkyAtmosphere);
            SkyAtmosphereMaterial->SetShaderFileVirtualPath("/Shaders/Private/Materials/SkyAtmosphereMaterial_Mat.slang");
            NPackage::SavePackage(Package);
        }
        ASphereActor* SphereActor = World->SpawnActor<ASphereActor>(FTransform::Identity, "test sky sphere");
        SphereActor->SphereComponent->SetCastShadow(false);
        SphereActor->SphereComponent->SetRelativeScale3D(FVector(4000));
        SphereActor->SphereComponent->SetMaterial(SkyAtmosphereMaterial);
        SphereActor->SphereComponent->SetReflectionProbeBlendMode(EReflectionProbeBlendMode::RPBM_Off);
        ASkyAtmosphereActor* SkyAtmosphereActor = World->SpawnActor<ASkyAtmosphereActor>(FTransform::Identity, "test atmosphere");

        AReflectionProbe* SkyboxReflectionProbe = World->SpawnActor<AReflectionProbe>(FTransform::Identity, "test SkyboxReflectionProbe");
        SkyboxReflectionProbe->ReflectionProbeComponent->SetExtent(FVector(0));
        SkyboxReflectionProbe->ReflectionProbeComponent->ShowOnlyActorComponents(SphereActor);
        World->SkyboxReflectionProbe = SkyboxReflectionProbe;
    }
    static void LoadVirtualHeightfieldMesh(UWorld* World)
    {
#if NILOU_ENABLE_VIRTUAL_HEIGHT_FIELD
        FTransform VHMTransform;
        VHMTransform.SetScale3D(dvec3(0.5, 0.5, 1));
        std::shared_ptr<AVirtualHeightfieldMeshActor> VHMActor = World->SpawnActor<AVirtualHeightfieldMeshActor>(VHMTransform, "test VHM");
        VHMActor->VHMComponent->SetHeightfieldTexture(dynamic_cast<UVirtualTexture*>(GetContentManager()->GetContentByPath("/Textures/Karelia_VirtualTexture.nasset")));
        VHMActor->VHMComponent->SetMaterial(GetContentManager()->GetMaterialByPath("/Materials/ColoredMaterial.nasset"));
        VHMActor->VHMComponent->SetReflectionProbeBlendMode(EReflectionProbeBlendMode::RPBM_Off);
#endif
    }

    static void Load3DTileset(UWorld* World)
    {
#if NILOU_ENABLE_3DTILES
        std::shared_ptr<AGeoreferenceActor> GeoreferenceActor = World->SpawnActor<AGeoreferenceActor>(FTransform::Identity, "test georeference");
        GeoreferenceActor->SetGeoreferenceOrigin(84.77921, 45.65067, 604.42679);
        // GeoreferenceActor->SetGeoreferenceOrigin(-75.612037, 40.043799, 123.340197);

        // std::shared_ptr<ACesiumTilesetActor> TilesetActor = SpawnActor<ACesiumTilesetActor>(FTransform::Identity, "test tileset");
        std::shared_ptr<ACesium3DTileset> TilesetActor = World->SpawnActor<ACesium3DTileset>(FTransform::Identity, "test tileset");
        // TilesetActor->SetURI(R"(E:\TuZiGou(20210608)\TuZiGou_3dtile_debug\tileset.json)");
        TilesetActor->SetURI(R"(E:\TuZiGou(20210608)\TuZiGou_3dtiles_cesiumlab\tileset.json)");
        // TilesetActor->GetTilesetComponent()->SetURI(R"(E:\cesium-unreal\extern\cesium-native\Cesium3DTilesSelection\test\data\Tileset\tileset.json)");
        // TilesetActor->GetTilesetComponent()->SetMaxScreenSpaceError(0);
        // TilesetActor->GetTilesetComponent()->SetEnableFrustumCulling(false);
        TilesetActor->bShowBoundingBox = true;
        // TilesetActor->GetTilesetComponent()->SetReflectionProbeBlendMode(RPBM_Off);
#endif
    }

    static void LoadFFTOcean(UWorld* World)
    {
        AFFTOceanActor* FFTOceanActor = World->SpawnActor<AFFTOceanActor>(FTransform::Identity, "test ocean");
    }

    static void LoadLineBatch(UWorld* World)
    {
        ALineBatchActor* LineBatchActor = World->SpawnActor<ALineBatchActor>(FTransform::Identity, "test linebatch");
        std::vector<FBatchedLine> lines;
        lines.emplace_back(FVector(0, 0, 0), FVector(1, 1, 1));
        LineBatchActor->LineBatchComponent->DrawLines(lines);
    }

    void UWorld::BeginPlay()
    {
        bHasBegunPlay = true;
        // TestSavePackage();
        TestLoadPackage();
        // TestSaveDependencyPackage();
        TestLoadDependencyPackage();

        FTransform CameraActorTransform;
        CameraActorTransform.SetTranslation(FVector(-2, 0, 2));
        CameraActorTransform.SetRotator(FRotator(-45, 0, 0));
        ACameraActor* CameraActor = SpawnActor<ACameraActor>(CameraActorTransform, "test camera");
        CameraActor->GetCameraComponent()->ScreenResolution = FIntVector2(1600, 900);

        FTransform LightActorTransform;
        LightActorTransform.SetRotator(FRotator(-45, -45, 0));
        ALightActor* DirectionalLightActor = SpawnActor<ALightActor>(LightActorTransform, "test directional light");

        UMaterial::GetDefaultMaterial();
        AArrowActor* ArrorActor = SpawnActor<AArrowActor>(FTransform::Identity, "test arrow");
        // LoadPBRExibition(this);

        LoadSkyAtmosphere(this);

        //LoadVirtualHeightfieldMesh(this);

        // LoadFFTOcean(this);

        // Load3DTileset(this);

        // LoadDamagedHelmet(this);

        // // std::vector<FDynamicMeshVertex> OutVerts;
        // // std::vector<uint32> OutIndices;
        // // BuildCuboidVerts(1, 1, 1, OutVerts, OutIndices);
        // // UStaticMesh *cube = GetContentManager()->CreateAsset<UStaticMesh>("StaticMeshes/Cube.nasset");
        // // cube->LocalBoundingBox = FBoundingBox(dvec3(-0.5), dvec3(0.5));
        // // cube->MaterialSlots.push_back(GetContentManager()->GetMaterialByPath("/Materials/ColoredMaterial.nasset"));
        // // std::unique_ptr<FStaticMeshLODResources> resources = std::make_unique<FStaticMeshLODResources>();
        // // std::unique_ptr<FStaticMeshSection> section = std::make_unique<FStaticMeshSection>();
        // // section->VertexBuffers.InitFromDynamicVertex(&section->VertexFactory, OutVerts);
        // // section->IndexBuffer.Init(OutIndices);
        // // resources->Sections.push_back(std::move(section));
        // // cube->RenderData->LODResources.push_back(std::move(resources));
        // // cube->Name = "Cube";

        // GetContentManager()->ForEachContent([](NAsset* Obj){
        //     if (Obj->IsA(UStaticMesh::StaticClass()))
        //     {
        //         Obj->ContentEntry->bIsDirty = true;
        //         Obj->ContentEntry->bNeedFlush = true;
        //     }
        // });
        // GetContentManager()->Flush();

        // std::shared_ptr<FImage> img =GetAssetLoader()->SyncOpenAndReadImage(R"(E:\Downloads\ibl_brdf_lut.png)");
        // UTexture2D* LUT = GetContentManager()->CreateAsset<UTexture2D>("/Textures/IBL_BRDF_LUT.nasset");
        // LUT->Name = "IBL_BRDF_LUT";
        // LUT->ImageData = img;
        // LUT->UpdateResource();

        // UMaterial* PBRExhibition = GetContentManager()->CreateAsset<UMaterial>("PBRExhibition", "/Materials");
        // PBRExhibition->SetShaderFileVirtualPath("/Shaders/Private/Materials/PBRExhibition_Mat.glsl");
        // PBRExhibition->SetScalarParameterValue("Red", 1.f);
        // PBRExhibition->SetScalarParameterValue("Green", 1.f);
        // PBRExhibition->SetScalarParameterValue("Blue", 1.f);
        // PBRExhibition->SetScalarParameterValue("Metallic", 0.f);
        // PBRExhibition->SetScalarParameterValue("Roughness", 0.8f);
        // PBRExhibition->MarkAssetDirty();

        // UMaterial* MirrorMaterial = GetContentManager()->CreateAsset<UMaterial>("MirrorMaterial", "/Materials");
        // MirrorMaterial->SetShaderFileVirtualPath("/Shaders/Private/Materials/MirrorMaterial_Mat.glsl");
        // MirrorMaterial->MarkAssetDirty();

        // UMaterial* OceanMaterial2 = GetContentManager()->CreateAsset<UMaterial>("OceanMaterial2", "/Materials");
        // OceanMaterial2->SetShaderFileVirtualPath("/Shaders/Private/Materials/OceanMaterial2_Mat.glsl");
        // OceanMaterial2->MarkAssetDirty();

    }

    void UWorld::Tick(double DeltaTime)
    {
        for (AActor* Actor : Actors)
        {
            Actor->Tick(DeltaTime);
            std::vector<UActorComponent*> Components;
            Actor->GetComponents(Components);
            for (auto Component : Components)
            {
                if (Component)
                    Component->TickComponent(DeltaTime);
            }
        }
    }

    void UWorld::Release()
    {
        CameraActors.clear();
        Actors.clear();
    }
    
    void UWorld::SendAllEndOfFrameUpdates()
    {
        for (AActor* Actor : Actors)
        {
            Actor->ForEachComponent<UActorComponent>([](UActorComponent* Component) {
                Component->DoDeferredRenderUpdates();
            });
        }
    }

}