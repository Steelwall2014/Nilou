#pragma once

#include "Actor.h"

namespace nilou {

    UCLASS()
    class AReflectionProbe : public AActor
    {
		GENERATE_CLASS_INFO()
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