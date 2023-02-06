#pragma once
#include "Common/MeshBatch.h"
#include "Frustum.h"
#include "SceneComponent.h"
#include "SceneView.h"

// #include "PrimitiveComponent.generated.h"

namespace nilou {
    class AActor;
    class FScene;
    class FPrimitiveSceneProxy;
    class FPrimitiveSceneInfo;

    UCLASS()
    class UPrimitiveComponent : public USceneComponent
    {
        GENERATE_CLASS_INFO()
    public:

        UPrimitiveComponent(AActor *InOwner = nullptr)
            : USceneComponent(InOwner)
            , SceneProxy(nullptr)
        { }

        FPrimitiveSceneProxy *SceneProxy;

        virtual FPrimitiveSceneProxy *CreateSceneProxy() { return nullptr; }

        virtual glm::mat4 GetRenderMatrix() const { return GetComponentToWorld().ToMatrix(); }

        virtual void CreateRenderState() override;

        virtual void DestroyRenderState() override;

        virtual void SendRenderTransform() override;

        bool bCastShadow;

        bool bNeverDistanceCull;
    };

    BEGIN_UNIFORM_BUFFER_STRUCT(FPrimitiveShaderParameters)
        SHADER_PARAMETER(mat4, LocalToWorld)
    END_UNIFORM_BUFFER_STRUCT()

    class FPrimitiveSceneProxy
    {
        friend class FScene;
    public:
        
        FPrimitiveSceneProxy(UPrimitiveComponent *Primitive, const std::string &InName = "");

        // virtual void GetDynamicMeshElements(const std::vector<class FSceneView *> &Views, uint32 VisibilityMap, std::vector<std::vector<FMeshBatch>> &OutPerViewMeshBatches, int32 LODIndex) { };
        virtual void GetDynamicMeshElement(FMeshBatch &OutMeshBatch, const FSceneView &View) { };
    
        virtual void SetTransform(const glm::mat4 &InLocalToWorld, const FBoundingBox &InBounds);

        virtual void CreateRenderThreadResources();

        virtual void DestroyRenderThreadResources();

        FPrimitiveSceneInfo *GetPrimitiveSceneInfo() const { return PrimitiveSceneInfo; }

        std::string GetName() { return Name; }

        FBoundingBox GetBounds() { return Bounds; }

        FUniformBuffer *GetUniformBuffer() { return PrimitiveUniformBuffer.get(); }

        void UpdateUniformBuffer() { PrimitiveUniformBuffer->UpdateUniformBuffer(); }

    protected:

        bool bRenderInDepthPass;
        bool bRenderInMainPass;

        std::string Name;
        glm::mat4 LocalToWorld;
        FBoundingBox Bounds;

        FPrimitiveSceneInfo *PrimitiveSceneInfo;
        FScene *Scene;

        TUniformBufferRef<FPrimitiveShaderParameters> PrimitiveUniformBuffer;
    };
}