#include "World.h"
#include "Common/BaseApplication.h"
#include "Common/Transform.h"

#include "Common/StaticMeshResources.h"
#include "Common/AssetLoader.h"
#include "Parser/GLTFParser.h"
#include "Common/Components/MeshComponent.h"
#include "Common/Components/ArrowComponent.h"
#include "Common/PrimitiveUtils.h"
#include "Common/ContentManager.h"

#include "Common/Actor/Actor.h"
#include "Common/Actor/StaticMeshActor.h"
#include "Common/Actor/CameraActor.h"
#include "Common/Actor/LightActor.h"
#include "Common/Actor/SkyAtmosphereActor.h"

namespace nilou {

    UWorld::UWorld()
    {
        
    }

    void UWorld::BeginPlay()
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

        std::shared_ptr<tinygltf::Model> Model = g_pAssetLoader->SyncReadGLTFModel(R"(D:\Nilou\Assets\Models\WaterBottle.gltf)");
        std::vector<std::shared_ptr<UStaticMesh>> Mesh = g_pGLTFParser->ParseToStaticMeshes(*Model);

            // std::vector<FDynamicMeshVertex> OutVerts;
            // std::vector<uint32> OutIndices;
            // BuildCylinderVerts(vec3(36, 0, 0), vec3(0, 0, 1), vec3(0, 1, 0), vec3(1, 0, 0), 2.4, 36, 16, OutVerts, OutIndices);

        FTransform MeshTransform;
        MeshTransform.SetTranslation(glm::vec3(1, 1, 1));
        // std::shared_ptr<AStaticMeshActor> StaticMeshActor = SpawnActor<AStaticMeshActor>(MeshTransform, "test mesh");
        // StaticMeshActor->SetStaticMesh(Mesh[0]);

        std::shared_ptr<AActor> ArrorActor = SpawnActor<AActor>(FTransform::Identity, "test arrow");
        std::shared_ptr<UArrowComponent> ArrowCompoennt = std::make_shared<UArrowComponent>(ArrorActor.get());
        ArrowCompoennt->RegisterComponentWithWorld(this);
        ArrorActor->SetRootComponent(ArrowCompoennt);
        // StaticMeshActor->StaticMeshComponent->Material->RasterizerState.CullMode = CM_None;
        // StaticMeshActor->StaticMeshComponent->Material->RasterizerState.FillMode = FM_Solid;
        // StaticMeshActor->StaticMeshComponent->Material->DepthStencilState.DepthTest = CF_Always;
        // StaticMeshActor->StaticMeshComponent->Material->DepthStencilState.bEnableDepthWrite = false;

        FTransform CameraActorTransform;
        CameraActorTransform.SetTranslation(glm::vec3(1, 2, 0));
        CameraActorTransform.SetRotator(FRotator(0, -90, 0));
        std::shared_ptr<ACameraActor> CameraActor = SpawnActor<ACameraActor>(CameraActorTransform, "test camera");
        auto f = CameraActor->GetActorForwardVector();
        auto r = CameraActor->GetActorRightVector();
        auto u = CameraActor->GetActorUpVector();
        CameraActor->SetCameraResolution(ivec2(g_pApp->GetConfiguration().screenWidth, g_pApp->GetConfiguration().screenHeight));


        FTransform LightActorTransform;
        LightActorTransform.SetTranslation(glm::vec3(1, 10, 0));
        LightActorTransform.SetRotator(FRotator(0, -90, 0));
        std::shared_ptr<ALightActor> LightActor = SpawnActor<ALightActor>(LightActorTransform, "test light");

        std::shared_ptr<ASkyAtmosphereActor> SkyAtmosphereActor = SpawnActor<ASkyAtmosphereActor>(FTransform::Identity, "test atmosphere");
    }

    void UWorld::Tick(double DeltaTime)
    {
        for (std::shared_ptr<AActor> Actor : Actors)
        {
            Actor->Tick(DeltaTime);
        }
    }

    template<class ActorClass>
    std::shared_ptr<ActorClass> UWorld::SpawnActor(const FTransform &ActorTransform, const std::string &ActorName)
    {
        static_assert(TIsDerivedFrom<ActorClass, AActor>::Value, "'ActorClass' template parameter to SpawnActor must be derived from AActor");
        std::shared_ptr<ActorClass> Actor = std::make_shared<ActorClass>();
        Actor->SetOwnedWorld(this);
        Actor->SetActorName(ActorName);
        Actor->SetActorLocation(ActorTransform.GetTranslation());
        Actor->SetActorRotation(ActorTransform.GetRotation());
        Actor->RegisterAllComponents();
        Actors.push_back(Actor);
        return Actor;
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