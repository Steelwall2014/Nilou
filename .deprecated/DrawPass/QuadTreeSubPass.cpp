#include "Common/ShaderManager.h"
#include "Common/SceneManager.h"
#include "RHIDefinitions.h"
#include "DynamicRHI.h"
#include "QuadTreeSubPass.h"

constexpr float MAX_NODE_LENGTH_MULT = 2;
namespace und {
    unsigned int fromNodeLoctoNodeDescriptionIndex(glm::uvec2 nodeLoc, unsigned int lod, const und::WorldLODParam &param)
    {
        return param.NodeDescriptionIndexOffset + nodeLoc.x * param.NodeSideNum + nodeLoc.y;
    }
    bool EvaluateNode(const QuadTree &Tree, glm::vec3 cameraPos, glm::uvec2 nodeLoc, unsigned int lod) {
        glm::vec3 positionWS = Tree.GetNodePositionWS(nodeLoc, lod);
        float dis = glm::distance(cameraPos, positionWS);
        float nodeSize = Tree.GetNodeSize(lod);
        float f = dis / (nodeSize * 1.5);
        if (f < 1)
            return true;
        return false;
    }

    int QuadTreeUpdateSubPass::Initialize(const QuadTree &Tree)
    {
        LODMapSize = Tree.LODMapSize;
        _GM->GLDEBUG();
        LODMap = _GM->RHICreateTexture2D("LODMap", EPixelFormat::PF_R16F, 1, LODMapSize, LODMapSize, nullptr);
        _GM->GLDEBUG();

#ifdef CPU_NODELIST
        NodeDescription = new unsigned int[Tree.NodeNum];
#endif // CPU_NODELIST

        char texname[32];
        std::vector<WorldLODParam> LODParams = Tree.LODParams;
        m_LODParamsBuffer = _GM->RHICreateShaderStorageBuffer(LODParams.size() * sizeof(und::WorldLODParam), LODParams.data());
        for (int LOD = 0; LOD < Tree.LODNum; LOD++)
        {
            int size = Tree.LODParams[LOD].NodeSideNum * 8;
            sprintf(texname, "MinMaxMap[%d]", LOD);
            MinMaxMap.push_back(_GM->RHICreateTexture2D(texname, EPixelFormat::PF_R32G32F, 1, size, size, nullptr));
            _GM->GLDEBUG();
        }
        for (int LOD = Tree.LODNum; LOD < Tree.LODNum+3; LOD++)
        {
            int size = Tree.LODParams[LOD-3].NodeSideNum;
            sprintf(texname, "MinMaxMap[%d]", LOD);
            MinMaxMap.push_back(_GM->RHICreateTexture2D(texname, EPixelFormat::PF_R32G32F, 1, size, size, nullptr));
            _GM->GLDEBUG();
        }
        m_NodeDescriptionBuffer = _GM->RHICreateShaderStorageBuffer(Tree.NodeNum * 4, 0);

        // 由于opengl并不支持在运行时动态分配内存(至少我没有找到)，也就实现不了push_back之类的功能
        // 因此需要预先分配空间，再使用一个atomic counter来间接实现这种功能，就需要预先计算最大长度。
        // 下面是为了计算NodeList TempA、NodeList TempB和NodeList Final最大的长度
        // 显然当摄像机在平面正中央，而且正好在平面上时，NodeList会是最长的
        glm::vec3 cameraPos(Tree.WorldMeterSize / 2, Tree.WorldMeterSize / 2, 0);
        CreateNodeListCPU(cameraPos, Tree);     

        m_FinalNodeListBuffer = _GM->RHICreateShaderStorageBuffer(NodeIDs_Final_Maxlength, 0);
        m_NodeIDs_TempA = _GM->RHICreateShaderStorageBuffer(NodeIDs_Temp_Maxlength, 0);
        m_NodeIDs_TempB = _GM->RHICreateShaderStorageBuffer(NodeIDs_Temp_Maxlength, 0);

        m_FinalNodeListIndirectArgs = _GM->RHICreateDispatchIndirectBuffer(0, 1, 1);

        CulledPatchListBuffer = _GM->RHICreateShaderStorageBuffer(NodeIDs_Final_Maxlength * 64 * sizeof(und::RenderPatch), 0);
        return 0;
    }

    void QuadTreeUpdateSubPass::CreateBoundingBox(const QuadTree &Tree, RHITexture2DRef HeightMap, float HeightMapMeterSize, ETextureWrapModes HeightMapWrapMode)
    {
        _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("QuadTree Create MinMaxMap First"));

