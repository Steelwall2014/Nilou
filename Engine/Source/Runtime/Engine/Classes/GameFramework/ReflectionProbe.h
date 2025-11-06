#pragma once

#include "GameFramework/Actor.h"

namespace nilou {

    class NCLASS AReflectionProbe : public AActor
    {
		GENERATED_BODY()
    public:
        AReflectionProbe();

        class UReflectionProbeComponent* ReflectionProbeComponent;

        class UTextureRenderTargetCube* EnvironmentTexture;

        class UTextureCube* IrradianceTexture;

        class UTextureCube* PrefilteredTexture;

        class UMaterial* DebugMat;

        class USphereComponent* DebugSphere;
    };

}