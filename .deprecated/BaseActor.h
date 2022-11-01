#pragma once
#include <ostream>
#include "Common/BaseSceneNode.h"

namespace und {
	class BaseActor
	{
	protected:
		std::shared_ptr<BaseSceneNode> m_pRootSceneNode;
	public:
		bool bFindMainCamemraWhenMove = true;
		virtual void Tick(double DeltaTime) {};

		// 下面两个函数都是获取Actor相对于世界的旋转
		glm::quat GetActorRotation();		// 以四元数表示
		und::Rotator GetActorRotator();		// 以欧拉角表示

		// 获取Actor相对于世界的位置
		glm::vec3 GetActorLocation();

		// 获取Actor在世界参考系下的前方向
		glm::vec3 GetActorForwardVector();
		// 获取Actor在世界参考系下的上方向
		glm::vec3 GetActorUpVector();
		// 获取Actor在世界参考系下的右方向
		glm::vec3 GetActorRightVector();

		// 设置Actor在世界参考系下的旋转
		void SetActorRotation(const glm::quat &rotation);		// 以四元数表示
		void SetActorRotator(const und::Rotator &rotator);		// 以欧拉角表示

		// 设置Actor在世界参考系下的位置
		void SetActorLocation(const glm::vec3 &location);

		// 以上Get和Set都是通过操作Actor下m_pRootSceneNode的变换实现的

		std::shared_ptr<BaseSceneNode> GetRootSceneNode();
	};
}