        _GM->RHISetShaderParameter("PatchOriginalMeterSize", Tree.LODParams[0].NodeMeterSize / 8);
        _GM->RHISetShaderParameter("HeightMapMeterSize", HeightMapMeterSize);
        RHITextureParams params;
        params.Wrap_S = HeightMapWrapMode;
        params.Wrap_T = HeightMapWrapMode;
        _GM->RHIBindTexture("HeightMap", HeightMap, params);
        _GM->RHIBindTexture(2, MinMaxMap[0], EDataAccessFlag::DA_WriteOnly);
        int group_num = Tree.LODParams[0].NodeSideNum;     // 一个work group负责一个node
        _GM->RHIDispatch(group_num, group_num, 1);
        _GM->RHIImageMemoryBarrier();


        _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("QuadTree Create MinMaxMap"));
        for (int LOD = 1; LOD < Tree.LODNum; LOD++)
        {
            _GM->RHIBindTexture(1, MinMaxMap[LOD - 1], EDataAccessFlag::DA_ReadOnly);
            _GM->RHIBindTexture(2, MinMaxMap[LOD], EDataAccessFlag::DA_WriteOnly);
            int group_num = Tree.LODParams[LOD].NodeSideNum;     // 一个work group负责一个node
            _GM->RHIDispatch(group_num, group_num, 1);
            _GM->RHIImageMemoryBarrier();
        }
        _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("QuadTree Create MinMaxMap Node"));
        for (int LOD = Tree.LODNum; LOD < Tree.LODNum+3; LOD++)
        {
            _GM->RHIBindTexture(1, MinMaxMap[LOD - 1], EDataAccessFlag::DA_ReadOnly);
            _GM->RHIBindTexture(2, MinMaxMap[LOD], EDataAccessFlag::DA_WriteOnly);
            int group_num = Tree.LODParams[LOD-3].NodeSideNum;     // 一个work group负责一个node
            _GM->RHIDispatch(group_num, group_num, 1);
            _GM->RHIImageMemoryBarrier();
        }

    }

    void QuadTreeUpdateSubPass::CreateNodeListCPU(const glm::vec3 &cameraPos, const QuadTree &Tree)
    {

        int TopLODNodeSideNum = Tree.LODParams[Tree.LODNum - 1].NodeSideNum;
        std::vector<glm::uvec2> NodeIDs_TempA;
        std::vector<glm::uvec2> NodeIDs_TempB;
        m_NodeList_Final.clear();
        for (int i = 0; i < TopLODNodeSideNum; i++)
        {
            for (int j = 0; j < TopLODNodeSideNum; j++)
            {
                NodeIDs_TempA.push_back(glm::uvec2(i, j));
            }
        }
        NodeIDs_Temp_Maxlength = std::max(NodeIDs_Temp_Maxlength, (unsigned int)NodeIDs_TempA.size());
        for (int lod = Tree.LODNum - 1; lod >= 0; lod--)
        {
            while (!NodeIDs_TempA.empty())
            {
                glm::uvec2 nodeLoc = NodeIDs_TempA.back(); NodeIDs_TempA.pop_back();
                unsigned int index = fromNodeLoctoNodeDescriptionIndex(nodeLoc, lod, Tree.LODParams[lod]);
                if (lod > 0 && EvaluateNode(Tree, cameraPos, nodeLoc, lod)) {
                    //divide
                    NodeIDs_TempB.push_back(nodeLoc * (unsigned int)2);
                    NodeIDs_TempB.push_back(nodeLoc * (unsigned int)2 + glm::uvec2(1, 0));
                    NodeIDs_TempB.push_back(nodeLoc * (unsigned int)2 + glm::uvec2(0, 1));
                    NodeIDs_TempB.push_back(nodeLoc * (unsigned int)2 + glm::uvec2(1, 1));

#ifdef CPU_NODELIST
                    NodeDescription[index] = true;
#endif // CPU_NODELIST

                }
                else {
                    m_NodeList_Final.push_back(glm::uvec3(nodeLoc, lod));

#ifdef CPU_NODELIST
                    NodeDescription[index] = false;
#endif // CPU_NODELIST

                }
            }
            NodeIDs_Temp_Maxlength = std::max(NodeIDs_Temp_Maxlength, (unsigned int)NodeIDs_TempB.size());
            std::swap(NodeIDs_TempA, NodeIDs_TempB);
        }
        m_FinalNodeListSize = NodeIDs_Final_Maxlength = m_NodeList_Final.size();

        // 留一点冗余
        NodeIDs_Temp_Maxlength *= sizeof(glm::uvec2) * MAX_NODE_LENGTH_MULT;
        NodeIDs_Final_Maxlength *= sizeof(glm::uvec3) * MAX_NODE_LENGTH_MULT;
    }

    void QuadTreeUpdateSubPass::CreateNodeList(FrameVariables &frame, QuadTree &Tree)
    {
        int TopLODNodeSideNum = Tree.LODParams[Tree.LODNum-1].NodeSideNum;
        unsigned int index_b_value = TopLODNodeSideNum * TopLODNodeSideNum;

        RHIBufferRef IndexB = _GM->RHICreateAtomicCounterBuffer(0);
        RHIBufferRef IndexFinal = _GM->RHICreateAtomicCounterBuffer(0);

#ifndef DIRECT_DISPATCH
        RHIBufferRef indirectArgs = _GM->RHICreateDispatchIndirectBuffer(index_b_value, 1, 1);
#endif // !DIRECT_DISPATCH

        _GM->GLDEBUG();
        _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("QuadTree Create NodeList"));
        _GM->GLDEBUG();
        _GM->RHIBindComputeBuffer(2, m_FinalNodeListBuffer);
        _GM->GLDEBUG();
        _GM->RHIBindComputeBuffer(4, IndexFinal);
        _GM->GLDEBUG();
        _GM->RHIBindComputeBuffer(5, m_LODParamsBuffer);
        _GM->RHIBindComputeBuffer(6, m_NodeDescriptionBuffer);
        _GM->GLDEBUG();
        _GM->RHISetShaderParameter("MaxLOD", Tree.LODNum-1);
        _GM->RHISetShaderParameter("cameraPos", frame.frameContext.cameraPosition);
        _GM->GLDEBUG();
        char uniformName[256];
        RHITextureParams params;
        params.Mag_Filter = ETextureFilters::TF_Nearest;
        params.Min_Filter = ETextureFilters::TF_Nearest;
        for (int LOD = 0; LOD < MinMaxMap.size(); LOD++)
        {
            sprintf(uniformName, "MinMaxMap[%d]", LOD);
            _GM->RHIBindTexture(uniformName, MinMaxMap[LOD], params);
        }

        //und::Frustum &frustum = frame.frameContext.cameraFrustum;
        //for (int i = 0; i < 6; i++)
        //{
        //    sprintf(uniformName, "FrustumPlanes[%d]", i);
        //    _GM->RHISetShaderParameter(uniformName, frustum.Planes[i]);
        //}

        for (int lod = Tree.LODNum - 1; lod >= 0; lod--)
        {
            _GM->RHISetShaderParameter("PassLOD", (unsigned int)lod);
            _GM->RHIBindComputeBuffer(0, m_NodeIDs_TempA);
            _GM->RHIBindComputeBuffer(1, m_NodeIDs_TempB);
            _GM->RHIBindComputeBuffer(3, IndexB);

#ifdef DIRECT_DISPATCH
            _GM->RHIDispatch(index_b_value, 1, 1);
#else
            _GM->RHIDispatchIndirect(indirectArgs);
#endif // DIRECT_DISPATCH

            _GM->RHIStorageMemoryBarrier();

#ifdef DIRECT_DISPATCH
            unsigned int *ptr = (unsigned int *)_GM->RHIMapComputeBuffer(IndexB, EDataAccessFlag::DA_ReadOnly);
            index_b_value = *ptr;
            _GM->RHIUnmapComputeBuffer(IndexB);
#else
            _GM->RHICopyBufferSubData(IndexB, indirectArgs, 0, 0, 4);
#endif // DIRECT_DISPATCH

            std::swap(m_NodeIDs_TempA, m_NodeIDs_TempB);
            IndexB = _GM->RHICreateAtomicCounterBuffer(0);
            _GM->GLDEBUG();
        }

