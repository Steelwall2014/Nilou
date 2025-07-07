#include "ShadowDepthPassRendering.h"
#include "Common/Log.h"
#include "Material.h"


constexpr float CASCADED_SHADOWMAP_SPLIT_FACTOR = 2.0f;

constexpr double CASCADED_SHADOWMAP_MAX_DISTANCE = 800.0;

constexpr double SHADOWMAP_FAR_CLIP = 10000.0;

constexpr double SHADOWMAP_NEAR_CLIP = 0.1;

namespace nilou {

    IMPLEMENT_SHADER_TYPE(FShadowDepthVS, "/Shaders/MaterialShaders/ShadowDepthVertexShader.vert", EShaderFrequency::SF_Vertex, Material);
    IMPLEMENT_SHADER_TYPE(FShadowDepthPS, "/Shaders/MaterialShaders/DepthOnlyPixelShader.frag", EShaderFrequency::SF_Pixel, Material);

    void FShadowDepthVS::ModifyCompilationEnvironment(const FShaderPermutationParameters &Parameter, FShaderCompilerEnvironment &Environment)
    {
        FPermutationDomain Domain(Parameter.PermutationId);
        Domain.ModifyCompilationEnvironment(Environment);
    }

    void ComputeShadowCullingVolume(std::array<dvec3, 8> CascadeFrustumVerts, const vec3& LightDirection, FConvexVolume& ConvexVolumeOut, FPlane& NearPlaneOut, FPlane& FarPlaneOut) 
    {

        // Pairs of plane indices from SubFrustumPlanes whose intersections
        // form the edges of the frustum.
        static const int32 AdjacentPlanePairs[12][2] =
        {
            {0,2}, {0,4}, {0,1}, {0,3},
            {2,3}, {4,2}, {1,4}, {3,1},
            {2,5}, {4,5}, {1,5}, {3,5}
        };
        // Maps a plane pair index to the index of the two frustum corners
        // which form the end points of the plane intersection.
        static const int32 LineVertexIndices[12][2] =
        {
            {0,1}, {1,3}, {3,2}, {2,0},
            {0,4}, {1,5}, {3,7}, {2,6},
            {4,5}, {5,7}, {7,6}, {6,4}
        };

        std::vector<FPlane> Planes;

        // Find the view frustum subsection planes which face away from the light and add them to the bounding volume
        FPlane SubFrustumPlanes[6];
        SubFrustumPlanes[0] = FPlane(CascadeFrustumVerts[3], CascadeFrustumVerts[2], CascadeFrustumVerts[0]); // Near
        SubFrustumPlanes[1] = FPlane(CascadeFrustumVerts[7], CascadeFrustumVerts[6], CascadeFrustumVerts[2]); // Left
        SubFrustumPlanes[2] = FPlane(CascadeFrustumVerts[0], CascadeFrustumVerts[4], CascadeFrustumVerts[5]); // Right
        SubFrustumPlanes[3] = FPlane(CascadeFrustumVerts[2], CascadeFrustumVerts[6], CascadeFrustumVerts[4]); // Top
        SubFrustumPlanes[4] = FPlane(CascadeFrustumVerts[5], CascadeFrustumVerts[7], CascadeFrustumVerts[3]); // Bottom
        SubFrustumPlanes[5] = FPlane(CascadeFrustumVerts[4], CascadeFrustumVerts[6], CascadeFrustumVerts[7]); // Far
        

        NearPlaneOut = SubFrustumPlanes[0];
        FarPlaneOut = SubFrustumPlanes[5];

        // Add the planes from the camera's frustum which form the back face of the frustum when in light space.
        for (int32 i = 0; i < 6; i++)
        {
            vec3 Normal = SubFrustumPlanes[i].Normal;
            float d = glm::dot(Normal, LightDirection);
            if (d < 0.0f)
            {
                Planes.push_back(SubFrustumPlanes[i]);
            }
        }

        // Now add the planes which form the silhouette edges of the camera frustum in light space.
        for (int32 i = 0; i < 12; i++)
        {
            vec3 NormalA = SubFrustumPlanes[AdjacentPlanePairs[i][0]].Normal;
            vec3 NormalB = SubFrustumPlanes[AdjacentPlanePairs[i][1]].Normal;

            float DotA = glm::dot(NormalA, LightDirection);
            float DotB = glm::dot(NormalB, LightDirection);

            // If the signs of the dot product are different
            if (DotA * DotB < 0.0f)
            {
                // Planes are opposing, so this is an edge. 
                // Extrude the plane along the light direction, and add it to the array.

                vec3 A = CascadeFrustumVerts[LineVertexIndices[i][0]];
                vec3 B = CascadeFrustumVerts[LineVertexIndices[i][1]];
                // Scale the 3rd vector by the length of AB for precision
                vec3 C = A + LightDirection * glm::length(A - B);

                // Account for winding
                if (DotA >= 0.0f)
                {
                    Planes.push_back(FPlane(A, B, C));
                }
                else
                {
                    Planes.push_back(FPlane(B, A, C));
                }
            }
        }
        ConvexVolumeOut = FConvexVolume(Planes);
    }

