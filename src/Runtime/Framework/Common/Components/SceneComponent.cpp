#include "SceneComponent.h"
#include "Common/Transform.h"
#include "Common/Actor/Actor.h"

#include <memory>

namespace nilou {

    FAttachmentTransformRules FAttachmentTransformRules::KeepRelativeTransform(EAttachmentRule::KeepRelative);
    FAttachmentTransformRules FAttachmentTransformRules::KeepWorldTransform(EAttachmentRule::KeepWorld);
    FAttachmentTransformRules FAttachmentTransformRules::SnapToTargetNotIncludingScale(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld);
    FAttachmentTransformRules FAttachmentTransformRules::SnapToTargetIncludingScale(EAttachmentRule::SnapToTarget);

    FDetachmentTransformRules FDetachmentTransformRules::KeepRelativeTransform(EDetachmentRule::KeepRelative, true);
    FDetachmentTransformRules FDetachmentTransformRules::KeepWorldTransform(EDetachmentRule::KeepWorld, true);
    // FBoundingBox USceneComponent::CalcBounds(const FTransform& LocalToWorld) const
    // {
    //     FBoundingBox Bounds;
    //     Bounds.Min = Bounds.Max = LocalToWorld.GetTranslation();
    //     return Bounds;
    // }

    void USceneComponent::DestroyComponent(bool bPromoteChildren)
    {
        if (bPromoteChildren)
        {
            AActor* Owner = GetOwner();
            if (Owner != nullptr)
            {
                std::shared_ptr<USceneComponent> ChildToPromote = nullptr;

                const std::vector<std::shared_ptr<USceneComponent>>& AttachedChildren = GetAttachChildren();
                // Handle removal of the root node
                if (this == Owner->GetRootComponent())
                {
                    auto FindResult = std::find_if(AttachedChildren.begin(), AttachedChildren.end(), 
                        [Owner](std::shared_ptr<USceneComponent> Child) 
                        {
                            return Child != nullptr && Child->GetOwner() == Owner;
                        });

                    if (FindResult != AttachedChildren.end())
                    {
                        ChildToPromote = *FindResult;
                    }
                    else
                    {
                        // Didn't find a suitable component to promote so create a new default component

                        // Construct a new default root component
                        std::shared_ptr<USceneComponent> NewRootComponent = CreateComponent<USceneComponent>(Owner);
                        NewRootComponent->SetWorldLocationAndRotation(GetComponentLocation(), GetComponentRotation());
                        Owner->AddOwnedComponent(NewRootComponent);
                        NewRootComponent->RegisterComponent();

                        // Designate the new default root as the child we're promoting
                        ChildToPromote = NewRootComponent;
                    }

                    // Set the selected child node as the new root
                    check(ChildToPromote != nullptr);
                    Owner->SetRootComponent(ChildToPromote);
                }
                else    // ...not the root node, so we'll promote the selected child node to this position in its AttachParent's child array.
                {
                    // Cache our AttachParent
                    USceneComponent* CachedAttachParent = GetAttachParent();
                    if (CachedAttachParent != nullptr)
                    {
                        // Find the our position in its AttachParent's child array
                        const std::vector<std::shared_ptr<USceneComponent>>& AttachSiblings = CachedAttachParent->GetAttachChildren();
                        int32 Index = -1;
                        for (int i = 0; i < AttachSiblings.size(); i++)
                        {
                            if (AttachSiblings[i].get() == this)
                            {
                                Index = i;
                                break;
                            }
                        }
                        check(Index != -1);

                        // Detach from parent
                        DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

                        // Find an appropriate child node to promote to this node's position in the hierarchy
                        if (AttachedChildren.size() > 0)
                        {
                            // Always choose non editor-only child nodes over editor-only child nodes (since we don't want editor-only nodes to end up with non editor-only child nodes)
                            auto FindResult = std::find_if(AttachedChildren.begin(), AttachedChildren.end(), 
                                [Owner](std::shared_ptr<USceneComponent> Child) 
                                {
                                    return Child != nullptr;
                                });
                            if (FindResult != AttachedChildren.end())
                            {
                                ChildToPromote = *FindResult;
                            }
                            else
                            {
                                // Default to first child node
                                if (AttachedChildren[0] != nullptr)
                                {
                                    ChildToPromote = AttachedChildren[0];
                                }
                            }
                        }

                        if (ChildToPromote != nullptr)
                        {
                            // Attach the child node that we're promoting to the parent and move it to the same position as the old node was in the array
                            ChildToPromote->AttachToComponent(CachedAttachParent, FAttachmentTransformRules::KeepWorldTransform);
                            CachedAttachParent->AttachChildren.erase(
                                std::find(
                                    CachedAttachParent->AttachChildren.begin(), 
                                    CachedAttachParent->AttachChildren.end(), ChildToPromote));

                            Index = glm::clamp<int32>(Index, 0, AttachSiblings.size());
                            CachedAttachParent->AttachChildren.insert(
                                CachedAttachParent->AttachChildren.begin()+Index, ChildToPromote);
                        }
                    }
                }

                // Detach child nodes from the node that's being removed and re-attach them to the child that's being promoted
                std::vector<std::shared_ptr<USceneComponent>> AttachChildrenLocalCopy(AttachedChildren);
                for (std::shared_ptr<USceneComponent> Child : AttachChildrenLocalCopy)
                {
                    if (Child)
                    {
                        // Note: This will internally call Modify(), so we don't need to call it here
                        Child->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
                        if (Child != ChildToPromote)
                        {
                            Child->AttachToComponent(ChildToPromote.get(), FAttachmentTransformRules::KeepWorldTransform);
                        }
                    }
                }
            }
        }
        UActorComponent::DestroyComponent(bPromoteChildren);
    }

