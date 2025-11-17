#ifndef VHM_WORLDLODPARAM_H
#define VHM_WORLDLODPARAM_H

struct WorldLODParam
{
    float NodeMeterSize_x;
    float NodeMeterSize_y;
    uint NodeSideNum_x;
    uint NodeSideNum_y;
    uint NodeDescriptionIndexOffset;
};

uint fromNodeLoctoNodeDescriptionIndex(uvec2 nodeLoc, uint lod, const WorldLODParam param)
{
    return param.NodeDescriptionIndexOffset + nodeLoc.x * param.NodeSideNum_y + nodeLoc.y;
}

#endif