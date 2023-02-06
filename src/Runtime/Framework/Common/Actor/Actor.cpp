#include "Actor.h"
#include "Common/AssertionMacros.h"
#include "Common/Components/ActorComponent.h"
#include "Common/Components/SceneComponent.h"
#include <string>
#include <vector>


namespace nilou {

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
    {
        RootComponent = std::make_shared<USceneComponent>(this);
    }

    quat AActor::GetActorRotation() const
    {
        return RootComponent->GetComponentToWorld().GetRotation();
    }
    FRotator AActor::GetActorRotator() const
    {
        return RootComponent->GetComponentRotation();
    }
    vec3 AActor::GetActorLocation() const
    {
        return RootComponent->GetComponentLocation();
    }
    vec3 AActor::GetActorScale3D() const 
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

    void AActor::SetActorRotation(const quat &rotation)
    {
    #ifdef _DEBUG
        //UNDDEBUG_PrintGLM(rotation);
        //UNDDEBUG_PrintGLM(Rotator(rotation));
    #endif

        RootComponent->SetWorldRotation(rotation);
        // RootComponent->MoveComponent(vec3(0, 0, 0), rotation);
    }

    void AActor::SetActorRotator(const FRotator &rotator)
    {
        RootComponent->SetWorldRotation(rotator);
        // RootComponent->MoveComponent(vec3(0, 0, 0), rotator);
        //SetActorRotation(quat(vec3(rotator.Pitch, rotator.Yaw, rotator.Roll)));
    }

    void AActor::SetActorLocation(const vec3 &location)
    {
        RootComponent->SetWorldLocation(location);
        // vec3 delta = location - RootComponent->GetComponentLocation();
        // //std::cout << location.x << " " << location.y << " " << location.z << std::endl;
        // RootComponent->MoveComponent(delta, RootComponent->GetComponentToWorld().GetRotation());
    }

    void AActor::SetActorScale3D(const vec3 &scale)
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

	void AActor::AddOwnedComponent(UActorComponent *InComponent)
    {
        OwnedComponents.insert(InComponent);
    }
    
	void AActor::RemoveOwnedComponent(UActorComponent *InComponent)
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
                RemoveOwnedComponent(RootComponent.get());
                AddOwnedComponent(NewRootComponent.get());
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

        std::vector<UActorComponent *> Components;
        GetComponents(Components);

        for (int i = 0; i < Components.size(); i++)
        {
            UActorComponent *Component = Components[i];
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

    
}
