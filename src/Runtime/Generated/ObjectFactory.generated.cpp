#include "D:/Nilou/src/Runtime/Framework/Common/Actor/Actor.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Actor/ArrowActor.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Actor/CameraActor.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Actor/Cesium3DTilesetActor.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Actor/GeoreferenceActor.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Actor/LightActor.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Actor/LineBatchActor.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Actor/ReflectionProbe.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Actor/SkyAtmosphereActor.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Actor/SphereActor.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Actor/StaticMeshActor.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Actor/VirtualHeightfieldMeshActor.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Components/ActorComponent.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Components/ArrowComponent.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Components/CameraComponent.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Components/Cesium3DTilesetComponent.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Components/FourierTransformOcean.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Components/LightComponent.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Components/LineBatchComponent.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Components/MeshComponent.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Components/PrimitiveComponent.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Components/ReflectionProbeComponent.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Components/SceneCaptureComponent.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Components/SceneCaptureComponent.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Components/SceneCaptureComponent.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Components/SceneComponent.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Components/SkyAtmosphereComponent.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Components/SphereComponent.h"
#include "D:/Nilou/src/Runtime/Framework/Common/Components/VirtualHeightfieldMeshComponent.h"
#include "D:/Nilou/src/Runtime/Framework/Common/CoreUObject/Object.h"
#include "D:/Nilou/src/Runtime/Rendering/Material.h"
#include "D:/Nilou/src/Runtime/Rendering/Material.h"
#include "D:/Nilou/src/Runtime/Rendering/StaticMeshResources.h"
#include "D:/Nilou/src/Runtime/Rendering/Texture.h"
#include "D:/Nilou/src/Runtime/Rendering/Texture2D.h"
#include "D:/Nilou/src/Runtime/Rendering/Texture2DArray.h"
#include "D:/Nilou/src/Runtime/Rendering/Texture3D.h"
#include "D:/Nilou/src/Runtime/Rendering/TextureCube.h"
#include "D:/Nilou/src/Runtime/Rendering/TextureRenderTarget.h"
#include "D:/Nilou/src/Runtime/Rendering/TextureRenderTarget.h"
#include "D:/Nilou/src/Runtime/Rendering/TextureRenderTarget.h"
#include "D:/Nilou/src/Runtime/Rendering/VirtualTexture2D.h"
using namespace nilou;
FObjectFactory::FObjectFactory()
{
    FunctionMap["AActor"] = &AActor::CreateDefaultObject;
    FunctionMap["AArrowActor"] = &AArrowActor::CreateDefaultObject;
    FunctionMap["ACameraActor"] = &ACameraActor::CreateDefaultObject;
    FunctionMap["ACesiumTilesetActor"] = &ACesiumTilesetActor::CreateDefaultObject;
    FunctionMap["AFFTOceanActor"] = &AFFTOceanActor::CreateDefaultObject;
    FunctionMap["AGeoreferenceActor"] = &AGeoreferenceActor::CreateDefaultObject;
    FunctionMap["ALightActor"] = &ALightActor::CreateDefaultObject;
    FunctionMap["ALineBatchActor"] = &ALineBatchActor::CreateDefaultObject;
    FunctionMap["AReflectionProbe"] = &AReflectionProbe::CreateDefaultObject;
    FunctionMap["ASkyAtmosphereActor"] = &ASkyAtmosphereActor::CreateDefaultObject;
    FunctionMap["ASphereActor"] = &ASphereActor::CreateDefaultObject;
    FunctionMap["AStaticMeshActor"] = &AStaticMeshActor::CreateDefaultObject;
    FunctionMap["AVirtualHeightfieldMeshActor"] = &AVirtualHeightfieldMeshActor::CreateDefaultObject;
    FunctionMap["UActorComponent"] = &UActorComponent::CreateDefaultObject;
    FunctionMap["UArrowComponent"] = &UArrowComponent::CreateDefaultObject;
    FunctionMap["UCameraComponent"] = &UCameraComponent::CreateDefaultObject;
    FunctionMap["UCesium3DTilesetComponent"] = &UCesium3DTilesetComponent::CreateDefaultObject;
    FunctionMap["UFourierTransformOceanComponent"] = &UFourierTransformOceanComponent::CreateDefaultObject;
    FunctionMap["ULightComponent"] = &ULightComponent::CreateDefaultObject;
    FunctionMap["ULineBatchComponent"] = &ULineBatchComponent::CreateDefaultObject;
    FunctionMap["UStaticMeshComponent"] = &UStaticMeshComponent::CreateDefaultObject;
    FunctionMap["UPrimitiveComponent"] = &UPrimitiveComponent::CreateDefaultObject;
    FunctionMap["UReflectionProbeComponent"] = &UReflectionProbeComponent::CreateDefaultObject;
    FunctionMap["USceneCaptureComponent"] = &USceneCaptureComponent::CreateDefaultObject;
    FunctionMap["USceneCaptureComponent2D"] = &USceneCaptureComponent2D::CreateDefaultObject;
    FunctionMap["USceneCaptureComponentCube"] = &USceneCaptureComponentCube::CreateDefaultObject;
    FunctionMap["USceneComponent"] = &USceneComponent::CreateDefaultObject;
    FunctionMap["USkyAtmosphereComponent"] = &USkyAtmosphereComponent::CreateDefaultObject;
    FunctionMap["USphereComponent"] = &USphereComponent::CreateDefaultObject;
    FunctionMap["UVirtualHeightfieldMeshComponent"] = &UVirtualHeightfieldMeshComponent::CreateDefaultObject;
    FunctionMap["UObject"] = &UObject::CreateDefaultObject;
    FunctionMap["UMaterial"] = &UMaterial::CreateDefaultObject;
    FunctionMap["UMaterialInstance"] = &UMaterialInstance::CreateDefaultObject;
    FunctionMap["UStaticMesh"] = &UStaticMesh::CreateDefaultObject;
    FunctionMap["UTexture"] = &UTexture::CreateDefaultObject;
    FunctionMap["UTexture2D"] = &UTexture2D::CreateDefaultObject;
    FunctionMap["UTexture2DArray"] = &UTexture2DArray::CreateDefaultObject;
    FunctionMap["UTexture3D"] = &UTexture3D::CreateDefaultObject;
    FunctionMap["UTextureCube"] = &UTextureCube::CreateDefaultObject;
    FunctionMap["UTextureRenderTarget"] = &UTextureRenderTarget::CreateDefaultObject;
    FunctionMap["UTextureRenderTarget2D"] = &UTextureRenderTarget2D::CreateDefaultObject;
    FunctionMap["UTextureRenderTargetCube"] = &UTextureRenderTargetCube::CreateDefaultObject;
    FunctionMap["UVirtualTexture"] = &UVirtualTexture::CreateDefaultObject;
}