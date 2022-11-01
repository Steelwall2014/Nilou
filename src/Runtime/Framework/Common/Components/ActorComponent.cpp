#include "ActorComponent.h"
#include "Common/Actor/Actor.h"
#include "Common/World.h"
#include "SceneComponent.h"

namespace nilou {

    UActorComponent::UActorComponent(AActor *InOwner) 
        : OwnedActor(InOwner)
        , bRegistered(false)
        , bRenderStateDirty(false)
        , bRenderStateCreated(false)
        , bRenderDynamicDataDirty(false)
        , bRenderTransformDirty(false)
        , bRenderInstancesDirty(false)
    {
        if (InOwner)
        {
            WorldPrivate = InOwner->GetWorld();
            InOwner->AddOwnedComponent(this);
        }
    }

    UWorld *UActorComponent::GetWorld() const
    { 
        return OwnedActor ? OwnedActor->GetWorld() : nullptr;
    }

    AActor *UActorComponent::GetOwner() const
    { 
        return OwnedActor;
    }

    void UActorComponent::RegisterComponentWithWorld(UWorld *World)
    {
        bRegistered = true;
        WorldPrivate = World;
        CreateRenderState();
    }

    void UActorComponent::RegisterComponent()
    {
        bRegistered = true;
        AActor *MyOwner = GetOwner();
        UWorld* MyOwnerWorld = (MyOwner ? MyOwner->GetWorld() : nullptr);
        if (MyOwnerWorld)
        {
            RegisterComponentWithWorld(MyOwnerWorld);
        }
    }

    void UActorComponent::UnregisterComponent()
    {
        bRegistered = false;
        DestroyRenderState();
    }

    bool UActorComponent::IsRegistered() const
    {
        return bRegistered;
    }

    void UActorComponent::DestroyComponent()
    {
        if (AActor *MyOwner = GetOwner())
        {
            MyOwner->RemoveOwnedComponent(this);
            if (static_cast<UActorComponent *>(MyOwner->GetRootComponent()) == this)
            {
                MyOwner->SetRootComponent(nullptr);
            }
        }
    }
    
    void UActorComponent::RecreateRenderState()
    {
        if(bRenderStateCreated)
        {
            DestroyRenderState();
        }
        if(IsRegistered() && WorldPrivate->Scene)
        {
            CreateRenderState();
        }
    }

    void UActorComponent::CreateRenderState()
    {
        bRenderStateCreated = true;
        bRenderStateDirty = false;
        bRenderTransformDirty = false;
    }

    void UActorComponent::DoDeferredRenderUpdates()
    {
        if(bRenderStateDirty)
        {
            RecreateRenderState();
        }
        else
        {
            if(bRenderTransformDirty)
            {
                // Update the component's transform if the actor has been moved since it was last updated.
                SendRenderTransform();
            }

            if (bRenderDynamicDataDirty)
            {
                SendRenderDynamicData();
            }

            if (bRenderInstancesDirty)
            {
                SendRenderInstanceData();
            }
        }
    }

    void UActorComponent::SendRenderTransform()
    {
        if (bRenderStateCreated)
	        bRenderTransformDirty = false;
    }

    void UActorComponent::SendRenderDynamicData()
    {
        if (bRenderDynamicDataDirty)
	        bRenderDynamicDataDirty = false;
    }

    void UActorComponent::SendRenderInstanceData()
    {
        if (bRenderStateCreated)
	        bRenderInstancesDirty = false;
    }

    void UActorComponent::DestroyRenderState()
    {
        bRenderStateCreated = false;	
        bRenderStateDirty = false;
        bRenderTransformDirty = false;
    }

    void UActorComponent::MarkRenderStateDirty()
    {
        bRenderStateDirty = true;
    }

    void UActorComponent::MarkRenderTransformDirty()
    {
        bRenderTransformDirty = true;
    }

    void UActorComponent::MarkRenderDynamicDataDirty()
    {
        bRenderDynamicDataDirty = true;
    }

    void UActorComponent::MarkRenderInstancesDirty()
    {
        bRenderInstancesDirty = true;
    }
}