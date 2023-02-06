#include "SceneComponent.h"
#include "Common/Transform.h"

#include <memory>

namespace nilou {

    FAttachmentTransformRules FAttachmentTransformRules::KeepRelativeTransform(EAttachmentRule::KeepRelative);
    FAttachmentTransformRules FAttachmentTransformRules::KeepWorldTransform(EAttachmentRule::KeepWorld);
    FAttachmentTransformRules FAttachmentTransformRules::SnapToTargetNotIncludingScale(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld);
    FAttachmentTransformRules FAttachmentTransformRules::SnapToTargetIncludingScale(EAttachmentRule::SnapToTarget);

    // FBoundingBox USceneComponent::CalcBounds(const FTransform& LocalToWorld) const
    // {
    //     FBoundingBox Bounds;
    //     Bounds.Min = Bounds.Max = LocalToWorld.GetTranslation();
    //     return Bounds;
    // }

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

    void USceneComponent::SetRelativeLocation(const vec3 &NewLocation)
    {
        SetRelativeLocationAndRotation(NewLocation, RelativeRotation);
    }
    void USceneComponent::SetRelativeRotation(const quat &NewRotation)
    {
        SetRelativeLocationAndRotation(RelativeLocation, NewRotation);
    }
    void USceneComponent::SetRelativeRotation(const FRotator &NewRotation)
    {
        SetRelativeLocationAndRotation(RelativeLocation, NewRotation.ToQuat());
    }
    void USceneComponent::SetRelativeScale3D(const vec3 &NewScale3D)
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
    void USceneComponent::SetRelativeLocationAndRotation(const vec3 &NewLocation, const quat &NewRotation)
    {
        FTransform DesiredRelTransform = FTransform(RelativeScale3D, NewRotation, NewLocation);
        FTransform DesiredWorldTransform = CalcNewComponentToWorld(DesiredRelTransform);
        vec3 DesiredDelta = DesiredWorldTransform.GetTranslation() - GetComponentTransform().GetTranslation();
        MoveComponent(DesiredDelta, DesiredWorldTransform.GetRotation());
    }
    void USceneComponent::SetRelativeLocationAndRotation(const vec3 &NewLocation, const FRotator &NewRotation)
    {
        SetRelativeLocationAndRotation(NewLocation, NewRotation.ToQuat());
    }

    void USceneComponent::SetWorldLocation(const vec3 &NewLocation)
    {
        vec3 NewRelLocation = NewLocation;
        if (GetAttachParent() != nullptr && !IsUsingAbsoluteLocation())
        {
            FTransform ParentToWorld = GetAttachParent()->GetSocketTransform(GetAttachSocketName());
            NewRelLocation = ParentToWorld.InverseTransformPosition(NewLocation);
        }
        SetRelativeLocation(NewRelLocation);
    }
    void USceneComponent::SetWorldRotation(const quat &NewRotation)
    {
        quat NewRelRotation = GetRelativeRotationFromWorld(NewRotation);
        SetRelativeRotation(NewRelRotation);
    }
    void USceneComponent::SetWorldRotation(const FRotator &NewRotation)
    {
        SetWorldRotation(NewRotation.ToQuat());
    }
    void USceneComponent::SetWorldScale3D(const vec3 &NewScale)
    {
        vec3 NewRelScale = NewScale;

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
    void USceneComponent::SetWorldLocationAndRotation(vec3 NewLocation, const quat &NewRotation)
    {
        quat NewFinalRotation = NewRotation;
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
                quat NewRelQuat = glm::inverse(ParentToWorld.GetRotation()) * NewRotation;
                NewFinalRotation = NewRelQuat;
            }
        }

