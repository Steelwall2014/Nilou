#include "Components/PrimitiveComponent.h"
// #include "PrimitiveSceneProxy.h"

#include "Math/Transform.h"
#include "UniformBuffer.h"
#include "Engine/World.h"

namespace nilou {

    static FVector3f GetInvNonUniformScaleFromMatrix(const FMatrix& LocalToWorld)
    {
        const FVector3f Scale3D(
            static_cast<float>(glm::length(FVector(LocalToWorld[0][0], LocalToWorld[0][1], LocalToWorld[0][2]))),
            static_cast<float>(glm::length(FVector(LocalToWorld[1][0], LocalToWorld[1][1], LocalToWorld[1][2]))),
            static_cast<float>(glm::length(FVector(LocalToWorld[2][0], LocalToWorld[2][1], LocalToWorld[2][2]))));
        FTransform3f Transform;
        return Transform.GetSafeScaleReciprocal(Scale3D);
    }
    
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
        UniformBuffer = RenderGraph::CreatePooledParameterBlock<shader::FPrimitiveUniformShaderParameters>(DebugActorName + "." + DebugComponentName + " UniformBuffer");
    }

    void FPrimitiveSceneProxy::UpdateUniformBuffer(RenderGraph& Graph)
    {
        UniformBuffer->LocalToWorld = FMatrix44f(LocalToWorld);
        UniformBuffer->WorldToLocal = FMatrix44f(glm::inverse(LocalToWorld));
        UniformBuffer->InvNonUniformScale = GetInvNonUniformScaleFromMatrix(LocalToWorld);
        Graph.UpdateParameterBlock(UniformBuffer.GetReference());
    }

}