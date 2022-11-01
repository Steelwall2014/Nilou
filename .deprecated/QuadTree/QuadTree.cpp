#include <queue>
#include <iostream>
#include "QuadTree.h"

#include "OpenGL/OpenGLComputeBuffer.h"
#include "OpenGL/OpenGLTexture.h"
#include "Common/GfxStructures.h"

#include "Common/DrawPass/QuadTreeSubPass.h"

unsigned int fromNodeLoctoNodeDescriptionIndex(glm::uvec2 nodeLoc, unsigned int lod, const und::WorldLODParam &param)
{
    return param.NodeDescriptionIndexOffset + nodeLoc.x * param.NodeSideNum + nodeLoc.y;
}
glm::uvec2 fromSectorLoctoNodeLoc(glm::uvec2 sector_loc, unsigned int lod)
{
    return sector_loc / (unsigned int)(pow(2, lod));
}

float und::QuadTree::GetNodeSize(unsigned int lod) const
{
    return LODParams[lod].NodeMeterSize;
}
glm::vec3 und::QuadTree::GetNodePositionWS(glm::uvec2 nodeLoc, unsigned int  lod) const
{
    // 这里没有实现从显存读取高度图，所以z是0
    glm::vec2 xy_center_coord = (glm::vec2(nodeLoc) + glm::vec2(0.5)) * LODParams[lod].NodeMeterSize;
    return glm::vec3(xy_center_coord, 0);
}
float und::QuadTree::GetOriginalPatchMeterSize()
{
    return LODParams[0].NodeMeterSize / 8.0;    // 固定一个node有8*8个patch
}
und::QuadTree::~QuadTree()
{
}

und::QuadTree::QuadTree(unsigned int LODNum, unsigned int TopLODNodeSideNum, float TopLODNodeMeterSize)
{   
    assert(MIN_LOD_NUM < LODNum && LODNum < MAX_LOD_NUM);
    // WorldOrigin是指左上角，瓦片坐标方向为从左上到右下
    // LOD0是最精细的
    this->LODNum = LODNum;
    LODParams.resize(LODNum);
    NodeNum = 0;
    for (int lod = LODNum -1; lod >= 0; lod--)
    {
        WorldLODParam param;
        param.NodeMeterSize = TopLODNodeMeterSize / pow(2, LODNum - 1 - lod);
        param.NodeSideNum = TopLODNodeSideNum * pow(2, LODNum-1-lod);
        LODParams[lod] = param;
        NodeNum += param.NodeSideNum * param.NodeSideNum;
    }
    LODParams[0].NodeDescriptionIndexOffset = 0;
    for (int lod = 1; lod < LODNum; lod++)
    {
        LODParams[lod].NodeDescriptionIndexOffset = LODParams[lod - 1].NodeDescriptionIndexOffset + LODParams[lod-1].NodeSideNum * LODParams[lod-1].NodeSideNum;
    }
    WorldMeterSize = TopLODNodeSideNum * TopLODNodeMeterSize;
    LODMapSize = TopLODNodeSideNum * pow(2, LODNum - 1);
}