        SetRelativeLocationAndRotation(NewLocation, NewFinalRotation);
    }
    void USceneComponent::SetWorldLocationAndRotation(vec3 NewLocation, FRotator NewRotation)
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

    quat USceneComponent::GetRelativeRotationFromWorld(const quat &NewRotation)
    {
        quat NewRelRotation = NewRotation;

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
                const quat NewRelQuat = NewTransform.GetRelativeTransform(ParentToWorld).GetRotation();
                NewRelRotation = NewRelQuat;
            }
            else
            {
                const quat ParentToWorldQuat = ParentToWorld.GetRotation();
                // Quat multiplication works reverse way, make sure you do Parent(-1) * World = Local, not World*Parent(-) = Local (the way matrix does)
                const quat NewRelQuat = glm::inverse(ParentToWorldQuat) * NewRotation;
                NewRelRotation = NewRelQuat;
            }
        }
        return NewRelRotation;
    }
    // void USceneComponent::SetTranslation(const vec3 &InTranslation)
    // {
    //     Transform.SetTranslation(InTranslation);
    // }

    // void USceneComponent::SetRelativeTransform(const FTransform &transform)
    // {
    //     RelativeTransform = transform;
    //     UpdateComponentToWorld();

    // }

    // void USceneComponent::SetWorldTransform(const FTransform &transform)
    // {
    //     WorldTransform = transform;
    //     UpdateComponentToParent();
    // }

    // FTransform USceneComponent::GetRelativeTransform() const
    // {
    //     return RelativeTransform;
    // }
    // FTransform USceneComponent::GetWorldTransform() const
    // {
    //     if (AttachParent)
    //     {
    //         FTransform ParentTransform = AttachParent->GetWorldTransform();
    //         if (bAbsoluteTranslation)
    //             ParentTransform.SetTranslation(vec3(0));
            
    //         if (bAbsoluteRotation)
    //             ParentTransform.SetRotation(quat());

    //         if (bAbsoluteScale)
    //             ParentTransform.SetScale3D(vec3(1));
    //         return ParentTransform * Transform;
    //     }
    //     else 
    //     {
    //         return Transform;
    //     }
    // }
    // vec3 USceneComponent::GetComponentLocation() const
    // {
    //     return WorldTransform.GetTranslation();
    // }
    // void USceneComponent::MoveComponentTo(const vec3 &Position)
    // {
    //     MoveComponent(Position - WorldTransform.GetTranslation(), WorldTransform.GetRotation());
    // }
    void USceneComponent::MoveComponent(const vec3 &Delta, const FRotator &NewRotation)
    {
        MoveComponent(Delta, NewRotation.ToQuat());
    }
    void USceneComponent::MoveComponent(const vec3 &Delta, const quat &NewRotation)
    {
	    quat NewRotationQuat(NewRotation);
        vec3 NewLocation = Delta + GetComponentLocation();
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

            // Here it is important to compute the quaternion from the rotator and not the opposite.
            // In some cases, similar quaternions generate the same rotator, which create issues.
            // When the component is loaded, the rotator is used to generate the quaternion, which
            // is then used to compute the ComponentToWorld matrix. When running a blueprint script,  
            // it is required to generate that same ComponentToWorld otherwise the FComponentInstanceDataCache
            // might fail to apply to the relevant component. In order to have the exact same transform
            // we must enforce the quaternion to come from the rotator (as in load)
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
        for (USceneComponent *ChildComp : AttachChildren)
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
    // void USceneComponent::UpdateComponentToParent()
    // {
    //     if (AttachParent)
    //         RelativeTransform = WorldTransform.CalcRelativeTransform(AttachParent->WorldTransform);
    //     else
    //         RelativeTransform = WorldTransform;

    //     for (USceneComponent *AttachChild : AttachChildren)
    //     {
    //         if (AttachChild)
    //             AttachChild->UpdateComponentToWorld();
    //     }
    // }

    void USceneComponent::AttachToComponent(USceneComponent *Parent, const FAttachmentTransformRules& AttachmentRules)
    {
        if (Parent == nullptr)
            return;
        // SetOwner(Parent->GetOwner());
        // if (!bRegistered)
        //     RegisterComponent();

        AttachParent = Parent;
        Parent->AttachChildren.push_back(this);
        
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
			RelativeLocation = vec3(0);
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
			RelativeScale3D = vec3(1.0f, 1.0f, 1.0f);
			break;
		}

        UpdateComponentToWorld();
    }

    // void USceneComponent::AttachToActor(AActor *Parent, const FAttachmentTransformRules& AttachmentRules)
    // {
        
    // }
}

