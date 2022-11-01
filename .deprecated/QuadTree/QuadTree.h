#pragma once
//#include "QuadTreeStructures.h"
#include <memory>
#include <vector>

#include <glm/glm.hpp>

#define MAX_LOD_NUM 12
#define MIN_LOD_NUM 3

namespace und {

	// class OpenGLComputeBuffer;
	// typedef std::shared_ptr<OpenGLComputeBuffer> OpenGLComputeBufferRef;
	// class OpenGLTexture2D;
	// typedef std::shared_ptr<OpenGLTexture2D> OpenGLTexture2DRef;

	class FrameVariables;
	struct WorldLODParam
	{
		float NodeMeterSize;
		unsigned int NodeSideNum;
		unsigned int NodeDescriptionIndexOffset;	// 用来把一个node id(i, j, lod)转换到一维
	};
	struct RenderPatch
	{
		glm::uvec4 DeltaLod;
		glm::vec2 offset;
		unsigned int lod;
		RenderPatch() : DeltaLod{0, 0, 0, 0}, offset { 0, 0 }, lod(0) {}
	};
	class QuadTree
	{
	public:
		std::vector<WorldLODParam> LODParams;
		unsigned int LODNum;
		unsigned int LODMapSize;
		unsigned int NodeNum;
		float WorldMeterSize;

		// 这里TopLODNodeSideNum * pow(2, LODNum - 1)不能小于32，因为要计算LOD Map。LOD Map的最小边长是32，因为是写死在shader中的		QuadTree(unsigned int LODNum, unsigned int TopLODNodeSideNum, float TopLODNodeMeterSize);
		QuadTree(unsigned int LODNum, unsigned int TopLODNodeSideNum, float TopLODNodeMeterSize);
		float		GetNodeSize(unsigned int lod) const;
		glm::vec3	GetNodePositionWS(glm::uvec2 nodeLoc, unsigned int lod) const;
		float		GetOriginalPatchMeterSize();
		~QuadTree();
	};
}