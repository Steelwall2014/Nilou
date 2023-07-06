#pragma once
#include "PrimitiveComponent.h"
#include "Material.h"

namespace nilou {
    
    BEGIN_UNIFORM_BUFFER_STRUCT(FOceanFastFourierTransformParameters)
        SHADER_PARAMETER(vec2, WindDirection)
        SHADER_PARAMETER(uint32, N)
        SHADER_PARAMETER(float, WindSpeed)
        SHADER_PARAMETER(float, Amplitude)
        SHADER_PARAMETER(float, DisplacementTextureSize)
        SHADER_PARAMETER(float, Time)
    END_UNIFORM_BUFFER_STRUCT()

    BEGIN_UNIFORM_BUFFER_STRUCT(FOceanFFTButterflyBlock)
        SHADER_PARAMETER(uint32, Ns)
    END_UNIFORM_BUFFER_STRUCT()

    class NCLASS UFourierTransformOceanComponent : public UPrimitiveComponent
    {
        GENERATED_BODY()
        friend class FFourierTransformOceanSceneProxy;
    public:

        UFourierTransformOceanComponent();

        virtual void TickComponent(double DeltaTime) override;

        virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

        virtual FBoundingBox CalcBounds(const FTransform& LocalToWorld) const override;

    protected:

        std::shared_ptr<UMaterialInstance> Material = nullptr;

        // Wind direction, must be normalized
		vec2 WindDirection = glm::normalize(vec2(1));

        float WindSpeed = 6.5f;

        uint32 FFTPow = 9;

        float Amplitude = 0.45f * 1e-3f;

        // Corresponding to UE5 WorldAlignedTexture TextureSize pin
        float DisplacementTextureSize = 0.05 * glm::pow(2, FFTPow);

        // 单个分段中四边形的数量（边长），一个分段即为海面渲染LOD过渡的单位
        // 默认缩放下一个四边形的大小为1*1
        uint32 NumQuadsPerNode = 64;

        // X和Y方向的节点数量，这将作为最精细一级LOD的节点数
        uint32 NodeCount = 160;

        uint32 LodCount = 1;

    private:
        BS::thread_pool pool;

        std::vector<uvec4> CreateNodeList(const FViewFrustum &Frustum, const dvec3& CameraPosition);

        struct OceanLODParam
        {
            vec2 NodeMeterSize;
            uvec2 NodeSideNum;
        };
        std::vector<OceanLODParam> LODParams;
        dvec3 GetNodePositionWS(uvec2 nodeLoc, OceanLODParam &LODParam)
        {
            vec2 xy_center_coord = (vec2(nodeLoc) + vec2(0.5)) * vec2(LODParam.NodeMeterSize.x, LODParam.NodeMeterSize.y);
            vec2 uv = (vec2(nodeLoc) + vec2(0.5)) / vec2(LODParam.NodeSideNum.x, LODParam.NodeSideNum.y);
            float height = 0.5;
            return GetComponentTransform().TransformPosition(dvec3(xy_center_coord, height));
        }
        bool MeetScreenSize(const dvec3& CameraPosition, uvec2 nodeLoc, OceanLODParam &LODParam)
        {
            dvec3 positionWS = GetNodePositionWS(nodeLoc, LODParam);
            double dis = glm::distance(CameraPosition, positionWS);
            dvec2 scale_xy = GetComponentTransform().GetScale3D();
            double nodeSize = glm::max(LODParam.NodeMeterSize.x*scale_xy.x, LODParam.NodeMeterSize.y*scale_xy.y);
            double f = (nodeSize * 2.5) / dis;
            return f <= 1;
        }

    };

}