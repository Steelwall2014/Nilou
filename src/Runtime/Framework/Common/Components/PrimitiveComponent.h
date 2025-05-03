#pragma once
#include "MeshBatch.h"
#include "Frustum.h"
#include "SceneComponent.h"
#include "SceneView.h"

// #include "PrimitiveComponent.generated.h"

namespace nilou {
    class AActor;
    class FScene;
    class FPrimitiveSceneProxy;
    class FPrimitiveSceneInfo;

    enum EReflectionProbeBlendMode 
    {
        RPBM_Off,    // Reflection probe blending is disabled. Only the skybox will be used for reflection
        RPBM_BlendProbes,    // Blends only adjacent probes and ignores the skybox. 
        RPBM_BlendProbesAndSkybox,   // Works like Blend Probes but also allows the skybox to be used in the blending.
        RPBM_Simple, // Disables blending between probes when there are two overlapping reflection probe volumes.
    };

    class NCLASS UPrimitiveComponent : public USceneComponent
    {
        GENERATED_BODY()
    public:

        UPrimitiveComponent()
            : SceneProxy(nullptr)
            , bCastShadow(true)
        { }

        FPrimitiveSceneProxy *GetSceneProxy() const { return SceneProxy; }

        virtual FPrimitiveSceneProxy *CreateSceneProxy() { return nullptr; }

        virtual glm::mat4 GetRenderMatrix() const { return GetComponentToWorld().ToMatrix(); }

        virtual void CreateRenderState() override;

        virtual void DestroyRenderState() override;

        virtual void SendRenderTransform() override;

        void SetCastShadow(bool bInCastShadow) { bCastShadow = bInCastShadow; MarkRenderStateDirty(); }

        bool GetCastShadow() const { return bCastShadow; }

        void SetReflectionProbeBlendMode(EReflectionProbeBlendMode NewBlendMode) { ReflectionProbeBlendMode = NewBlendMode; MarkRenderStateDirty(); }

        EReflectionProbeBlendMode GetReflectionProbeBlendMode() const { return ReflectionProbeBlendMode; }

    protected:

        bool bCastShadow;

        EReflectionProbeBlendMode ReflectionProbeBlendMode = RPBM_BlendProbes;

        FPrimitiveSceneProxy *SceneProxy;

        friend class FPrimitiveSceneProxy;
        friend class FScene;
        
    };

    BEGIN_UNIFORM_BUFFER_STRUCT(FPrimitiveUniformShaderParameters)
        SHADER_PARAMETER(FMatrix44f, LocalToWorld)
        SHADER_PARAMETER(FMatrix44f, ModelToLocal)
    END_UNIFORM_BUFFER_STRUCT()

    class FPrimitiveSceneProxy
    {
        friend class FScene;
        friend class FDeferredShadingSceneRenderer;
    public:
        
        FPrimitiveSceneProxy(UPrimitiveComponent *Primitive);

        virtual void GetDynamicMeshElements(const std::vector<FSceneView>& Views, uint32 VisibilityMap, FMeshElementCollector &Collector) { };
        
        /**
        *	Called when the rendering thread adds the proxy to the scene.
        *	This function allows for generating renderer-side resources.
        *	Called in the rendering thread.
        */
        virtual void CreateRenderThreadResources() {}

        /**
        *	Called when the rendering thread removes the proxy from the scene.
        *	This function allows for removing renderer-side resources.
        *	Called in the rendering thread.
        */
        virtual void DestroyRenderThreadResources() {}

        virtual ~FPrimitiveSceneProxy()
        {
            DestroyRenderThreadResources();
        }

        FPrimitiveSceneInfo *GetPrimitiveSceneInfo() const { return PrimitiveSceneInfo; }

        dmat4 GetLocalToWorld() const { return LocalToWorld; }

        FBoxSphereBounds GetBounds() const { return Bounds; }

        RHIBuffer *GetUniformBuffer() const { return UniformBuffer->GetRHI(); }

        void CreateUniformBuffer();

        void UpdateUniformBuffer();

    protected:

        bool bCastShadow;

        FPrimitiveSceneInfo *PrimitiveSceneInfo;
        FScene *Scene;

        TRDGUniformBufferRef<FPrimitiveUniformShaderParameters> UniformBuffer;

        EReflectionProbeBlendMode ReflectionProbeBlendMode;

    private:
    
	    /** The primitive's local to world transform. */
        FMatrix LocalToWorld;

	    /** The primitive's bounds. */
        FBoxSphereBounds Bounds;

	    /** The primitive's local space bounds. */
        FBoxSphereBounds LocalBounds;

        std::string DebugComponentName;
        std::string DebugActorName;
        std::string DebugResourceName;
        std::string DebugLevelName;

    };
}