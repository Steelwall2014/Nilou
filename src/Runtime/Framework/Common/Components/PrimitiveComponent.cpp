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
        SceneProxy->SetTransform(GetRenderMatrix(), GetBounds());
        // if(World && World->Scene)
        // {
        //     World->Scene->UpdatePrimitiveTransform(this);
        // }

        USceneComponent::SendRenderTransform();
    }

    FPrimitiveSceneProxy::FPrimitiveSceneProxy(UPrimitiveComponent *Primitive, const std::string &InName)
        : Scene(nullptr)
        , PrimitiveSceneInfo(nullptr)
    {
        Name = InName;
        bCastShadow = Primitive->GetCastShadow();
        Primitive->SceneProxy = this;
        PrimitiveUniformBuffer = CreateUniformBuffer<FPrimitiveShaderParameters>();
        SetTransform(Primitive->GetRenderMatrix(), Primitive->GetBounds());
    }

    void FPrimitiveSceneProxy::SetTransform(const glm::dmat4 &InLocalToWorld, const FBoundingBox &InBounds)
    {
        Bounds = InBounds;
        LocalToWorld = InLocalToWorld;
        PrimitiveUniformBuffer->Data.LocalToWorld = LocalToWorld;
        if (PrimitiveSceneInfo)
            PrimitiveSceneInfo->SetNeedsUniformBufferUpdate(true);
    }

    void FPrimitiveSceneProxy::CreateRenderThreadResources()
    {
        assert(IsInRenderingThread());
        PrimitiveUniformBuffer->InitResource();
        if (PrimitiveSceneInfo)
            PrimitiveSceneInfo->SetNeedsUniformBufferUpdate(false);
    }

    void FPrimitiveSceneProxy::DestroyRenderThreadResources()
    {
        assert(IsInRenderingThread());
        PrimitiveUniformBuffer->ReleaseResource();
    }

    void FPrimitiveSceneProxy::UpdateUniformBuffer()
    {
        ENQUEUE_RENDER_COMMAND(FPrimitiveSceneProxy_UpdateUniformBuffer)(
            [this](FDynamicRHI *DynamicRHI)
            {
                PrimitiveUniformBuffer->UpdateUniformBuffer();
            });
    }
}