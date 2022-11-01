#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Interface/IDrawPass.h"
#include "OpenGL/OpenGLComputeBuffer.h"
#include "OpenGL/OpenGLTexture.h"
#include "Common/GfxStructures.h"
#include "OpenGL/OpenGLIndirectBuffer.h"
#include "RHIResources.h"


// 是否使用CPU生成node list
// !!注意!! CPU生成node list的话海底地形是无法正确计算node到相机距离的，
// 这是这一步需要读取高度图得到node中心点高度，但是从显存读取高度图到CPU端没有实现，所以这一步中的高度写死为0
//#define CPU_NODELIST

//#define DIRECT_DISPATCH

// 如果用cpu生成node list的话就不能用indirect draw了
#if defined(CPU_NODELIST) && !defined(DIRECT_DISPATCH)
#define DIRECT_DISPATCH
#endif // CPU_NODELIST

//#define SHOW_PATCH_COUNT

namespace und {
	class QuadTree;
	class QuadTreeUpdateSubPass : implements ISubPass
	{
	private:
		RHIBufferRef m_NodeIDs_TempA;
		RHIBufferRef m_NodeIDs_TempB;
		RHIBufferRef m_FinalNodeListBuffer;
		RHIBufferRef m_FinalNodeListIndirectArgs;
		unsigned int m_FinalNodeListSize;
		RHIBufferRef m_LODParamsBuffer;
		RHIBufferRef m_NodeDescriptionBuffer;
		std::vector<glm::uvec3> m_NodeList_Final;
		unsigned int LODMapSize;
		unsigned int NodeIDs_Temp_Maxlength = 0, NodeIDs_Final_Maxlength = 0;

		unsigned int *NodeDescription;

		void CreateNodeList(FrameVariables &frame, QuadTree &Tree);
		void CreateNodeListCPU(const glm::vec3 &cameraPos, const QuadTree &Tree);

		void CreateLODMap(FrameVariables &frame, const QuadTree &Tree);
		void CreateBoundingBox(const QuadTree &Tree, RHITexture2DRef HeightMap, float HeightMapMeterSize, ETextureWrapModes HeightMapWrapMode);
		void CreatePatch(FrameVariables &frame, const QuadTree &Tree, RHITexture2DRef DisplacementMap, float HeightMapMeterSize, ETextureWrapModes HeightMapWrapMode);
	public:
		// MinMaxMap[n]代表LOD为n的patch的minmax值，
		// 比如，如果四叉树有6个LOD级别，最高的LOD级别有5*5个node，那么MinMaxMap[5]的大小就是(5*8)*(5*8)=40*40(一个node对应8*8个patch)，
		// MinMaxMap[0]的大小是(160*8)*(160*8)=1280*1280
		// 如果有N个LOD级别，那么这个MinMaxMap就有N+3个，这样MinMaxMap也可以用来表示node的minmax值，只需要取LOD+3处的那个Map就行
		std::vector<RHITexture2DRef> MinMaxMap;
		RHIBufferRef CulledPatchListBuffer;
		RHIBufferRef AtomicPatchCounterBuffer;
		RHITexture2DRef LODMap;
		unsigned int AtomicPatchCounter;
		int Initialize(const QuadTree &Tree);
		virtual void DrawOrCompute(FrameVariables &frame, QuadTree &Tree, RHITexture2DRef HeightMap, RHITexture2DRef DisplacementMap, float HeightMapMeterSize, ETextureWrapModes HeightMapWrapMode);
		~QuadTreeUpdateSubPass();
	};
}