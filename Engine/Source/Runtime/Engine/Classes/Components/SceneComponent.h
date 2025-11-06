#pragma once
#include <memory>
#include <string>
#include <vector>

#include "ActorComponent.h"
#include "Frustum.h"
#include "HAL/Platform.h"
#include "Math/Transform.h"
#include "Math/BoxSphereBounds.h"
#include "UniformBuffer.h"

// #include "SceneComponent.generated.h"

namespace nilou {

    enum class EAttachmentRule : uint8
    {
        /** Keeps current relative transform as the relative transform to the new parent. */
        KeepRelative,

        /** Automatically calculates the relative transform such that the attached component maintains the same world transform. */
        KeepWorld,

        /** Snaps transform to the attach point */
        SnapToTarget,
    };

    struct FAttachmentTransformRules
    {
        /** Various preset attachment rules. */
        static FAttachmentTransformRules KeepRelativeTransform;
        static FAttachmentTransformRules KeepWorldTransform;
        static FAttachmentTransformRules SnapToTargetNotIncludingScale;
        static FAttachmentTransformRules SnapToTargetIncludingScale;

        FAttachmentTransformRules(EAttachmentRule InRule)
            : LocationRule(InRule)
            , RotationRule(InRule)
            , ScaleRule(InRule)
        {}

        FAttachmentTransformRules(EAttachmentRule InLocationRule, EAttachmentRule InRotationRule, EAttachmentRule InScaleRule)
            : LocationRule(InLocationRule)
            , RotationRule(InRotationRule)
            , ScaleRule(InScaleRule)
        {}

        /** The rule to apply to location when attaching */
        EAttachmentRule LocationRule;

        /** The rule to apply to rotation when attaching */
        EAttachmentRule RotationRule;

        /** The rule to apply to scale when attaching */
        EAttachmentRule ScaleRule;
    };

    enum class EDetachmentRule : uint8
    {
        /** Keeps current relative transform. */
        KeepRelative,

        /** Automatically calculates the relative transform such that the detached component maintains the same world transform. */
        KeepWorld,
    };

    /** Rules for detaching components */
    struct FDetachmentTransformRules
    {
        /** Various preset detachment rules */
        static FDetachmentTransformRules KeepRelativeTransform;
        static FDetachmentTransformRules KeepWorldTransform;

        FDetachmentTransformRules(EDetachmentRule InRule, bool bInCallModify)
            : LocationRule(InRule)
            , RotationRule(InRule)
            , ScaleRule(InRule)
            , bCallModify(bInCallModify)
        {}

        FDetachmentTransformRules(EDetachmentRule InLocationRule, EDetachmentRule InRotationRule, EDetachmentRule InScaleRule, bool bInCallModify)
            : LocationRule(InLocationRule)
            , RotationRule(InRotationRule)
            , ScaleRule(InScaleRule)
            , bCallModify(bInCallModify)
        {}

        FDetachmentTransformRules(const FAttachmentTransformRules& AttachmentRules, bool bInCallModify)
            : LocationRule(AttachmentRules.LocationRule == EAttachmentRule::KeepRelative ? EDetachmentRule::KeepRelative : EDetachmentRule::KeepWorld)
            , RotationRule(AttachmentRules.RotationRule == EAttachmentRule::KeepRelative ? EDetachmentRule::KeepRelative : EDetachmentRule::KeepWorld)
            , ScaleRule(AttachmentRules.ScaleRule == EAttachmentRule::KeepRelative ? EDetachmentRule::KeepRelative : EDetachmentRule::KeepWorld)
            , bCallModify(bInCallModify)
        {}

        /** The rule to apply to location when detaching */
        EDetachmentRule LocationRule;

        /** The rule to apply to rotation when detaching */
        EDetachmentRule RotationRule;

        /** The rule to apply to scale when detaching */
        EDetachmentRule ScaleRule;

        /** Whether to call Modify() on the components concerned when detaching */
        bool bCallModify;
    };

    class NCLASS USceneComponent : public UActorComponent
    {
        GENERATED_BODY()
    public:
        USceneComponent() 
            : AttachParent(nullptr)
        { }

        /** Calculate the bounds of the component. Default behavior is a bounding box/sphere of zero size. */
        virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const
        {
            FBox Box;
            Box.Min = Box.Max = LocalToWorld.GetTranslation();
            return FBoxSphereBounds(Box);
        }

        /** Calculate the local bounds of the component. Default behavior is calling CalcBounds with an identity transform. */
        virtual FBoxSphereBounds CalcLocalBounds() const 
        { 
            return CalcBounds(FTransform::Identity);
        }

        virtual void UpdateBounds()
        {
            Bounds = CalcBounds(GetComponentTransform());
        }

        virtual void DestroyComponent(bool bPromoteChildren = false) override;

	    FBoxSphereBounds GetBounds() const
        {
            return Bounds;
        }

