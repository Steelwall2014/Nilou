#include "ActorComponent.h"
#include "Common/Actor/Actor.h"
#include "Common/Log.h"
#include "Common/World.h"
#include "SceneComponent.h"

namespace nilou {

    UActorComponent::UActorComponent(AActor *InOwner) 
        : OwnedActor(InOwner)
        , WorldPrivate(nullptr)
        , bWantsInitializeComponent(true)
        , bHasBeenInitialized(false)
        , bHasBeenCreated(false)
        , bHasBegunPlay(false)
        , bRegistered(false)
        , bRenderStateDirty(false)
        , bRenderStateCreated(false)
        , bRenderDynamicDataDirty(false)
        , bRenderTransformDirty(false)
        , bRenderInstancesDirty(false)
    {
        SetOwner(InOwner);
    }

    UWorld *UActorComponent::GetWorld() const
    { 
        return OwnedActor ? OwnedActor->GetWorld() : nullptr;
    }

    AActor *UActorComponent::GetOwner() const
    { 
        return OwnedActor;
    }

    void UActorComponent::OnRegister()
    {
	    check(!bRegistered);
        bRegistered = true;

        UpdateComponentToWorld();
        CreateRenderState();
    }

    void UActorComponent::OnUnregister()
    {
	    check(bRegistered);
	    bRegistered = false;
    }

    void UActorComponent::OnComponentCreated()
    {
	    bHasBeenCreated = true;
    }

    void UActorComponent::RegisterComponentWithWorld(UWorld *InWorld)
    {
        if (IsRegistered())
        {
            NILOU_LOG(Info, "RegisterComponentWithWorld (%s) Already registered. Aborting.", GetClassName());
            return;
        }
        if (InWorld == nullptr)
        {
            return;
        }
	    AActor* MyOwner = GetOwner();
        if(MyOwner)
        {
            if(InWorld != MyOwner->GetWorld())
            {
                // The only time you should specify a scene that is not Owner->GetWorld() is when you don't have an Actor
                NILOU_LOG(Info, "RegisterComponentWithWorld: (%s) Specifying a world, but an Owner Actor found, and InWorld is not GetOwner()->GetWorld()", GetClassName());
            }
        }
        if (!bHasBeenCreated)
        {
            OnComponentCreated();
        }
	    WorldPrivate = InWorld;
        ExecuteRegisterEvents();

        if (MyOwner == nullptr)
        {
            if (bWantsInitializeComponent && !bHasBeenInitialized)
            {
                InitializeComponent();
            }
        }
        else 
        {
            if (bWantsInitializeComponent && !bHasBeenInitialized && MyOwner->IsActorInitialized())
            {
                InitializeComponent();
            }
            		
            if (MyOwner->HasActorBegunPlay() || MyOwner->IsActorBeginningPlay())
            {
                if (!bHasBegunPlay)
                {
                    BeginPlay();
                }
            }
        }
    }

    void UActorComponent::RegisterComponent()
    {
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

    void UActorComponent::InitializeComponent()
    {
        check(bRegistered);
        check(!bHasBeenInitialized);

        bHasBeenInitialized = true;
    }

    void UActorComponent::UninitializeComponent()
    {
        check(bHasBeenInitialized);

        bHasBeenInitialized = false;
    }

    void UActorComponent::SetOwner(AActor *InOwner)
    {
        OwnedActor = InOwner;
        if (InOwner)
        {
            WorldPrivate = InOwner->GetWorld();
            InOwner->AddOwnedComponent(this);
        }
    }

    void UActorComponent::BeginPlay()
    {
        check(bRegistered);
        check(!bHasBegunPlay);
        bHasBegunPlay = true;
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
    
    void UActorComponent::ExecuteRegisterEvents()
    {
        if (!bRegistered)
        {
            OnRegister();
        }

        if (!bRenderStateCreated && WorldPrivate->Scene)
        {
            CreateRenderState();
        }
    }
    
    void UActorComponent::ExecuteUnregisterEvents()
    {
        if (bRenderStateCreated)
        {
            DestroyRenderState();
        }

        if (bRegistered)
        {
            OnUnregister();
        }
    }
}