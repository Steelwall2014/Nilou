#pragma once
// #include "ClassInfo.h"
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <vector>

#include "Common/AssertionMacros.h"
#include "Common/Components/SceneComponent.h"
#include "Common/Components/ActorComponent.h"
#include "Common/Transform.h"
#include "Templates/TypeTraits.h"
#include "GameStatics.h"
// #include "Actor.generated.h"

namespace nilou {
	class UActorComponent;
	class USceneComponent;
	class UWorld;

	UCLASS()
	class AActor : public UObject
	{
		GENERATE_CLASS_INFO()
	public:

		AActor();

		virtual void Tick(double DeltaTime) {};

		// 下面两个函数都是获取Actor相对于世界的旋转
		glm::quat GetActorRotation() const;		// 以四元数表示
		nilou::FRotator GetActorRotator() const;		// 以欧拉角表示

		// 获取Actor相对于世界的位置
		glm::vec3 GetActorLocation() const;

		// 获取Actor在世界参考系下的前方向
		glm::vec3 GetActorForwardVector() const;
		// 获取Actor在世界参考系下的上方向
		glm::vec3 GetActorUpVector() const;
		// 获取Actor在世界参考系下的右方向
		glm::vec3 GetActorRightVector() const;

		// 设置Actor在世界参考系下的变换
		void SetActorTransform(const FTransform &InTransform);		// 以四元数表示

		// 设置Actor在世界参考系下的旋转
		void SetActorRotation(const glm::quat &rotation);		// 以四元数表示
		void SetActorRotator(const nilou::FRotator &rotator);		// 以欧拉角表示

		// 设置Actor在世界参考系下的位置
		void SetActorLocation(const glm::vec3 &location);

		void SetActorName(const std::string &InName);

		// 以上Get和Set都是通过操作Actor下RootComponent的变换实现的

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

		void AddOwnedComponent(UActorComponent *InComponent);

		void RemoveOwnedComponent(UActorComponent *InComponent);

		void SetRootComponent(std::shared_ptr<USceneComponent> InComponent);

		void RegisterAllComponents();

		template<class T>
		void GetComponents(std::vector<T *> &OutComponents, bool bIncludeFromChildActors = false)
		{
			OutComponents.clear();
			ForEachComponent_Internal<T>(T::StaticClass(), bIncludeFromChildActors, [&](T* InComp)
			{
				OutComponents.push_back(InComp);
			});
		}

		template<class ComponentType, typename Func>
		void ForEachComponent(Func InFunc, bool bIncludeFromChildActors=false)
		{
			ForEachComponent_Internal<ComponentType>(ComponentType::StaticClass(), bIncludeFromChildActors, InFunc);
		}

		void SetOwnedWorld(UWorld *InOwnedWorld)
		{
			OwnedWorld = InOwnedWorld;
		}

	protected:

		template<class ComponentType, typename Func>
		void ForEachComponent_Internal(const UClass *ComponentClass, bool bIncludeFromChildActors, Func InFunc)
		{
			static_assert(TIsDerivedFrom<ComponentType, UActorComponent>::Value, "'ComponentType' template parameter to ForEachComponent must be derived from UActorComponent");
			if (*ComponentClass == *UActorComponent::StaticClass())
			{
				if (bIncludeFromChildActors)
				{
					ForEachComponent_Internal<ComponentType, true /*bClassIsActorComponent*/, true /*bIncludeFromChildActors*/>(ComponentClass, InFunc);
				}
				else
				{
					ForEachComponent_Internal<ComponentType, true /*bClassIsActorComponent*/, false /*bIncludeFromChildActors*/>(ComponentClass, InFunc);
				}
			}
			else
			{
				if (bIncludeFromChildActors)
				{
					ForEachComponent_Internal<ComponentType, false /*bClassIsActorComponent*/, true /*bIncludeFromChildActors*/>(ComponentClass, InFunc);
				}
				else
				{
					ForEachComponent_Internal<ComponentType, false /*bClassIsActorComponent*/, false /*bIncludeFromChildActors*/>(ComponentClass, InFunc);
				}
			}
		}

		template<class ComponentType, bool bClassIsActorComponent, bool bIncludeFromChildActors, typename Func>
		void ForEachComponent_Internal(const UClass *ComponentClass, Func InFunc)
		{
			check(ComponentClass->IsChildOf(ComponentType::StaticClass()));
			if constexpr (bIncludeFromChildActors)
			{
				std::vector<AActor*> ChildActors;
				for (UActorComponent* OwnedComponent : OwnedComponents)
				{
					if (OwnedComponent)
					{
						if (bClassIsActorComponent || OwnedComponent->IsA(ComponentClass))
						{
							InFunc(static_cast<ComponentType*>(OwnedComponent));
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
				for (UActorComponent* OwnedComponent : OwnedComponents)
				{
					if (OwnedComponent)
					{
						if (bClassIsActorComponent || OwnedComponent->IsA(ComponentClass))
						{
							InFunc(static_cast<ComponentType*>(OwnedComponent));
						}
					}
				}
			}
		}

		std::string ActorName;
		std::shared_ptr<USceneComponent> RootComponent;
		std::set<UActorComponent *> OwnedComponents;
		UWorld *OwnedWorld;
	};
}