    void FDeferredShadingSceneRenderer::RenderCSMShadowPass(RenderGraph& Graph)
    {
        std::vector<int> FrustumsToBeUpdated = {0, 1, 2, 3};
        // The last 4 cascade levels are not updated every frame;
        static int FrustumRoundRobinIndex = -1;
        if (FrustumRoundRobinIndex == -1)   // first frame, update all
        {
            FrustumsToBeUpdated.push_back(4);
            FrustumsToBeUpdated.push_back(5);
            FrustumsToBeUpdated.push_back(6);
            FrustumsToBeUpdated.push_back(7);
            FrustumRoundRobinIndex++;
        }
        else 
        {
            FrustumRoundRobinIndex = FrustumRoundRobinIndex % 4;
            FrustumsToBeUpdated.push_back(FrustumRoundRobinIndex + 4);
            FrustumRoundRobinIndex++;
        }

        float TotalScale = 0;
        float CurrentScale = 1;
        std::vector<float> Scales;
        for (int i = 0; i < CASCADED_SHADOWMAP_SPLIT_COUNT; i++)
        {
            Scales.push_back(CurrentScale);
            TotalScale += CurrentScale;
            CurrentScale *= CASCADED_SHADOWMAP_SPLIT_FACTOR;
        }
        for (int i = 0; i < Scales.size(); i++)
            Scales[i] /= TotalScale;

        for (int LightIndex = 0; LightIndex < Lights.size(); LightIndex++)
        {
            FLightSceneProxy* LightSceneProxy = Lights[LightIndex].LightSceneProxy;
            // Temporarily only support directional light
            if (LightSceneProxy->LightType != ELightType::LT_Directional)
                continue;
            auto &Light = Lights[LightIndex];
            

            for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
            {
                FShadowMapResource& Resources = Light.ShadowMapResources[ViewIndex];
                FSceneView& View = Views[ViewIndex];
                std::vector<std::array<dvec3, 8>> CascadeFrustums;

                FViewport ShadowViewport;
                ShadowViewport.Width = LightSceneProxy->ShadowMapResolution.x;
                ShadowViewport.Height = LightSceneProxy->ShadowMapResolution.y;
                FSceneViewFamily ShadowViewFamily(ShadowViewport, Scene);

                RDGBuffer* ShadowMappingBlock = Graph.CreateUniformBuffer<FDirectionalShadowMappingBlock>(NFormat("Light {} DirectionalShadowMappingBlock", LightIndex));

                std::vector<std::vector<FMeshBatch>> ShadowMeshBatches;
                std::vector<FViewElementPDI> ShadowPDIs;

                const double ShadowFarClip = glm::clamp(View.FarClipDistance, View.NearClipDistance, CASCADED_SHADOWMAP_MAX_DISTANCE);
                const double ShadowNearClip = glm::clamp(View.NearClipDistance, 0.0, ShadowFarClip);
                const double t = glm::tan(0.5 * View.VerticalFieldOfView);
                const double b = -t;
                const double r = t * View.AspectRatio;
                const double l = -r;
                const glm::dvec3 Right = glm::cross(View.Forward, View.Up);
                const double FrustumLength = ShadowFarClip - ShadowNearClip;
                double SplitNear = 0;
                double SplitFar = ShadowNearClip;
                for (int FrustumIndex : FrustumsToBeUpdated)
                {
                    std::array<dvec3, 8> CascadeFrustumVerts;
                    SplitNear = SplitFar;
                    SplitFar = Scales[FrustumIndex] * FrustumLength + SplitNear;
                    glm::dvec3 nearCenter = View.Forward * SplitNear + View.Position;
                    glm::dvec3 farCenter = View.Forward * SplitFar + View.Position;
                    glm::dvec3 TopNear = View.Up * t * SplitNear;
                    glm::dvec3 TopFar = View.Up * t * SplitFar;
                    glm::dvec3 RightNear = Right * r * SplitNear;
                    glm::dvec3 RightFar = Right * r * SplitFar;
                    CascadeFrustumVerts[0] = nearCenter + TopNear + RightNear;
                    CascadeFrustumVerts[1] = nearCenter - TopNear + RightNear;
                    CascadeFrustumVerts[2] = nearCenter + TopNear - RightNear;
                    CascadeFrustumVerts[3] = nearCenter - TopNear - RightNear;
                    CascadeFrustumVerts[4] = farCenter + TopFar + RightFar;
                    CascadeFrustumVerts[5] = farCenter - TopFar + RightFar;
                    CascadeFrustumVerts[6] = farCenter + TopFar - RightFar;
                    CascadeFrustumVerts[7] = farCenter - TopFar - RightFar;
                    CascadeFrustums.push_back(CascadeFrustumVerts);
                    const double DiagonalSq_Near = glm::dot(TopNear+RightNear, TopNear+RightNear);
                    const double DiagonalSq_Far = glm::dot(TopFar+RightFar, TopFar+RightFar);
                    const double SplitFrustumLength = SplitFar-SplitNear;
                    const double OptimalOffset = (DiagonalSq_Far - DiagonalSq_Near + SplitFrustumLength*SplitFrustumLength) / (2.0 * SplitFrustumLength);
                    const double CenterZ = glm::clamp(SplitNear+OptimalOffset, SplitNear, SplitFar);
                    dvec3 Center = View.Forward * CenterZ + View.Position;
                    double Radius = glm::max(glm::distance(Center, CascadeFrustumVerts[0]), glm::distance(Center, CascadeFrustumVerts[4]));
                    FBoundingSphere Sphere(Center, Radius);
                    FConvexVolume ConvexVolume;
                    FPlane NearPlane, FarPlane;
                    ComputeShadowCullingVolume(
                        CascadeFrustumVerts, 
                        LightSceneProxy->Direction, 
                        ConvexVolume, 
                        NearPlane, 
                        FarPlane);
                    dvec3 Direction = glm::dvec3(LightSceneProxy->Direction);
                    dmat4 ViewMatrix = glm::lookAt(
                        Sphere.Center, 
                        Direction+Sphere.Center, 
                        glm::dvec3(LightSceneProxy->Up));
                        
                    FSceneView LightSceneView;
                    LightSceneView.VerticalFieldOfView = LightSceneProxy->VerticalFieldOfView;
                    LightSceneView.NearClipDistance = 0;
                    LightSceneView.FarClipDistance = 2*Radius;
                    LightSceneView.Forward = Direction;
                    LightSceneView.Position = Sphere.Center;
                    LightSceneView.Up = LightSceneProxy->Up;
                    LightSceneView.ViewMatrix = ViewMatrix;
                    LightSceneView.ProjectionMatrix = glm::ortho(
                                                        -Sphere.Radius, 
                                                        Sphere.Radius,
                                                        -Sphere.Radius,
                                                        Sphere.Radius,
                                                        0.0, 2*Radius);
                    LightSceneView.ViewFrustum = FViewFrustum(ViewMatrix, LightSceneView.ProjectionMatrix);
                    LightSceneView.ScreenResolution = ivec2(LightSceneProxy->ShadowMapResolution.x, LightSceneProxy->ShadowMapResolution.y);
                    ShadowViewFamily.Views.push_back(LightSceneView);

                    // {   // To remove jittering
                    //     float cascadeAABBSize = Sphere.Radius * 2.0f;
                    //     dvec2 worldUnitsPerPixel = dvec2(cascadeAABBSize) / dvec2(Light.LightSceneInfo->SceneProxy->ShadowMapResolution);
                    //     dmat4 view_matrix = glm::lookAt(
                    //         dvec3(0), 
                    //         Direction, 
                    //         glm::dvec3(LightInfo->SceneProxy->Up));	
                    //     dvec4 LightSpaceCenter = view_matrix * dvec4(Center, 1);
                    //     double offsetx = glm::modf(LightSpaceCenter.x, worldUnitsPerPixel.x);
                    //     double offsety = glm::modf(LightSpaceCenter.y, worldUnitsPerPixel.y);
                    //     LightSpaceCenter -= vec4(offsetx, offsety, 0, 0);
                    //     Center = glm::inverse(view_matrix) * LightSpaceCenter;
                    // }

                    dvec3 Position = Center - Direction * SHADOWMAP_FAR_CLIP;
                    ViewMatrix = glm::lookAt(
                        Position, 
                        Center, 
                        glm::dvec3(LightSceneProxy->Up));
                        
                    dmat4 ProjectionMatrix = glm::ortho(
                        -Sphere.Radius, 
                        Sphere.Radius,
                        -Sphere.Radius,
                        Sphere.Radius,
                        0.0, glm::abs(SHADOWMAP_FAR_CLIP-SHADOWMAP_NEAR_CLIP));

                    Resources.Frustums[FrustumIndex].WorldToClip = ProjectionMatrix * ViewMatrix;
                    Resources.Frustums[FrustumIndex].FrustumFar = SplitFar;
                    Resources.Frustums[FrustumIndex].Resolution = Light.LightSceneProxy->ShadowMapResolution;
                }
                Graph.QueueBufferUpload(ShadowMappingBlock, Resources.Frustums.data(), Resources.Frustums.size() * sizeof(FShadowMappingParameters));

                ComputeViewVisibility(ShadowViewFamily, ShadowMeshBatches, ShadowPDIs);

                for (int ShadowViewIndex = 0; ShadowViewIndex < ShadowMeshBatches.size(); ShadowViewIndex++)
                {
                    FParallelMeshDrawCommands DrawCommands;
                    RDGRenderTargets RenderTargets;
                    RenderTargets.DepthStencilAttachment = Resources.DepthViews[ShadowViewIndex];
                    int FrustumIndex = FrustumsToBeUpdated[ShadowViewIndex];
                    for (FMeshBatch& Mesh : ShadowMeshBatches[ShadowViewIndex])
                    {
                        if (!Mesh.CastShadow)   
                            continue;
                        for (FMeshBatchElement& Element : Mesh.Elements)
                        {
                            FVertexFactoryPermutationParameters VertexFactoryParams(Element.VertexFactory->GetType(), Element.VertexFactory->GetPermutationId());
                            FShadowDepthVS::FPermutationDomain PermutationVector;
                            PermutationVector.Set<FShadowDepthVS::FDimensionFrustumCount>(CASCADED_SHADOWMAP_SPLIT_COUNT);
                            FShaderPermutationParameters PermutationParametersVS(&FShadowDepthVS::StaticType, PermutationVector.ToDimensionValueId());
                            FShaderPermutationParameters PermutationParametersPS(&FShadowDepthPS::StaticType, 0);

                            FMeshDrawShaderBindings ShaderBindings = Mesh.MaterialRenderProxy->GetShaderBindings();
                            ShaderBindings.SetBuffer("FViewShaderParameters", View.ViewUniformBuffer);
                            ShaderBindings.SetBuffer("FShadowMappingBlock", ShadowMappingBlock);
                            ShaderBindings.SetBuffer("FPrimitiveShaderParameters", Element.PrimitiveUniformBuffer);
                            ShaderBindings.SetPushConstant(EShaderStage::Vertex, sizeof(int), &FrustumIndex);

                            FMeshDrawCommand MeshDrawCommand;
                            BuildMeshDrawCommand(
                                Graph,
                                VertexFactoryParams,
                                Mesh.MaterialRenderProxy,
                                PermutationParametersVS,
                                PermutationParametersPS,
                                Element.VertexFactory->GetVertexDeclaration(),
                                Element,
                                RenderTargets.GetRenderTargetLayout(),
                                ShaderBindings,
                                MeshDrawCommand);

                            DrawCommands.AddMeshDrawCommand(MeshDrawCommand);
                        }
                    }
                    RDGPassDesc PassDesc{NFormat("ShadowDepthPass of view {} of ShadowView {}", ViewIndex, ShadowViewIndex)};
                    PassDesc.bNeverCull = true;
                    Graph.AddGraphicsPass(
                        PassDesc,
                        RenderTargets,
                        DrawCommands.GetIndexBuffers(),
                        DrawCommands.GetVertexBuffers(),
                        DrawCommands.GetDescriptorSets(),
                        [=](RHICommandList& RHICmdList)
                        {
                            DrawCommands.DispatchDraw(RHICmdList);
                        }
                    );
                }

            }
        }        
    }

}