#include <string>
#include <vector>

#include "Common/World.h"

#include "Actor.h"
#include "Common/AssertionMacros.h"
#include "Common/Components/ActorComponent.h"
#include "Common/Components/SceneComponent.h"


namespace nilou {

    static void DispatchOnComponentsCreated(AActor* NewActor)
    {
        std::vector<std::weak_ptr<UActorComponent>> Components;
        NewActor->GetComponents(Components);

        for (auto WeakActorComp : Components)
        {
            if (!WeakActorComp.expired())
            {
                auto ActorComp = WeakActorComp.lock();
                if (ActorComp->HasBeenCreated())
                    ActorComp->OnComponentCreated();
            }
        }
    }

    static USceneComponent* GetUnregisteredParent(UActorComponent* Component)
    {
        USceneComponent* ParentComponent = nullptr;
        USceneComponent* SceneComponent = dynamic_cast<USceneComponent *>(Component);
        
        while (	SceneComponent && 
                SceneComponent->GetAttachParent() &&
                SceneComponent->GetAttachParent()->GetOwner() == Component->GetOwner() &&
                !SceneComponent->GetAttachParent()->IsRegistered())
        {
            SceneComponent = SceneComponent->GetAttachParent();
            ParentComponent = SceneComponent;
        }

        return ParentComponent;
    }
    
    AActor::AActor()
        : OwnedWorld(nullptr)
        , ActorName("")
        , bActorInitialized(false)
        , ActorHasBegunPlay(EActorBeginPlayState::HasNotBegunPlay)
    {
        RootComponent = CreateComponent<USceneComponent>(this);
    }

    void AActor::PostSpawnInitialize(FTransform const& UserSpawnTransform)
    {
	    UWorld* const World = GetWorld();

        DispatchOnComponentsCreated(this);

        if (World)
        {
            RegisterAllComponents();
        }

	    PostActorCreated();
        FinishSpawning(UserSpawnTransform);
    }

    void AActor::FinishSpawning(const FTransform &UserTransform)
    {
        ExecuteConstruction(UserTransform);
        PostActorConstruction();
    }

    void AActor::BeginPlay()
    {
		ActorHasBegunPlay = EActorBeginPlayState::BeginningPlay;

        std::vector<std::weak_ptr<UActorComponent>> Components;
        GetComponents(Components);
	    for (auto WeakComponent : Components)
        {
            if (WeakComponent.expired())
                continue;
            auto Component = WeakComponent.lock();
            if (Component->IsRegistered() && !Component->HasBegunPlay())
            {
                Component->BeginPlay();
            }
        }

	    ActorHasBegunPlay = EActorBeginPlayState::HasBegunPlay;
    }

    dquat AActor::GetActorRotation() const
    {
        return RootComponent->GetComponentToWorld().GetRotation();
    }
    FRotator AActor::GetActorRotator() const
    {
        return RootComponent->GetComponentRotation();
    }
    dvec3 AActor::GetActorLocation() const
    {
        return RootComponent->GetComponentLocation();
    }
    dvec3 AActor::GetActorScale3D() const 
    {
        return RootComponent->GetComponentScale();
    }

    vec3 AActor::GetActorForwardVector() const
    {
        return RootComponent->GetForwardVector();
    }

    vec3 AActor::GetActorUpVector() const
    {
        return RootComponent->GetUpVector();
    }

    vec3 AActor::GetActorRightVector() const
    {
        return RootComponent->GetRightVector();
    }

    void AActor::SetActorTransform(const FTransform &InTransform)
    {
        RootComponent->SetWorldTransform(InTransform);
    }

    void AActor::SetActorRotation(const dquat &rotation)
    {
        RootComponent->SetWorldRotation(rotation);
    }

    void AActor::SetActorRotator(const FRotator &rotator)
    {
        RootComponent->SetWorldRotation(rotator);
    }

    void AActor::SetActorLocation(const dvec3 &location)
    {
        RootComponent->SetWorldLocation(location);
    }