#ifdef DIRECT_DISPATCH
        unsigned int *ptr = (unsigned int *)_GM->RHIMapComputeBuffer(IndexFinal, EDataAccessFlag::DA_ReadOnly);
        m_FinalNodeListSize = *ptr;
        _GM->RHIUnmapComputeBuffer(IndexFinal);
#else
        _GM->RHICopyBufferSubData(IndexFinal, m_FinalNodeListIndirectArgs, 0, 0, 4);
#endif // DIRECT_DISPATCH

    }

    void QuadTreeUpdateSubPass::CreateLODMap(FrameVariables &frame, const QuadTree &Tree)
    {
        _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("QuadTree Create LOD Map"));


#ifdef CPU_NODELIST
        m_NodeDescriptionBuffer = _GM->RHICreateShaderStorageBuffer(Tree.NodeNum * 4, NodeDescription);
#endif // CPU_NODELIST
        _GM->RHIBindComputeBuffer(1, m_NodeDescriptionBuffer);

        _GM->RHIBindComputeBuffer(2, m_LODParamsBuffer);

        _GM->RHIBindTexture(3, LODMap, EDataAccessFlag::DA_WriteOnly);
        _GM->RHISetShaderParameter("LODNum", Tree.LODNum);

        unsigned int group_num = LODMapSize / 32;
        _GM->RHIDispatch(group_num, group_num, 1);
        _GM->RHIImageMemoryBarrier();

    }

    void QuadTreeUpdateSubPass::CreatePatch(FrameVariables &frame, const QuadTree &Tree, RHITexture2DRef DisplacementMap, float HeightMapMeterSize, ETextureWrapModes HeightMapWrapMode)
    {
        _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("QuadTree Create Patch"));

