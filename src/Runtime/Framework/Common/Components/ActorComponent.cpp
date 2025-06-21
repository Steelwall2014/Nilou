#include "ActorComponent.h"
#include "Common/Actor/Actor.h"
#include "Common/Log.h"
#include "Common/World.h"
#include "SceneComponent.h"

namespace nilou {

    UActorComponent::UActorComponent() 
        : WorldPrivate(nullptr)
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
	    Ncheck(!bRegistered);
        bRegistered = true;

        UpdateComponentToWorld();
        CreateRenderState();
    }

    void UActorComponent::OnUnregister()
    {
	    Ncheck(bRegistered);
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
            NILOU_LOG(Display, "RegisterComponentWithWorld ({}) Already registered. Aborting.", GetClassName());
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
                NILOU_LOG(Display, "RegisterComponentWithWorld: ({}) Specifying a world, but an Owner Actor found, and InWorld is not GetOwner()->GetWorld()", GetClassName());
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
        Ncheck(bRegistered);
        Ncheck(!bHasBeenInitialized);

        bHasBeenInitialized = true;
    }

    void UActorComponent::UninitializeComponent()
    {
        Ncheck(bHasBeenInitialized);

        bHasBeenInitialized = false;
    }

    void UActorComponent::SetOwner(AActor *InOwner)
    {
        OwnedActor = InOwner;
        if (InOwner)
        {
            WorldPrivate = InOwner->GetWorld();
            InOwner->AddOwnedComponent(std::static_pointer_cast<UActorComponent>(shared_from_this()));
        }
    }

    void UActorComponent::BeginPlay()
    {
        Ncheck(bRegistered);
        Ncheck(!bHasBegunPlay);
        bHasBegunPlay = true;
    }

    void UActorComponent::DestroyComponent(bool bPromoteChildren)
    {
        if (bHasBeenInitialized)
        {
            UninitializeComponent();
        }

        if(IsRegistered())
        {
            UnregisterComponent();
        }

        if (AActor *MyOwner = GetOwner())
        {
            MyOwner->RemoveOwnedComponent(std::static_pointer_cast<UActorComponent>(this->shared_from_this()));
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