    void AActor::SetActorScale3D(const dvec3 &scale)
    {
        RootComponent->SetWorldScale3D(scale);
    }

    void AActor::SetActorName(const std::string &InName)
    {
        ActorName = InName;
    }


    void AActor::AttachToActor(AActor *ParentActor, const FAttachmentTransformRules& AttachmentRules)
    {
        AttachToComponent(ParentActor->GetRootComponent(), AttachmentRules);
    }

    void AActor::AttachToComponent(USceneComponent *Parent, const FAttachmentTransformRules& AttachmentRules)
    {
        if (RootComponent && Parent)
        {
            RootComponent->AttachToComponent(Parent, AttachmentRules);
        }
    }

	void AActor::AddOwnedComponent(std::shared_ptr<UActorComponent> InComponent)
    {
        OwnedComponents.insert(InComponent);
    }
    
	void AActor::RemoveOwnedComponent(std::shared_ptr<UActorComponent> InComponent)
    {
        OwnedComponents.erase(InComponent);
    }
    
	void AActor::SetRootComponent(std::shared_ptr<USceneComponent> NewRootComponent)
    {
        /** Only components owned by this actor can be used as a its root component. */
        if (NewRootComponent == nullptr || NewRootComponent->GetOwner() == this)
        {
            if (RootComponent != NewRootComponent)
            {
                RemoveOwnedComponent(RootComponent);
                AddOwnedComponent(NewRootComponent);
                RootComponent = NewRootComponent;
            }
        }
    }
    
	void AActor::RegisterAllComponents()
    {
        UWorld* const World = GetWorld();
        check(World);

        if (RootComponent != nullptr && !RootComponent->IsRegistered())
        {
            RootComponent->RegisterComponentWithWorld(World);
        }

        std::vector<std::weak_ptr<UActorComponent>> Components;
        GetComponents(Components);

        for (int i = 0; i < Components.size(); i++)
        {
            auto WeakComponent = Components[i];
            if (WeakComponent.expired())
                continue;
            auto Component = WeakComponent.lock().get();
            if (!Component->IsRegistered())
            {
                USceneComponent *UnregisteredParentComponent = GetUnregisteredParent(Component);
                if (UnregisteredParentComponent)
                {
                    Component = UnregisteredParentComponent;
                    i--;
                }
			    Component->RegisterComponentWithWorld(World);
            }
        }
    }

    void AActor::InitializeComponents()
    {
        std::vector<std::weak_ptr<UActorComponent>> Components;
        GetComponents(Components);

        for (auto WeakActorComp : Components)
        {
            if (WeakActorComp.expired())
                continue;
            auto ActorComp = WeakActorComp.lock();
            if (ActorComp->IsRegistered())
            {
                if (ActorComp->bWantsInitializeComponent && !ActorComp->HasBeenInitialized())
                {
                    // Broadcast the activation event since Activate occurs too early to fire a callback in a game
                    ActorComp->InitializeComponent();
                }
            }
        }
    }

    void AActor::UninitializeComponents()
    {
        std::vector<std::weak_ptr<UActorComponent>> Components;
        GetComponents(Components);

        for (auto WeakActorComp : Components)
        {
            if (WeakActorComp.expired())
                continue;
            auto ActorComp = WeakActorComp.lock();
            if (ActorComp->HasBeenInitialized())
            {
                ActorComp->UninitializeComponent();
            }
        }
    }

    void AActor::ExecuteConstruction(const FTransform& Transform)
    {
        if (RootComponent)
        {
            RootComponent->SetWorldLocation(Transform.GetLocation());
            RootComponent->SetWorldRotation(Transform.GetRotation());
            RootComponent->SetWorldScale3D(Transform.GetScale3D());
        }
        
	    OnConstruction(Transform);
    }

    void AActor::PostActorConstruction()
    {
	    UWorld* const World = GetWorld();
        InitializeComponents();
        bActorInitialized = true;
		bool bRunBeginPlay = World->HasBegunPlay();
        if (bRunBeginPlay)
        {
			BeginPlay();
        }
    }
}
