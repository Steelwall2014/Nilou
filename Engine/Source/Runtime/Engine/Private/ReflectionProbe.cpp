#include "GameFramework/ReflectionProbe.h"
#include "Components/SceneCaptureComponent.h"
#include "Components/ReflectionProbeComponent.h"
#include "Engine/TextureCube.h"
#include "Engine/TextureRenderTarget.h"
#include "Materials/Material.h"
#include "Components/SphereComponent.h"

namespace nilou {

    AReflectionProbe::AReflectionProbe()
    { 
        ReflectionProbeComponent = CreateComponent<UReflectionProbeComponent>(this, "ReflectionProbeComponent"); 
        ReflectionProbeComponent->AttachToComponent(GetRootComponent());

        // Always keep false, we will take over the capture in this class
        ReflectionProbeComponent->bCaptureEveryFrame = false;
        ReflectionProbeComponent->bCaptureOnMovement = false;
        ReflectionProbeComponent->CaptureSceneDeferred();

        {
            FImage Image = FImage(1024, 1024, EPixelFormat::PF_R16G16B16A16F, EImageType::IT_ImageCube);
            EnvironmentTexture = NewObject<UTextureRenderTargetCube>(this, "Test EnvironmentTexture");
            EnvironmentTexture->ImageData = Image;
            EnvironmentTexture->UpdateResource();
            ReflectionProbeComponent->TextureTarget = EnvironmentTexture;
        }

        {
            FImage Image = FImage(16, 16, EPixelFormat::PF_R16G16B16A16F, EImageType::IT_ImageCube);
            IrradianceTexture = NewObject<UTextureCube>(this, "Test IrradianceTexture");
            IrradianceTexture->ImageData = Image;
            IrradianceTexture->UpdateResource();
        }

        {
            FImage Image = FImage(1024, 1024, EPixelFormat::PF_R16G16B16A16F, EImageType::IT_ImageCube);
            PrefilteredTexture = NewObject<UTextureCube>(this, "Test PrefilteredTexture");
            PrefilteredTexture->ImageData = Image;
            PrefilteredTexture->NumMips = 5;
            PrefilteredTexture->UpdateResource();
        }
        ReflectionProbeComponent->TextureTarget = EnvironmentTexture;
        ReflectionProbeComponent->IrradianceTexture = IrradianceTexture;
        ReflectionProbeComponent->PrefilteredTexture = PrefilteredTexture;
        
        DebugMat = NewObject<UMaterial>(this, "ReflectionProbeDebugMaterial");
        DebugMat->InitializeResources();
        DebugMat->SetShadingModel(EShadingModel::SM_Unlit);
        FRasterizerStateInitializer RasterizerState;
        RasterizerState.CullMode = ERasterizerCullMode::CM_None;
        DebugMat->SetRasterizerState(RasterizerState);
        DebugMat->SetShaderFileVirtualPath("/Shaders/Private/Materials/ReflectionProbeDebug_Mat.slang");
        DebugMat->SetTextureParameterValue("Cube", PrefilteredTexture);

        // DebugSphere = CreateComponent<USphereComponent>(this); 
        // DebugSphere->SetMaterial(DebugMat.get());
        // DebugSphere->SetRelativeScale3D(dvec3(70));
        // DebugSphere->AttachToComponent(GetRootComponent());
        // DebugSphere->SetCastShadow(false);
        // ReflectionProbeComponent->HideComponent(DebugSphere.get());
    }

}