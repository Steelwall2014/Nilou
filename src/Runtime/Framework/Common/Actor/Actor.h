#pragma once
// #include "ClassInfo.h"
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <vector>

#include "Common/Log.h"
#include "Common/AssertionMacros.h"
#include "Common/Components/SceneComponent.h"
#include "Common/Components/ActorComponent.h"
#include "Common/Transform.h"
#include "Templates/TypeTraits.h"
#include "GameStatics.h"

namespace nilou {
	class UActorComponent;
	class USceneComponent;
	class UWorld;

	
	class NCLASS AActor : public NObject,
                   public std::enable_shared_from_this<AActor>
	{
		GENERATED_BODY()
	public:

		AActor();

		NPROPERTY()
		std::string ActorName;

		virtual void Tick(double DeltaTime) {};

		virtual void BeginPlay();

		virtual void OnConstruction(const FTransform &Transform) {}

		void PostSpawnInitialize(FTransform const& UserSpawnTransform);

		void FinishSpawning(const FTransform &UserTransform);

		void PostActorCreated() { }

		/** Returns whether an actor has been initialized for gameplay */
		bool IsActorInitialized() const { return bActorInitialized; }

		/** Returns whether an actor is in the process of beginning play */
		bool IsActorBeginningPlay() const { return ActorHasBegunPlay == EActorBeginPlayState::BeginningPlay; }

		/** Returns whether an actor has had BeginPlay called on it (and not subsequently had EndPlay called) */
		bool HasActorBegunPlay() const { return ActorHasBegunPlay == EActorBeginPlayState::HasBegunPlay; }

		// 下面两个函数都是获取Actor相对于世界的旋转
		dquat GetActorRotation() const;		// 以四元数表示
		FRotator GetActorRotator() const;		// 以欧拉角表示

		// 获取Actor相对于世界的位置
		dvec3 GetActorLocation() const;

		dvec3 GetActorScale3D() const;

		// 获取Actor在世界参考系下的前方向
		vec3 GetActorForwardVector() const;
		// 获取Actor在世界参考系下的上方向
		vec3 GetActorUpVector() const;
		// 获取Actor在世界参考系下的右方向
		vec3 GetActorRightVector() const;

		// 设置Actor在世界参考系下的变换
		void SetActorTransform(const FTransform &InTransform);		// 以四元数表示

		// 设置Actor在世界参考系下的旋转
		void SetActorRotation(const dquat &rotation);		// 以四元数表示
		void SetActorRotator(const nilou::FRotator &rotator);		// 以欧拉角表示

		// 设置Actor在世界参考系下的位置
		void SetActorLocation(const dvec3 &location);

		void SetActorScale3D(const dvec3 &scale);
		// 以上Get和Set都是通过操作Actor下RootComponent的变换实现的

		void SetActorName(const std::string &InName);


		// 获取根组件的裸指针
		inline USceneComponent *GetRootComponent() const { return RootComponent.get(); }

		inline UWorld *GetWorld() const { return OwnedWorld; }

		/**
		* Attaches the RootComponent of this Actor to the RootComponent of the supplied actor, optionally at a named socket.
		* @param ParentActor				Actor to attach this actor's RootComponent to
		* @param AttachmentRules			How to handle transforms and modification when attaching.
		*/
		void AttachToActor(AActor *ParentActor, const FAttachmentTransformRules& AttachmentRules);

		/**
		* Attaches the RootComponent of this Actor to the supplied component, optionally at a named socket. It is not valid to call this on components that are not Registered.
		* @param  Parent					Parent to attach to.
		* @param  AttachmentRules			How to handle transforms and welding when attaching.
		*/
		void AttachToComponent(USceneComponent *Parent, const FAttachmentTransformRules& AttachmentRules);

		void AddOwnedComponent(std::shared_ptr<UActorComponent> InComponent);

		void RemoveOwnedComponent(std::shared_ptr<UActorComponent> InComponent);

		void SetRootComponent(std::shared_ptr<USceneComponent> InComponent);

		void RegisterAllComponents();

		void InitializeComponents();

		void UninitializeComponents();

		template<class T>
		void GetComponents(std::vector<T*> &OutComponents, bool bIncludeFromChildActors = false)
		{
			OutComponents.clear();
			ForEachComponent_Internal<T>(T::StaticClass(), bIncludeFromChildActors, [&](std::shared_ptr<T> InComp)
			{
				OutComponents.push_back(InComp.get());
			});
		}

		template<class ComponentType, typename Func>
		void ForEachComponent(Func&& InFunc, bool bIncludeFromChildActors=false)
		{
			ForEachComponent_Internal<ComponentType>(ComponentType::StaticClass(), bIncludeFromChildActors, std::forward<Func>(InFunc));
		}

		void SetOwnedWorld(UWorld *InOwnedWorld)
		{
			OwnedWorld = InOwnedWorld;
		}

		bool IsValid() const { return !weak_from_this().expired(); }

	protected:

		template<class ComponentType, typename Func>
		void ForEachComponent_Internal(const NClass *ComponentClass, bool bIncludeFromChildActors, Func&& InFunc)
		{
			static_assert(TIsDerivedFrom<ComponentType, UActorComponent>::Value, "'ComponentType' template parameter to ForEachComponent must be derived from UActorComponent");
			if (*ComponentClass == *UActorComponent::StaticClass())
			{
				if (bIncludeFromChildActors)
				{
					ForEachComponent_Internal<ComponentType, true /*bClassIsActorComponent*/, true /*bIncludeFromChildActors*/>(ComponentClass, std::forward<Func>(InFunc));
				}
				else
				{
					ForEachComponent_Internal<ComponentType, true /*bClassIsActorComponent*/, false /*bIncludeFromChildActors*/>(ComponentClass, std::forward<Func>(InFunc));
				}
			}
			else
			{
				if (bIncludeFromChildActors)
				{
					ForEachComponent_Internal<ComponentType, false /*bClassIsActorComponent*/, true /*bIncludeFromChildActors*/>(ComponentClass, std::forward<Func>(InFunc));
				}
				else
				{
					ForEachComponent_Internal<ComponentType, false /*bClassIsActorComponent*/, false /*bIncludeFromChildActors*/>(ComponentClass, std::forward<Func>(InFunc));
				}
			}
		}

		template<class ComponentType, bool bClassIsActorComponent, bool bIncludeFromChildActors, typename Func>
		void ForEachComponent_Internal(const NClass *ComponentClass, Func&& InFunc)
		{
			check(ComponentClass->IsChildOf(ComponentType::StaticClass()));
			if constexpr (bIncludeFromChildActors)
			{
				std::vector<AActor*> ChildActors;
				for (std::shared_ptr<UActorComponent> OwnedComponent : OwnedComponents)
				{
					if (OwnedComponent)
					{
						if (bClassIsActorComponent || OwnedComponent->IsA(ComponentClass))
						{
							InFunc(std::static_pointer_cast<ComponentType>(OwnedComponent));
						}
					}
				}

				for (AActor* ChildActor : ChildActors)
				{
					ChildActor->ForEachComponent_Internal<ComponentType, bClassIsActorComponent, bIncludeFromChildActors>(ComponentClass, InFunc);
				}
			}
			else
			{
				for (std::shared_ptr<UActorComponent> OwnedComponent : OwnedComponents)
				{
					if (OwnedComponent)
					{
						if (bClassIsActorComponent || OwnedComponent->IsA(ComponentClass))
						{
							InFunc(std::static_pointer_cast<ComponentType>(OwnedComponent));
						}
					}
				}
			}
		}

		std::shared_ptr<USceneComponent> RootComponent;
		std::set<std::shared_ptr<UActorComponent>> OwnedComponents;
		UWorld *OwnedWorld;
		bool bActorInitialized;

		enum class EActorBeginPlayState : uint8
		{
			HasNotBegunPlay,
			BeginningPlay,
			HasBegunPlay,
		};
		/** 
		*	Indicates that BeginPlay has been called for this Actor.
		*  Set back to HasNotBegunPlay once EndPlay has been called.
		*/
		EActorBeginPlayState ActorHasBegunPlay;

	private:

		void ExecuteConstruction(const FTransform& Transform);

		void PostActorConstruction();
	};
}