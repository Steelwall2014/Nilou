#include "PrimitiveComponent.h"
// #include "PrimitiveSceneProxy.h"

#include "UniformBuffer.h"
#include "Common/World.h"

namespace nilou {
    
    void UPrimitiveComponent::CreateRenderState()
    {
        USceneComponent::CreateRenderState();
        UWorld* World = GetWorld();
        if(World && World->Scene)
        {
            World->Scene->AddPrimitive(this);
        }
    }

    void UPrimitiveComponent::DestroyRenderState()
    {
        UWorld* World = GetWorld();
        if(World && World->Scene)
        {
            World->Scene->RemovePrimitive(this);
        }

        USceneComponent::DestroyRenderState();
    }

    void UPrimitiveComponent::SendRenderTransform()
    {
        UpdateBounds();

        GetWorld()->Scene->UpdatePrimitiveTransform(this);

        USceneComponent::SendRenderTransform();
    }

    FPrimitiveSceneProxy::FPrimitiveSceneProxy(UPrimitiveComponent *Primitive)
        : Scene(nullptr)
        , PrimitiveSceneInfo(nullptr)
        , ReflectionProbeBlendMode(Primitive->GetReflectionProbeBlendMode())
        , DebugComponentName(Primitive->GetName())
        , bCastShadow(Primitive->GetCastShadow())
    {
        Primitive->SceneProxy = this;
        LocalToWorld = Primitive->GetRenderMatrix();
        Bounds = Primitive->GetBounds();
    }

    void FPrimitiveSceneProxy::CreateUniformBuffer()
    {
        Ncheck(IsInRenderingThread());
        UniformBuffer = RenderGraph::CreateExternalUniformBuffer<FPrimitiveUniformShaderParameters>(DebugActorName + "." + DebugComponentName + " UniformBuffer", nullptr);
    }

    void FPrimitiveSceneProxy::UpdateUniformBuffer(RenderGraph& Graph)
    {
        FPrimitiveUniformShaderParameters Data;
        Data.LocalToWorld = LocalToWorld;
        Data.ModelToLocal = glm::inverse(LocalToWorld);
        Graph.QueueBufferUpload(UniformBuffer, &Data, sizeof(Data));
    }

}