    FTransform USceneComponent::CalcNewComponentToWorld(const FTransform &NewRelativeTransform)
    {
        USceneComponent *Parent = GetAttachParent();
        if (Parent)
        {
            bool bGeneral = IsUsingAbsoluteLocation() || IsUsingAbsoluteRotation() || IsUsingAbsoluteScale();
            if (!bGeneral)
			{
				return NewRelativeTransform * Parent->GetComponentToWorld();
			}
            else
            {
                FTransform NewCompToWorld = NewRelativeTransform * Parent->GetComponentToWorld();
                if (IsUsingAbsoluteLocation())
                    NewCompToWorld.SetTranslation(NewRelativeTransform.GetTranslation());
                if (IsUsingAbsoluteRotation())
                    NewCompToWorld.SetRotation(NewRelativeTransform.GetRotation());
                if (IsUsingAbsoluteScale())
                    NewCompToWorld.SetScale3D(NewRelativeTransform.GetScale3D());

                return NewCompToWorld;
            }
        }
        else 
        {
            return NewRelativeTransform;
        }
    }

    void USceneComponent::SetRelativeLocation(const dvec3 &NewLocation)
    {
        SetRelativeLocationAndRotation(NewLocation, RelativeRotation);
    }
    void USceneComponent::SetRelativeRotation(const dquat &NewRotation)
    {
        SetRelativeLocationAndRotation(RelativeLocation, NewRotation);
    }
    void USceneComponent::SetRelativeRotation(const FRotator &NewRotation)
    {
        SetRelativeLocationAndRotation(RelativeLocation, NewRotation.ToQuat());
    }
    void USceneComponent::SetRelativeScale3D(const dvec3 &NewScale3D)
    {
        RelativeScale3D = NewScale3D;
		UpdateComponentToWorld();
    }
    void USceneComponent::SetRelativeTransform(const FTransform &NewTransform)
    {
        SetRelativeLocationAndRotation(NewTransform.GetTranslation(), NewTransform.GetRotation());
        SetRelativeScale3D(NewTransform.GetScale3D());
    }
    FTransform USceneComponent::GetRelativeTransform() const
    {
        return FTransform(RelativeScale3D, RelativeRotation.ToQuat(), RelativeLocation);
    }
    void USceneComponent::SetRelativeLocationAndRotation(const dvec3 &NewLocation, const dquat &NewRotation)
    {
        FTransform DesiredRelTransform = FTransform(RelativeScale3D, NewRotation, NewLocation);
        FTransform DesiredWorldTransform = CalcNewComponentToWorld(DesiredRelTransform);
        dvec3 DesiredDelta = DesiredWorldTransform.GetTranslation() - GetComponentTransform().GetTranslation();
        MoveComponent(DesiredDelta, DesiredWorldTransform.GetRotation());
    }
    void USceneComponent::SetRelativeLocationAndRotation(const dvec3 &NewLocation, const FRotator &NewRotation)
    {
        SetRelativeLocationAndRotation(NewLocation, NewRotation.ToQuat());
    }

