#include "../../src/Runtime/Framework/Common/Actor/Actor.h"
#include "../../src/Runtime/Framework/Common/Actor/ArrowActor.h"
#include "../../src/Runtime/Framework/Common/Actor/CameraActor.h"
#include "../../src/Runtime/Framework/Common/Actor/Cesium3DTilesetActor.h"
#include "../../src/Runtime/Framework/Common/Actor/GeoreferenceActor.h"
#include "../../src/Runtime/Framework/Common/Actor/LightActor.h"
#include "../../src/Runtime/Framework/Common/Actor/LineBatchActor.h"
#include "../../src/Runtime/Framework/Common/Actor/SkyAtmosphereActor.h"
#include "../../src/Runtime/Framework/Common/Actor/SphereActor.h"
#include "../../src/Runtime/Framework/Common/Actor/StaticMeshActor.h"
#include "../../src/Runtime/Framework/Common/Actor/VirtualHeightfieldMeshActor.h"
#include "../../src/Runtime/Framework/Common/Components/ActorComponent.h"
#include "../../src/Runtime/Framework/Common/Components/ArrowComponent.h"
#include "../../src/Runtime/Framework/Common/Components/CameraComponent.h"
#include "../../src/Runtime/Framework/Common/Components/Cesium3DTilesetComponent.h"
#include "../../src/Runtime/Framework/Common/Components/LightComponent.h"
#include "../../src/Runtime/Framework/Common/Components/LineBatchComponent.h"
#include "../../src/Runtime/Framework/Common/Components/MeshComponent.h"
#include "../../src/Runtime/Framework/Common/Components/PrimitiveComponent.h"
#include "../../src/Runtime/Framework/Common/Components/SceneComponent.h"
#include "../../src/Runtime/Framework/Common/Components/SkyAtmosphereComponent.h"
#include "../../src/Runtime/Framework/Common/Components/SphereComponent.h"
#include "../../src/Runtime/Framework/Common/Components/VirtualHeightfieldMeshComponent.h"
#include "../../src/Runtime/Framework/Common/CoreUObject/Object.h"
#include "../../src/Runtime/Framework/Common/StaticMeshResources.h"
#include "../../src/Runtime/Rendering/Material.h"
#include "../../src/Runtime/Rendering/Material.h"
#include "../../src/Runtime/Rendering/Texture.h"
using namespace nilou;
FObjectFactory::FObjectFactory()
{
    FunctionMap["AActor"] = &AActor::CreateDefaultObject;
    FunctionMap["AArrowActor"] = &AArrowActor::CreateDefaultObject;
    FunctionMap["ACameraActor"] = &ACameraActor::CreateDefaultObject;
    FunctionMap["ACesiumTilesetActor"] = &ACesiumTilesetActor::CreateDefaultObject;
    FunctionMap["AGeoreferenceActor"] = &AGeoreferenceActor::CreateDefaultObject;
    FunctionMap["ALightActor"] = &ALightActor::CreateDefaultObject;
    FunctionMap["ALineBatchActor"] = &ALineBatchActor::CreateDefaultObject;
    FunctionMap["ASkyAtmosphereActor"] = &ASkyAtmosphereActor::CreateDefaultObject;
    FunctionMap["ASphereActor"] = &ASphereActor::CreateDefaultObject;
    FunctionMap["AStaticMeshActor"] = &AStaticMeshActor::CreateDefaultObject;
    FunctionMap["AVirtualHeightfieldMeshActor"] = &AVirtualHeightfieldMeshActor::CreateDefaultObject;
    FunctionMap["UActorComponent"] = &UActorComponent::CreateDefaultObject;
    FunctionMap["UArrowComponent"] = &UArrowComponent::CreateDefaultObject;
    FunctionMap["UCameraComponent"] = &UCameraComponent::CreateDefaultObject;
    FunctionMap["UCesium3DTilesetComponent"] = &UCesium3DTilesetComponent::CreateDefaultObject;
    FunctionMap["ULightComponent"] = &ULightComponent::CreateDefaultObject;
    FunctionMap["ULineBatchComponent"] = &ULineBatchComponent::CreateDefaultObject;
    FunctionMap["UStaticMeshComponent"] = &UStaticMeshComponent::CreateDefaultObject;
    FunctionMap["UPrimitiveComponent"] = &UPrimitiveComponent::CreateDefaultObject;
    FunctionMap["USceneComponent"] = &USceneComponent::CreateDefaultObject;
    FunctionMap["USkyAtmosphereComponent"] = &USkyAtmosphereComponent::CreateDefaultObject;
    FunctionMap["USphereComponent"] = &USphereComponent::CreateDefaultObject;
    FunctionMap["UVirtualHeightfieldMeshComponent"] = &UVirtualHeightfieldMeshComponent::CreateDefaultObject;
    FunctionMap["UObject"] = &UObject::CreateDefaultObject;
    FunctionMap["UStaticMesh"] = &UStaticMesh::CreateDefaultObject;
    FunctionMap["UMaterial"] = &UMaterial::CreateDefaultObject;
    FunctionMap["UMaterialInstance"] = &UMaterialInstance::CreateDefaultObject;
    FunctionMap["UTexture"] = &UTexture::CreateDefaultObject;
}