#ifdef CPU_NODELIST
        m_FinalNodeListBuffer = _GM->RHICreateShaderStorageBuffer(m_FinalNodeListSize * sizeof(glm::uvec3), m_NodeList_Final.data());
#endif // CPU_NODELIST
        _GM->RHIBindComputeBuffer(0, m_FinalNodeListBuffer);

        _GM->RHIBindComputeBuffer(1, CulledPatchListBuffer);

        _GM->RHIBindComputeBuffer(2, m_LODParamsBuffer);

        AtomicPatchCounterBuffer = _GM->RHICreateAtomicCounterBuffer(0);
        _GM->RHIBindComputeBuffer(3, AtomicPatchCounterBuffer);

        _GM->RHIBindTexture(4, LODMap, EDataAccessFlag::DA_WriteOnly);

        char uniformName[256];
        RHITextureParams params;
        params.Mag_Filter = ETextureFilters::TF_Nearest;
        params.Min_Filter = ETextureFilters::TF_Nearest;
        for (int LOD = 0; LOD < MinMaxMap.size(); LOD++)
        {
            sprintf(uniformName, "MinMaxMap[%d]", LOD);
            _GM->RHIBindTexture(uniformName, MinMaxMap[LOD], params);
        }

        und::Frustum &frustum = frame.frameContext.cameraFrustum;
        for (int i = 0; i < 6; i++)
        {
            sprintf(uniformName, "FrustumPlanes[%d]", i);
            _GM->RHISetShaderParameter(uniformName, frustum.Planes[i]);
        }
        _GM->GLDEBUG();
        _GM->RHISetShaderParameter("LODMapSize", Tree.LODParams[0].NodeSideNum);

        _GM->RHISetShaderParameter("DisplacementMapMeterSize", HeightMapMeterSize);
        if (DisplacementMap != nullptr)
        {
          RHITextureParams params;
          params.Wrap_S = HeightMapWrapMode;
          params.Wrap_T = HeightMapWrapMode;
          _GM->RHIBindTexture("DisplacementMap", DisplacementMap, params);
        }
#ifdef DIRECT_DISPATCH
        _GM->RHIDispatch(m_FinalNodeListSize, 1, 1);
#else
        _GM->RHIDispatchIndirect(m_FinalNodeListIndirectArgs);
#endif // DIRECT_DISPATCH

        _GM->RHIStorageMemoryBarrier();

#if defined(DIRECT_DISPATCH) || defined(SHOW_PATCH_COUNT)
        unsigned int *ptr = (unsigned int *)_GM->RHIMapComputeBuffer(AtomicPatchCounterBuffer, EDataAccessFlag::DA_ReadOnly);
        AtomicPatchCounter = *ptr;
        _GM->RHIUnmapComputeBuffer(AtomicPatchCounterBuffer);
#ifdef SHOW_PATCH_COUNT
        std::cout << AtomicPatchCounter << std::endl;
#endif // SHOW_PATCH_COUNT

#endif // DIRECT_DISPATCH

    }
    void QuadTreeUpdateSubPass::DrawOrCompute(FrameVariables &frame, QuadTree &Tree, 
        RHITexture2DRef HeightMap, RHITexture2DRef DisplacementMap, float HeightMapMeterSize, ETextureWrapModes HeightMapWrapMode)
    {
        CreateBoundingBox(Tree, HeightMap, HeightMapMeterSize, HeightMapWrapMode);

#ifdef CPU_NODELIST
        CreateNodeListCPU(frame.frameContext.cameraPosition, Tree);
#else
        CreateNodeList(frame, Tree);
#endif // CPU_NODELIST

        CreateLODMap(frame, Tree);
        CreatePatch(frame, Tree, DisplacementMap, HeightMapMeterSize, HeightMapWrapMode);
    }
    QuadTreeUpdateSubPass::~QuadTreeUpdateSubPass()
    { 
        delete[] NodeDescription;
    }
}
