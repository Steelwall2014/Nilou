#include "ReflectionProbe.h"
#include "Common/Components/SceneCaptureComponent.h"
#include "Common/Components/ReflectionProbeComponent.h"
#include "Common/ContentManager.h"
#include "TextureCube.h"

#include "Material.h"
#include "Common/Components/SphereComponent.h"

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
            EnvironmentTexture = std::make_shared<UTextureRenderTargetCube>();
            EnvironmentTexture->Rename("Test EnvironmentTexture");
            EnvironmentTexture->ImageData = Image;
            EnvironmentTexture->UpdateResource();
            ReflectionProbeComponent->TextureTarget = EnvironmentTexture.get();
        }

        {
            FImage Image = FImage(16, 16, EPixelFormat::PF_R16G16B16A16F, EImageType::IT_ImageCube);
            IrradianceTexture = std::make_shared<UTextureCube>();
            IrradianceTexture->Rename("Test IrradianceTexture");
            IrradianceTexture->ImageData = Image;
            IrradianceTexture->UpdateResource();
        }

        {
            FImage Image = FImage(1024, 1024, EPixelFormat::PF_R16G16B16A16F, EImageType::IT_ImageCube);
            PrefilteredTexture = std::make_shared<UTextureCube>();
            PrefilteredTexture->Rename("Test PrefilteredTexture");
            PrefilteredTexture->ImageData = Image;
            PrefilteredTexture->NumMips = 5;
            PrefilteredTexture->UpdateResource();
        }
        ReflectionProbeComponent->TextureTarget = EnvironmentTexture.get();
        ReflectionProbeComponent->IrradianceTexture = IrradianceTexture.get();
        ReflectionProbeComponent->PrefilteredTexture = PrefilteredTexture.get();

        DebugMat = std::make_shared<UMaterial>();
        DebugMat->InitializeResources();
        DebugMat->SetShadingModel(EShadingModel::SM_Unlit);
        FRasterizerStateInitializer RasterizerState;
        RasterizerState.CullMode = ERasterizerCullMode::CM_None;
        DebugMat->SetRasterizerState(RasterizerState);
        DebugMat->UpdateCode(R"(
#include "../include/Common.glsl"
#include "../include/BasePassCommon.glsl"
#include "../include/functions.glsl"
layout(set=SET_INDEX, binding=BINDING_INDEX) uniform samplerCube Cube;

vec4 MaterialGetBaseColor(VS_Out vs_out)
{
    return mytextureCubeLod(Cube, normalize(vs_out.RelativeWorldPosition), 0);
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
        DebugMat->SetTextureParameterValue("Cube", PrefilteredTexture.get());

        // DebugSphere = CreateComponent<USphereComponent>(this); 
        // DebugSphere->SetMaterial(DebugMat.get());
        // DebugSphere->SetRelativeScale3D(dvec3(70));
        // DebugSphere->AttachToComponent(GetRootComponent());
        // DebugSphere->SetCastShadow(false);
        // ReflectionProbeComponent->HideComponent(DebugSphere.get());
    }

}