    void USceneComponent::SetWorldLocation(const dvec3 &NewLocation)
    {
        dvec3 NewRelLocation = NewLocation;
        if (GetAttachParent() != nullptr && !IsUsingAbsoluteLocation())
        {
            FTransform ParentToWorld = GetAttachParent()->GetSocketTransform(GetAttachSocketName());
            NewRelLocation = ParentToWorld.InverseTransformPosition(NewLocation);
        }
        SetRelativeLocation(NewRelLocation);
    }
    void USceneComponent::SetWorldRotation(const dquat &NewRotation)
    {
        dquat NewRelRotation = GetRelativeRotationFromWorld(NewRotation);
        SetRelativeRotation(NewRelRotation);
    }
    void USceneComponent::SetWorldRotation(const FRotator &NewRotation)
    {
        SetWorldRotation(NewRotation.ToQuat());
    }
    void USceneComponent::SetWorldScale3D(const dvec3 &NewScale)
    {
        dvec3 NewRelScale = NewScale;

        // If attached to something, transform into local space
        if(GetAttachParent() != nullptr && !IsUsingAbsoluteScale())
        {
            FTransform ParentToWorld = GetAttachParent()->GetSocketTransform(GetAttachSocketName());
            NewRelScale = NewScale * ParentToWorld.GetSafeScaleReciprocal(ParentToWorld.GetScale3D());
        }

        SetRelativeScale3D(NewRelScale);
    }
    void USceneComponent::SetWorldTransform(const FTransform &NewTransform)
    {
        // If attached to something, transform into local space
        if (GetAttachParent() != nullptr)
        {
            const FTransform ParentToWorld = GetAttachParent()->GetSocketTransform(GetAttachSocketName());
            FTransform RelativeTM = NewTransform.GetRelativeTransform(ParentToWorld);

            // Absolute location, rotation, and scale use the world transform directly.
            if (IsUsingAbsoluteLocation())
            {
                RelativeTM.SetTranslation(NewTransform.GetTranslation());
            }

            if (IsUsingAbsoluteRotation())
            {
                RelativeTM.SetRotation(NewTransform.GetRotation());
            }

            if (IsUsingAbsoluteScale())
            {
                RelativeTM.SetScale3D(NewTransform.GetScale3D());
            }

            SetRelativeTransform(RelativeTM);
        }
        else
        {
            SetRelativeTransform(NewTransform);
        }
    }
    void USceneComponent::SetWorldLocationAndRotation(dvec3 NewLocation, const dquat &NewRotation)
    {
        dquat NewFinalRotation = NewRotation;
        if (GetAttachParent() != nullptr)
        {
            FTransform ParentToWorld = GetAttachParent()->GetSocketTransform(GetAttachSocketName());

            if (!IsUsingAbsoluteLocation())
            {
                NewLocation = ParentToWorld.InverseTransformPosition(NewLocation);
            }

            if (!IsUsingAbsoluteRotation())
            {
                // Quat multiplication works reverse way, make sure you do Parent(-1) * World = Local, not World*Parent(-) = Local (the way matrix does)
                dquat NewRelQuat = glm::inverse(ParentToWorld.GetRotation()) * NewRotation;
                NewFinalRotation = NewRelQuat;
            }
        }

        SetRelativeLocationAndRotation(NewLocation, NewFinalRotation);
    }
    void USceneComponent::SetWorldLocationAndRotation(dvec3 NewLocation, FRotator NewRotation)
    {
        if (GetAttachParent() == nullptr)
        {
            // No parent, relative == world. Use FRotator version because it can check for rotation change without conversion issues.
            SetRelativeLocationAndRotation(NewLocation, NewRotation);
        }
        else
        {
            SetWorldLocationAndRotation(NewLocation, NewRotation.ToQuat());
        }
    }

    dquat USceneComponent::GetRelativeRotationFromWorld(const dquat &NewRotation)
    {
        dquat NewRelRotation = NewRotation;

        // If already attached to something, transform into local space
        if (GetAttachParent() != nullptr && !IsUsingAbsoluteRotation())
        {
            const FTransform  ParentToWorld = GetAttachParent()->GetSocketTransform(GetAttachSocketName());
            // in order to support mirroring, you'll have to use FTransform.GetRelativeTransform
            // because negative SCALE should flip the rotation
            if (FTransform::AnyHasNegativeScale(GetRelativeScale3D(), ParentToWorld.GetScale3D()))
            {
                FTransform NewTransform = GetComponentTransform();
                // set new desired rotation
                NewTransform.SetRotation(NewRotation);
                // Get relative transform from ParentToWorld
                const dquat NewRelQuat = NewTransform.GetRelativeTransform(ParentToWorld).GetRotation();
                NewRelRotation = NewRelQuat;
            }
            else
            {
                const dquat ParentToWorldQuat = ParentToWorld.GetRotation();
                // Quat multiplication works reverse way, make sure you do Parent(-1) * World = Local, not World*Parent(-) = Local (the way matrix does)
                const dquat NewRelQuat = glm::inverse(ParentToWorldQuat) * NewRotation;
                NewRelRotation = NewRelQuat;
            }
        }
        return NewRelRotation;
    }

