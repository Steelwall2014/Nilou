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

    void CreateMaterials()
    {
        std::shared_ptr<FMaterial> DefaultMaterial = std::make_shared<FMaterial>("DefaultMaterial");
        FContentManager::GetContentManager().AddGlobalMaterial(DefaultMaterial->GetMaterialName(), DefaultMaterial);
        DefaultMaterial->UpdateMaterialCode(
        R"(
            #include "../include/BasePassCommon.glsl"
            vec4 MaterialGetBaseColor(VS_Out vs_out)
            {
                return vec4(0, 0, 0, 1);
            }
            vec3 MaterialGetEmissive(VS_Out vs_out)
            {
                return vec3(0);
            }
            vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
            {
                return normalize(vs_out.TBN * vec3(0, 0, 1));
            }
            float MaterialGetRoughness(VS_Out vs_out)
            {
                return 0.5;
            }
            float MaterialGetMetallic(VS_Out vs_out)
            {
                return 0.5;
            }
            vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
            {
                return vec3(0);
            }
        )");

        std::shared_ptr<FMaterial> ColoredMaterial = std::make_shared<FMaterial>("ColoredMaterial");
        FContentManager::GetContentManager().AddGlobalMaterial(ColoredMaterial->GetMaterialName(), ColoredMaterial);
        ColoredMaterial->RasterizerState.CullMode = CM_None;
        ColoredMaterial->UpdateMaterialCode(
        R"(
            #include "../include/BasePassCommon.glsl"
            vec4 MaterialGetBaseColor(VS_Out vs_out)
            {
                return vs_out.Color;
            }
            vec3 MaterialGetEmissive(VS_Out vs_out)
            {
                return vec3(0);
            }
            vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
            {
                return normalize(vs_out.TBN * vec3(0, 0, 1));
            }
            float MaterialGetRoughness(VS_Out vs_out)
            {
                return 0.5;
            }
            float MaterialGetMetallic(VS_Out vs_out)
            {
                return 0.5;
            }
            vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
            {
                return vec3(0);
            }
        )");

        std::shared_ptr<FMaterial> SkyAtmosphereMaterial = std::make_shared<FMaterial>("SkyAtmosphereMaterial");
        FContentManager::GetContentManager().AddGlobalMaterial(SkyAtmosphereMaterial->GetMaterialName(), SkyAtmosphereMaterial);
        SkyAtmosphereMaterial->RasterizerState.CullMode = CM_None;
        SkyAtmosphereMaterial->DepthStencilState.bEnableFrontFaceStencil = true;
        SkyAtmosphereMaterial->DepthStencilState.FrontFaceStencilTest = ECompareFunction::CF_Always;
        SkyAtmosphereMaterial->DepthStencilState.FrontFacePassStencilOp = EStencilOp::SO_Replace;
        SkyAtmosphereMaterial->DepthStencilState.bEnableBackFaceStencil = true;
        SkyAtmosphereMaterial->DepthStencilState.BackFaceStencilTest = ECompareFunction::CF_Always;
        SkyAtmosphereMaterial->DepthStencilState.BackFacePassStencilOp = EStencilOp::SO_Replace;
        SkyAtmosphereMaterial->StencilRefValue = 255;
        SkyAtmosphereMaterial->UpdateMaterialCode(
        R"(
            #include "../include/BasePassCommon.glsl"
            vec4 MaterialGetBaseColor(VS_Out vs_out)
            {
                return vec4(0);
            }
            vec3 MaterialGetEmissive(VS_Out vs_out)
            {
                return vec3(0);
            }
            vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
            {
                return normalize(vs_out.TBN * vec3(0, 0, 1));
            }
            float MaterialGetRoughness(VS_Out vs_out)
            {
                return 0.5;
            }
            float MaterialGetMetallic(VS_Out vs_out)
            {
                return 0.5;
            }
            vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
            {
                return vec3(0);
            }
        )");
        
        std::shared_ptr<FMaterial> WireframeMaterial = std::make_shared<FMaterial>("WireframeMaterial");
        FContentManager::GetContentManager().AddGlobalMaterial(WireframeMaterial->GetMaterialName(), WireframeMaterial);
        WireframeMaterial->UpdateMaterialCode(
        R"(
            #include "../include/BasePassCommon.glsl"
            vec4 MaterialGetBaseColor(VS_Out vs_out)
            {
                return vec4(0, 0, 0, 1);
            }
            vec3 MaterialGetEmissive(VS_Out vs_out)
            {
                return vec3(0);
            }
            vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
            {
                return normalize(vs_out.TBN * vec3(0, 0, 1));
            }
            float MaterialGetRoughness(VS_Out vs_out)
            {
                return 0.5;
            }
            float MaterialGetMetallic(VS_Out vs_out)
            {
                return 0.5;
            }
            vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
            {
                return vec3(0);
            }
        )");
        WireframeMaterial->RasterizerState.FillMode = ERasterizerFillMode::FM_Wireframe;
    }

    UWorld::UWorld()
    {
        
    }

    void UWorld::InitWorld()
    {
        CreateMaterials();
    }

    void UWorld::InitializeActorsForPlay() 
    { 
        // TODO: 这里暂时是空，以后反序列化出来的Actor会在这里注册和初始化
    }

    void UWorld::BeginPlay()
    {
        bHasBegunPlay = true;

        std::shared_ptr<tinygltf::Model> Model = g_pAssetLoader->SyncReadGLTFModel(R"(D:\Nilou\Assets\Models\WaterBottle.gltf)");
        GLTFParseResult Mesh = GameStatics::ParseToStaticMeshes(*Model);
        for (auto &Texture : Mesh.Textures)
            FContentManager::GetContentManager().AddGlobalTexture(Texture->GetTextureName(), Texture);
        for (auto &Material : Mesh.Materials)
            FContentManager::GetContentManager().AddGlobalMaterial(Material->GetMaterialName(), Material);
        for (auto &StaticMesh : Mesh.StaticMeshes)
            FContentManager::GetContentManager().AddGlobalStaticMesh("WaterBottle.gltf", StaticMesh);
            // std::vector<FDynamicMeshVertex> OutVerts;
            // std::vector<uint32> OutIndices;
            // BuildCylinderVerts(vec3(36, 0, 0), vec3(0, 0, 1), vec3(0, 1, 0), vec3(1, 0, 0), 2.4, 36, 16, OutVerts, OutIndices);

        FTransform MeshTransform;
        MeshTransform.SetTranslation(glm::vec3(1, 1, 1));
        std::shared_ptr<AStaticMeshActor> StaticMeshActor = SpawnActor<AStaticMeshActor>(MeshTransform, "test mesh");
        StaticMeshActor->SetStaticMesh(Mesh.StaticMeshes[0]);
        
        std::shared_ptr<ASphereActor> SphereActor = SpawnActor<ASphereActor>(FTransform::Identity, "test sky sphere");
        SphereActor->SphereComponent->SetRelativeScale3D(vec3(4000));
        SphereActor->SphereComponent->SetMaterial(FContentManager::GetContentManager().GetGlobalMaterial("SkyAtmosphereMaterial"));
        std::shared_ptr<AArrowActor> ArrorActor = SpawnActor<AArrowActor>(FTransform::Identity, "test arrow");
        // StaticMeshActor->StaticMeshComponent->Material->RasterizerState.CullMode = CM_None;
        // StaticMeshActor->StaticMeshComponent->Material->RasterizerState.FillMode = FM_Solid;
        // StaticMeshActor->StaticMeshComponent->Material->DepthStencilState.DepthTest = CF_Always;
        // StaticMeshActor->StaticMeshComponent->Material->DepthStencilState.bEnableDepthWrite = false;

        FTransform CameraActorTransform;
        CameraActorTransform.SetTranslation(glm::vec3(0, -2, 0));
        CameraActorTransform.SetRotator(FRotator(0, 90, 0));
        std::shared_ptr<ACameraActor> CameraActor = SpawnActor<ACameraActor>(CameraActorTransform, "test camera");
        auto f = CameraActor->GetActorForwardVector();
        auto r = CameraActor->GetActorRightVector();
        auto u = CameraActor->GetActorUpVector();
        CameraActor->SetCameraResolution(ivec2(GetAppication()->GetConfiguration().screenWidth, GetAppication()->GetConfiguration().screenHeight));
        CameraActor->SetMoveSpeed(100);

        FTransform LightActorTransform;
        
        // LightActorTransform.SetTranslation(glm::vec3(0, 0, -2));
        // LightActorTransform.SetRotator(FRotator(90, 0, 0));
        // std::shared_ptr<ALightActor> LightActor2 = SpawnActor<ALightActor>(LightActorTransform, "test light2");
        
        // LightActorTransform.SetTranslation(glm::vec3(2, 2, 2));
        // LightActorTransform.SetRotator(FRotator(-45, -45, 0));
        // std::shared_ptr<ALightActor> LightActor = SpawnActor<ALightActor>(LightActorTransform, "test light");

        std::shared_ptr<ASkyAtmosphereActor> SkyAtmosphereActor = SpawnActor<ASkyAtmosphereActor>(FTransform::Identity, "test atmosphere");
        
        // LightActorTransform.SetTranslation(glm::vec3(10, 10, 10));
        LightActorTransform.SetRotator(FRotator(-45, 0, 0));
        std::shared_ptr<ALightActor> DirectionalLightActor = SpawnActor<ALightActor>(LightActorTransform, "test directional light");
        DirectionalLightActor->LightComponent->SetLightType(ELightType::LT_Directional);
        DirectionalLightActor->LightComponent->SetIntensity(10.f);

        
        std::shared_ptr<AGeoreferenceActor> GeoreferenceActor = SpawnActor<AGeoreferenceActor>(FTransform::Identity, "test georeference");
        GeoreferenceActor->SetGeoreferenceOrigin(84.77921, 45.65067, 604.42679);

        std::shared_ptr<ACesiumTilesetActor> TilesetActor = SpawnActor<ACesiumTilesetActor>(FTransform::Identity, "test tileset");
        TilesetActor->GetTilesetComponent()->SetURI(R"(E:\TuZiGou(20210608)\TuZiGou_3dtiles_cesiumlab\tileset.json)");
        TilesetActor->GetTilesetComponent()->SetMaxScreenSpaceError(32);
        // TilesetActor->GetTilesetComponent()->SetShowBoundingBox(true);

        std::shared_ptr<ALineBatchActor> LineBatchActor = SpawnActor<ALineBatchActor>(FTransform::Identity, "test linebatch");
        std::vector<FBatchedLine> lines;
        lines.emplace_back(dvec3(0, 0, 0), dvec3(1, 1, 1));
        LineBatchActor->LineBatchComponent->DrawLines(lines);

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