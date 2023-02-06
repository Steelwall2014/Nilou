#pragma once

#include <memory>
#include <string>

#include "Common/CoreUObject/Object.h"
// #include "ActorComponent.generated.h"

namespace nilou {
    class AActor;
    class UWorld;

    UCLASS()
    class UActorComponent : public UObject
    {
        GENERATE_CLASS_INFO()
    public:
    
        UActorComponent(AActor *InOwner = nullptr);

        UWorld *GetWorld() const;

        AActor *GetOwner() const;

        bool IsRegistered() const;

        void SetOwner(AActor *InOwner);

        /** 重新计算transform，这里是空实现 */
        virtual void UpdateComponentToWorld() {}

        // 注册组件，创建渲染状态/物理状态。
        void RegisterComponent();
        void RegisterComponentWithWorld(UWorld *World);
        // It will be called from RegisterComponent
        virtual void CreateRenderState();

        void UnregisterComponent();
        // It will be called from UnregisterComponent
        virtual void DestroyRenderState();

        virtual void DestroyComponent();

        virtual void DoDeferredRenderUpdates();
        // It will be called from DoDeferredRenderUpdates
        virtual void RecreateRenderState();
        // It will be called from DoDeferredRenderUpdates
        virtual void SendRenderTransform();
        // It will be called from DoDeferredRenderUpdates
        virtual void SendRenderDynamicData();
        // It will be called from DoDeferredRenderUpdates
        virtual void SendRenderInstanceData();

        /** Mark the render state as dirty - will be sent to the render thread at the end of the frame. */
        void MarkRenderStateDirty();

        /** Marks the transform as dirty - will be sent to the render thread at the end of the frame*/
        void MarkRenderTransformDirty();

        /** Marks the dynamic data as dirty - will be sent to the render thread at the end of the frame*/
        void MarkRenderDynamicDataDirty();

        /** Marks the instances as dirty - changes to instance transforms/custom data will be sent to the render thread at the end of the frame*/
        void MarkRenderInstancesDirty();

    protected:
        AActor *OwnedActor;
        UWorld *WorldPrivate;
        bool bRegistered;
	    bool bRenderStateCreated;
	    bool bRenderStateDirty;
	    bool bRenderTransformDirty;
        bool bRenderDynamicDataDirty;
        bool bRenderInstancesDirty;

    };
}