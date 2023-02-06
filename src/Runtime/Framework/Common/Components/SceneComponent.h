#pragma once
#include <memory>
#include <string>
#include <vector>

#include "ActorComponent.h"
#include "Frustum.h"
#include "Platform.h"
#include "Common/Transform.h"
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

    UCLASS()
    class USceneComponent : public UActorComponent
    {
        GENERATE_CLASS_INFO()
    public:
        USceneComponent(AActor *InOwner = nullptr) 
            : UActorComponent(InOwner)
            , AttachParent(nullptr)
        { }

        /** Calculate the bounds of the component. Default behavior is a bounding box/sphere of zero size. */
        virtual FBoundingBox CalcBounds(const FTransform& LocalToWorld) const
        {
            FBoundingBox Bounds;
            Bounds.Min = Bounds.Max = LocalToWorld.GetTranslation();
            return Bounds;
        }

        /** Calculate the local bounds of the component. Default behavior is calling CalcBounds with an identity transform. */
        virtual FBoundingBox CalcLocalBounds() const 
        { 
            return CalcBounds(FTransform::Identity);
        }

        virtual void UpdateBounds()
        {
            Bounds = CalcBounds(GetComponentTransform());
        }

	    FBoundingBox GetBounds() const
        {
            return Bounds;
        }

        FTransform CalcNewComponentToWorld(const FTransform &NewRelTransform);

        void SetRelativeLocation(const vec3 &NewLocation);
        void SetRelativeRotation(const quat &NewRotation);
        void SetRelativeRotation(const FRotator &NewRotation);
	    void SetRelativeScale3D(const vec3 &NewScale3D);
        void SetRelativeTransform(const FTransform &NewTransform);
        void SetRelativeLocationAndRotation(const vec3 &NewLocation, const quat &NewRotation);
        void SetRelativeLocationAndRotation(const vec3 &NewLocation, const FRotator &NewRotation);

        vec3 GetRelativeLocation() const { return RelativeLocation; }
        FRotator GetRelativeRotation() const { return RelativeRotation; }
        vec3 GetRelativeScale3D() const { return RelativeScale3D; }
        
	    FTransform GetRelativeTransform() const;

        void SetWorldLocation(const vec3 &NewLocation);
        void SetWorldRotation(const quat &NewRotation);
        void SetWorldRotation(const FRotator &NewRotation);
	    void SetWorldScale3D(const vec3 &NewScale);
        void SetWorldTransform(const FTransform &NewTransform);
        void SetWorldLocationAndRotation(vec3 NewLocation, const quat &NewRotation);
        void SetWorldLocationAndRotation(vec3 NewLocation, FRotator NewRotation);

        /** Currently not implemented*/
        std::string GetAttachSocketName() { return ""; }
        /** Currently not implemented*/
        FTransform GetSocketTransform(const std::string &) { return GetComponentTransform(); }

        /**  
        * Convenience function to get the relative rotation from the passed in world rotation
        * @param WorldRotation  World rotation that we want to convert to relative to the components parent
        * @return Returns the relative rotation
        */
	    quat GetRelativeRotationFromWorld(const quat &NewRotation);

        inline vec3 GetComponentLocation() const
        {
            return GetComponentTransform().GetLocation();
        }
        inline FRotator GetComponentRotation() const
        {
            return GetComponentTransform().GetRotator();
        }
        inline quat GetComponentQuat() const
        {
            return GetComponentTransform().GetRotation();
        }
        inline vec3 GetComponentScale() const
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

        // // 获取局部变换
        // FTransform GetRelativeTransform() const;


        // // 获取相对于世界参考系的位置，通过调用GetWorldTransform实现
        // vec3 GetComponentLocation() const;

        // void MoveComponentTo(const vec3 &Position);
        // 在世界参考系中移动node，将node在世界参考系中的旋转设置为NewRotation
        void MoveComponent(const vec3 &Delta, const FRotator &NewRotation);
        void MoveComponent(const vec3 &Delta, const quat &NewRotation);

        // 获取这个节点的forward、up和right向量，在世界坐标系
        vec3 GetForwardVector();
        vec3 GetUpVector();
        vec3 GetRightVector();

        // 更新组件的transform
        void UpdateComponentToWorld();
        void UpdateChildTransforms();

        void AttachToComponent(USceneComponent *Parent, const FAttachmentTransformRules &AttachmentRules=FAttachmentTransformRules::KeepRelativeTransform);
        // void AttachToActor(AActor *Parent, const FAttachmentTransformRules &AttachmentRules);

        USceneComponent *GetAttachParent() { return AttachParent; }

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
        std::vector<USceneComponent *> AttachChildren;

        FBoundingBox Bounds;

    private:
        vec3 RelativeLocation = vec3(0.0f, 0.0f, 0.0f);

        FRotator RelativeRotation = FRotator();

        vec3 RelativeScale3D = vec3(1.0f, 1.0f, 1.0f);

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