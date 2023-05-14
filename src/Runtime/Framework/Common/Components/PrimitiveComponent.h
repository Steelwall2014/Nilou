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
        GENERATE_BODY()
    public:

        UPrimitiveComponent()
            : SceneProxy(nullptr)
            , bCastShadow(true)
        { }

        FPrimitiveSceneProxy *SceneProxy;

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
        
    };

    BEGIN_UNIFORM_BUFFER_STRUCT(FPrimitiveShaderParameters)
        SHADER_PARAMETER(dmat4, LocalToWorld)
        SHADER_PARAMETER(dmat4, ModelToLocal)
    END_UNIFORM_BUFFER_STRUCT()

    class FPrimitiveSceneProxy
    {
        friend class FScene;
        friend class FDefferedShadingSceneRenderer;
    public:
        
        FPrimitiveSceneProxy(UPrimitiveComponent *Primitive, const std::string &InName = "");

        virtual void GetDynamicMeshElements(const std::vector<FSceneView*> &Views, uint32 VisibilityMap, FMeshElementCollector &Collector) { };
        // virtual void GetDynamicMeshElement(FMeshBatch &OutMeshBatch, const FSceneView &View) { };
    
        virtual void SetTransform(const dmat4 &InLocalToWorld, const FBoundingBox &InBounds);

        virtual void CreateRenderThreadResources();

        virtual void DestroyRenderThreadResources();

        virtual ~FPrimitiveSceneProxy()
        {
            DestroyRenderThreadResources();
        }

        FPrimitiveSceneInfo *GetPrimitiveSceneInfo() const { return PrimitiveSceneInfo; }

        std::string GetName() const { return Name; }

        dmat4 GetLocalToWorld() const { return LocalToWorld; }

        FBoundingBox GetBounds() const { return Bounds; }

        FUniformBuffer *GetUniformBuffer() const { return PrimitiveUniformBuffer.get(); }

        void UpdateUniformBuffer();

    protected:

        bool bCastShadow;

        std::string Name;

        FPrimitiveSceneInfo *PrimitiveSceneInfo;
        FScene *Scene;

        TUniformBufferRef<FPrimitiveShaderParameters> PrimitiveUniformBuffer;

        EReflectionProbeBlendMode ReflectionProbeBlendMode;

    private:
    
        dmat4 LocalToWorld;
        FBoundingBox Bounds;

    };
}