        FTransform CalcNewComponentToWorld(const FTransform &NewRelTransform);

        void SetRelativeLocation(const FVector &NewLocation);
        void SetRelativeRotation(const FQuat &NewRotation);
        void SetRelativeRotation(const FRotator &NewRotation);
	    void SetRelativeScale3D(const FVector &NewScale3D);
        void SetRelativeTransform(const FTransform &NewTransform);
        void SetRelativeLocationAndRotation(const FVector &NewLocation, const FQuat &NewRotation);
        void SetRelativeLocationAndRotation(const FVector &NewLocation, const FRotator &NewRotation);

        FVector GetRelativeLocation() const { return RelativeLocation; }
        FRotator GetRelativeRotation() const { return RelativeRotation; }
        FVector GetRelativeScale3D() const { return RelativeScale3D; }
        
	    FTransform GetRelativeTransform() const;

        void SetWorldLocation(const FVector &NewLocation);
        void SetWorldRotation(const FQuat &NewRotation);
        void SetWorldRotation(const FRotator &NewRotation);
	    void SetWorldScale3D(const FVector &NewScale);
        void SetWorldTransform(const FTransform &NewTransform);
        void SetWorldLocationAndRotation(FVector NewLocation, const FQuat &NewRotation);
        void SetWorldLocationAndRotation(FVector NewLocation, FRotator NewRotation);

        /** Currently not implemented*/
        std::string GetAttachSocketName() { return ""; }
        /** Currently not implemented*/
        FTransform GetSocketTransform(const std::string &) { return GetComponentTransform(); }

        /**  
        * Convenience function to get the relative rotation from the passed in world rotation
        * @param WorldRotation  World rotation that we want to convert to relative to the components parent
        * @return Returns the relative rotation
        */
	    FQuat GetRelativeRotationFromWorld(const FQuat &NewRotation);

        inline FVector GetComponentLocation() const
        {
            return GetComponentTransform().GetLocation();
        }
        inline FRotator GetComponentRotation() const
        {
            return GetComponentTransform().GetRotator();
        }
        inline FQuat GetComponentQuat() const
        {
            return GetComponentTransform().GetRotation();
        }
        inline FVector GetComponentScale() const
        {
            return GetComponentTransform().GetScale3D();
        }
        inline FTransform GetComponentToWorld() const
        {
            return ComponentToWorld;
        }
        inline FTransform GetComponentTransform() const
        {
            return ComponentToWorld;
        }

        const std::vector<USceneComponent*>& GetAttachChildren() const { return AttachChildren; }

        // 在世界参考系中移动node，将node在世界参考系中的旋转设置为NewRotation
        void MoveComponent(const FVector &Delta, const FRotator &NewRotation);
        void MoveComponent(const FVector &Delta, const FQuat &NewRotation);

        // 获取这个节点的forward、up和right向量，在世界坐标系
        FVector GetForwardVector();
        FVector GetUpVector();
        FVector GetRightVector();

        // 更新组件的transform
        void UpdateComponentToWorld();
        void UpdateChildTransforms();

        void AttachToComponent(USceneComponent *Parent, const FAttachmentTransformRules &AttachmentRules=FAttachmentTransformRules::KeepRelativeTransform);
        // void AttachToActor(AActor *Parent, const FAttachmentTransformRules &AttachmentRules);
        void DetachFromComponent(const FDetachmentTransformRules& DetachmentRules);

        USceneComponent *GetAttachParent() { return AttachParent; }

        void SetAttachParent(USceneComponent* NewAttachParent) { AttachParent = NewAttachParent; }

        bool IsUsingAbsoluteLocation() const
        {
            return bAbsoluteLocation;
        }
        
        bool IsUsingAbsoluteRotation() const
        {
            return bAbsoluteRotation;
        }
        
        bool IsUsingAbsoluteScale() const
        {
            return bAbsoluteScale;
        }

    protected:
        FTransform ComponentToWorld;

        USceneComponent *AttachParent = nullptr;
        std::vector<USceneComponent*> AttachChildren;

        FBoxSphereBounds Bounds;

    private:
        NPROPERTY()
        FVector RelativeLocation = FVector(0.0f, 0.0f, 0.0f);

        NPROPERTY()
        FRotator RelativeRotation = FRotator();

        NPROPERTY()
        FVector RelativeScale3D = FVector(1.0f, 1.0f, 1.0f);

 	    /** True if we have ever updated ComponentToWorld based on RelativeLocation/Rotation/Scale. Used at startup to make sure it is initialized. */
        bool bComponentToWorldUpdated = false;

        /** If RelativeLocation should be considered relative to the world, rather than the parent */
        bool bAbsoluteLocation = false;

        /** If RelativeRotation should be considered relative to the world, rather than the parent */
        bool bAbsoluteRotation = false;

        /** If RelativeScale3D should be considered relative to the world, rather than the parent */
        bool bAbsoluteScale = false;
    };
    
}