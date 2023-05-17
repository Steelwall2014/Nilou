#pragma once

#include "Actor.h"

namespace nilou {

    class NCLASS AReflectionProbe : public AActor
    {
		GENERATED_BODY()
    public:
        AReflectionProbe();

        std::shared_ptr<class UReflectionProbeComponent> ReflectionProbeComponent;

        std::shared_ptr<class UTextureRenderTargetCube> EnvironmentTexture;

        std::shared_ptr<class UTextureCube> IrradianceTexture;

        std::shared_ptr<class UTextureCube> PrefilteredTexture;

        std::shared_ptr<class UMaterial> DebugMat;

        std::shared_ptr<class USphereComponent> DebugSphere;
    };

}