    void USceneComponent::MoveComponent(const dvec3 &Delta, const FRotator &NewRotation)
    {
        MoveComponent(Delta, NewRotation.ToQuat());
    }

    void USceneComponent::MoveComponent(const dvec3 &Delta, const dquat &NewRotation)
    {
	    dquat NewRotationQuat(NewRotation);
        dvec3 NewLocation = Delta + GetComponentLocation();
        if (GetAttachParent() != nullptr)
        {
            FTransform const ParentToWorld = GetAttachParent()->GetSocketTransform(GetAttachSocketName());
            // in order to support mirroring, you'll have to use FTransform.GetrelativeTransform
            // because negative scale should flip the rotation
            if (FTransform::AnyHasNegativeScale(GetRelativeScale3D(), ParentToWorld.GetScale3D()))
            {
                FTransform const WorldTransform = FTransform(GetRelativeScale3D() * ParentToWorld.GetScale3D(), NewRotation, NewLocation);
                FTransform const RelativeTransform = WorldTransform.GetRelativeTransform(ParentToWorld);

                if (!IsUsingAbsoluteLocation())
                {
                    NewLocation = RelativeTransform.GetLocation();
                }

                if (!IsUsingAbsoluteRotation())
                {
                    NewRotationQuat = RelativeTransform.GetRotation();
                }
            }
            else
            {
                if (!IsUsingAbsoluteLocation())
                {
                    NewLocation = ParentToWorld.InverseTransformPosition(NewLocation);
                }

                if (!IsUsingAbsoluteRotation())
                {
                    // Quat multiplication works reverse way, make sure you do Parent(-1) * World = Local, not World*Parent(-) = Local (the way matrix does)
                    NewRotationQuat = glm::inverse(ParentToWorld.GetRotation()) * NewRotationQuat;
                }
            }
        }

        const FRotator NewRelativeRotation(NewRotationQuat);
        
        bool bDiffLocation = !Equals(NewLocation, GetRelativeLocation());
	    bool bDiffRotation = !NewRelativeRotation.Equals(GetRelativeRotation());
        if (bDiffLocation || bDiffRotation)
        {
            RelativeLocation = NewLocation;

            // Here it is important to compute the dquaternion from the rotator and not the opposite.
            // In some cases, similar dquaternions generate the same rotator, which create issues.
            // When the component is loaded, the rotator is used to generate the dquaternion, which
            // is then used to compute the ComponentToWorld matrix. When running a blueprint script,  
            // it is required to generate that same ComponentToWorld otherwise the FComponentInstanceDataCache
            // might fail to apply to the relevant component. In order to have the exact same transform
            // we must enforce the dquaternion to come from the rotator (as in load)
            if (bDiffRotation)
            {
                RelativeRotation = NewRelativeRotation;
            }

            UpdateComponentToWorld();

        }
    }

    vec3 USceneComponent::GetForwardVector()
    {
        return GetComponentToWorld().TransformVectorNoScale(WORLD_FORWARD);
    }
    vec3 USceneComponent::GetUpVector()
    {
        return GetComponentToWorld().TransformVectorNoScale(WORLD_UP);
    }
    vec3 USceneComponent::GetRightVector()
    {
        return GetComponentToWorld().TransformVectorNoScale(WORLD_RIGHT);
    }
    
    void USceneComponent::UpdateComponentToWorld()
    {
        if (AttachParent && !AttachParent->bComponentToWorldUpdated)
        {
            AttachParent->UpdateComponentToWorld();
            if (bComponentToWorldUpdated)
                return;
        }

        bComponentToWorldUpdated = true;
        FTransform RelativeTransform(RelativeScale3D, RelativeRotation.ToQuat(), RelativeLocation);
        ComponentToWorld = CalcNewComponentToWorld(RelativeTransform);
        MarkRenderTransformDirty();
        UpdateBounds();

        UpdateChildTransforms();
    }

    void USceneComponent::UpdateChildTransforms()
    {
        for (std::shared_ptr<USceneComponent> ChildComp : AttachChildren)
        {
            if (ChildComp)
            {
                if (!ChildComp->bComponentToWorldUpdated)
				{
					ChildComp->UpdateComponentToWorld();
				}
                else
				{
					// Don't update the child if it uses a completely absolute (world-relative) scheme.
					if (ChildComp->IsUsingAbsoluteLocation() && ChildComp->IsUsingAbsoluteRotation() && ChildComp->IsUsingAbsoluteScale())
					{
						continue;
					}

					ChildComp->UpdateComponentToWorld();
				}
            }
        }
    }

    void USceneComponent::AttachToComponent(USceneComponent *Parent, const FAttachmentTransformRules& AttachmentRules)
    {
        if (Parent == nullptr)
            return;
        // SetOwner(Parent->GetOwner());
        // if (!bRegistered)
        //     RegisterComponent();

        AttachParent = Parent;

        int32 Index = -1;
        for (int i = 0; i < Parent->AttachChildren.size(); i++)
        {
            if (Parent->AttachChildren[i].get() == this)
            {
                Index = i;
                break;
            }
        }
        if (Index == -1)
        {
            Parent->AttachChildren.push_back(std::static_pointer_cast<USceneComponent>(this->shared_from_this()));
        }
        
		FTransform SocketTransform = GetAttachParent()->GetSocketTransform(GetAttachSocketName());
		FTransform RelativeTM = GetComponentTransform().GetRelativeTransform(SocketTransform);

		switch (AttachmentRules.LocationRule)
		{
		case EAttachmentRule::KeepRelative:
			// dont do anything, keep relative position the same
			break;
		case EAttachmentRule::KeepWorld:
			if (IsUsingAbsoluteLocation())
			{
				RelativeLocation = GetComponentTransform().GetTranslation();
			}
			else
			{
				RelativeLocation = RelativeTM.GetTranslation();
			}
			break;
		case EAttachmentRule::SnapToTarget:
			RelativeLocation = dvec3(0);
			break;
		}

		switch (AttachmentRules.RotationRule)
		{
		case EAttachmentRule::KeepRelative:
			// dont do anything, keep relative rotation the same
			break;
		case EAttachmentRule::KeepWorld:
			if (IsUsingAbsoluteRotation())
			{
				RelativeRotation = GetComponentRotation();
			}
			else
			{
				RelativeRotation = FRotator(RelativeTM.GetRotation());
			}
			break;
		case EAttachmentRule::SnapToTarget:
			RelativeRotation = (FRotator());
			break;
		}

		switch (AttachmentRules.ScaleRule)
		{
		case EAttachmentRule::KeepRelative:
			// dont do anything, keep relative scale the same
			break;
		case EAttachmentRule::KeepWorld:
			if (IsUsingAbsoluteScale())
			{
				RelativeScale3D = GetComponentTransform().GetScale3D();
			}
			else
			{
				RelativeScale3D = RelativeTM.GetScale3D();
			}
			break;
		case EAttachmentRule::SnapToTarget:
			RelativeScale3D = dvec3(1.0f, 1.0f, 1.0f);
			break;
		}

        UpdateComponentToWorld();
    }

    void USceneComponent::DetachFromComponent(const FDetachmentTransformRules& DetachmentRules)
    {
        if (GetAttachParent() != nullptr)
        {
            AActor* Owner = GetOwner();

            for (int i = 0; i < GetAttachParent()->AttachChildren.size(); i++)
            {
                if (GetAttachParent()->AttachChildren[i].get() == this)
                {
                    GetAttachParent()->AttachChildren.erase(GetAttachParent()->AttachChildren.begin()+i);
                    break;
                }
            }
            SetAttachParent(nullptr);

            // If desired, update RelativeLocation and RelativeRotation to maintain current world position after detachment
            switch (DetachmentRules.LocationRule)
            {
            case EDetachmentRule::KeepRelative:
                break;
            case EDetachmentRule::KeepWorld:
                RelativeLocation = GetComponentTransform().GetTranslation(); // or GetComponentLocation, but worried about custom location...
                break;
            }

            switch (DetachmentRules.RotationRule)
            {
            case EDetachmentRule::KeepRelative:
                break;
            case EDetachmentRule::KeepWorld:
                RelativeRotation = GetComponentRotation();
                break;
            }

            switch (DetachmentRules.ScaleRule)
            {
            case EDetachmentRule::KeepRelative:
                break;
            case EDetachmentRule::KeepWorld:
                RelativeScale3D = GetComponentScale();
                break;
            }

            // calculate transform with new attachment condition
            UpdateComponentToWorld();
        }
    }

    // void USceneComponent::AttachToActor(AActor *Parent, const FAttachmentTransformRules& AttachmentRules)
    // {
        
    // }
}

