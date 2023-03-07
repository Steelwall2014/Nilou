#include "ShadowDepthPassRendering.h"
#include "Common/Log.h"
#include "Material.h"


constexpr float CASCADED_SHADOWMAP_SPLIT_FACTOR = 3.0f;

namespace nilou {

    IMPLEMENT_SHADER_TYPE(FShadowDepthVS, "/Shaders/MaterialShaders/ShadowDepthVertexShader.vert", EShaderFrequency::SF_Vertex, Material);
    IMPLEMENT_SHADER_TYPE(FShadowDepthPS, "/Shaders/GlobalShaders/DepthOnlyPixelShader.frag", EShaderFrequency::SF_Pixel, Global);

    void FShadowDepthVS::ModifyCompilationEnvironment(const FShaderPermutationParameters &Parameter, FShaderCompilerEnvironment &Environment)
    {
        FPermutationDomain Domain(Parameter.PermutationId);
        int FrustumCount = Domain.Get<FDimensionFrustumCount>();
        Environment.SetDefine("FrustumCount", FrustumCount);
    }

    static void BuildMeshDrawCommand(
        FDynamicRHI *RHICmdList,
        const FVertexFactoryPermutationParameters &VFPermutationParameters,
        FMaterialRenderProxy *Material,
        const FShaderPermutationParameters &PermutationParametersVS,
        const FShaderPermutationParameters &PermutationParametersPS,
        const FDepthStencilStateInitializer &DepthStencilStateInitializer,
        const FRasterizerStateInitializer &RasterizerStateInitializer,
        const FBlendStateInitializer &BlendStateInitializer,
        FInputShaderBindings &InputBindings,
        std::vector<FRHIVertexInput> &VertexInputs,
        const FMeshBatchElement &Element,
        FMeshDrawCommand &OutMeshDrawCommand
    )
    {
        
        FRHIGraphicsPipelineInitializer Initializer;

        FShaderInstance *VertexShader = Material->GetShader(VFPermutationParameters, PermutationParametersVS);
        Initializer.VertexShader = VertexShader;

        FShaderInstance *PixelShader = GetContentManager()->GetGlobalShader(PermutationParametersPS);
        Initializer.PixelShader = PixelShader;

        OutMeshDrawCommand.StencilRef = Material->StencilRefValue;
        OutMeshDrawCommand.DepthStencilState = RHICmdList->RHICreateDepthStencilState(DepthStencilStateInitializer);
        OutMeshDrawCommand.RasterizerState = RHICmdList->RHICreateRasterizerState(RasterizerStateInitializer);
        OutMeshDrawCommand.BlendState = RHICmdList->RHICreateBlendState(BlendStateInitializer);

        {
            OutMeshDrawCommand.PipelineState = RHICmdList->RHIGetOrCreatePipelineStateObject(Initializer);
            OutMeshDrawCommand.IndexBuffer = Element.IndexBuffer->IndexBufferRHI.get();

            Material->FillShaderBindings(InputBindings);
       
            auto &StageUniformBufferBindings = OutMeshDrawCommand.ShaderBindings.UniformBufferBindings[PS_Vertex]; // alias
            auto &StageSamplerBindings = OutMeshDrawCommand.ShaderBindings.SamplerBindings[PS_Vertex]; // alias
            FRHIDescriptorSet &DescriptorSets = OutMeshDrawCommand.PipelineState->PipelineLayout.DescriptorSets[PS_Vertex];
            
            for (auto [Name,Binding] : DescriptorSets.Bindings)
            {
                bool bResourceFound = OutMeshDrawCommand.ShaderBindings.SetShaderBinding(
                    static_cast<EPipelineStage>(PS_Vertex), Binding, InputBindings);

                if (!bResourceFound)
                {
                        NILOU_LOG(Warning, 
                            "Material: {}"
                            " |Vertex Factory: {}"
                            " |Vertex Shader: {}"
                            " |Pixel Shader: {}"
                            " |Pipeline Stage: {}"
                            " |\"{}\" Resource not provided",
                            Material->Name,
                            VFPermutationParameters.Type->Name,
                            PermutationParametersVS.Type->Name,
                            PermutationParametersVS.Type->Name,
                            magic_enum::enum_name(PS_Vertex),
                            Binding.Name);
                }

            }

            OutMeshDrawCommand.ShaderBindings.VertexAttributeBindings = VertexInputs;
            if (Element.NumVertices == 0)
            {
                OutMeshDrawCommand.IndirectArgs.Buffer = Element.IndirectArgsBuffer;
                OutMeshDrawCommand.IndirectArgs.Offset = Element.IndirectArgsOffset;
                OutMeshDrawCommand.UseIndirect = true;
            }
            else
            {
                OutMeshDrawCommand.DirectArgs.BaseVertexIndex = Element.FirstIndex;
                OutMeshDrawCommand.DirectArgs.NumInstances = Element.NumInstances;
                OutMeshDrawCommand.DirectArgs.NumVertices = Element.NumVertices;
                OutMeshDrawCommand.UseIndirect = false;
            }
        }
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

    void FDefferedShadingSceneRenderer::RenderCSMShadowPass(FDynamicRHI *RHICmdList)
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

        for (int LightIndex = 0; LightIndex < Lights.size(); LightIndex++)
        {
            if (Lights[LightIndex].LightSceneInfo->SceneProxy->LightType != ELightType::LT_Directional)
                continue;

            auto &Light = Lights[LightIndex];
            FLightSceneInfo *LightInfo = Lights[LightIndex].LightSceneInfo;

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
            

            for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
            {
                FViewSceneInfo *ViewInfo = Views[ViewIndex].ViewSceneInfo;
                FSceneView SceneView = ViewInfo->SceneProxy->GetSceneView();
                std::vector<std::array<dvec3, 8>> CascadeFrustums;
                
                const double t = glm::tan(0.5 * SceneView.VerticalFieldOfView);
                const double b = -t;
                const double r = t * SceneView.AspectRatio;
                const double l = -r;
                const glm::dvec3 Right = glm::cross(SceneView.Forward, SceneView.Up);
                const double FrustumLength = SceneView.FarClipDistance - SceneView.NearClipDistance;
                double SplitNear = 0;
                double SplitFar = SceneView.NearClipDistance;
                for (int SplitIndex : FrustumsToBeUpdated)
                {
                    std::array<dvec3, 8> CascadeFrustumVerts;
                    SplitNear = SplitFar;
                    SplitFar = Scales[SplitIndex] * FrustumLength + SplitNear;
                    glm::dvec3 nearCenter = SceneView.Forward * SplitNear + SceneView.Position;
                    glm::dvec3 farCenter = SceneView.Forward * SplitFar + SceneView.Position;
                    glm::dvec3 TopNear = SceneView.Up * t * SplitNear;
                    glm::dvec3 TopFar = SceneView.Up * t * SplitFar;
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
                    dvec3 Center = SceneView.Forward * CenterZ + SceneView.Position;
                    double Radius = glm::max(glm::distance(Center, CascadeFrustumVerts[0]), glm::distance(Center, CascadeFrustumVerts[4]));
                    FBoundingSphere Sphere(Center, Radius);
                    FConvexVolume ConvexVolume;
                    FPlane NearPlane, FarPlane;
                    ComputeShadowCullingVolume(
                        CascadeFrustumVerts, 
                        LightInfo->SceneProxy->Direction, 
                        ConvexVolume, 
                        NearPlane, 
                        FarPlane);
                    dvec3 Direction = glm::dvec3(LightInfo->SceneProxy->Direction);
                    dmat4 ViewMatrix = glm::lookAt(
                        Sphere.Center, 
                        Direction+Sphere.Center, 
                        glm::dvec3(LightInfo->SceneProxy->Up));

                    // The collector used for shadow mapping
                    FMeshElementCollector LightCollector;
                    std::vector<FMeshBatch> &MeshBatches = Light.ShadowMapMeshBatches[ViewIndex].MeshBatches[SplitIndex];
                    MeshBatches.clear();
                    FParallelMeshDrawCommands &DrawCommands = Light.ShadowMapMeshDrawCommands[ViewIndex].DrawCommands[SplitIndex];
                    DrawCommands.Clear();
                    LightCollector.PerViewMeshBatches.push_back(&MeshBatches);
                    double near, far;
                    far = DBL_MAX;
                    near = -DBL_MAX;
                    for (auto &&PrimitiveInfo : Scene->AddedPrimitiveSceneInfos)
                    {
                        if (!PrimitiveInfo->SceneProxy->bCastShadow)
                            continue;
                        const FBoundingBox Box = PrimitiveInfo->SceneProxy->GetBounds();
                        const FBoundingBox ViewSpaceBox = Box.TransformBy(FTransform(ViewMatrix));
                        bool bCulled = ViewSpaceBox.Max.x < -Radius || 
                                       ViewSpaceBox.Min.x > Radius ||
                                       ViewSpaceBox.Max.y < -Radius ||
                                       ViewSpaceBox.Min.x > Radius;
                        if (!bCulled)
                        {
                            near = glm::max(near, ViewSpaceBox.Max.z);
                            far = glm::min(far, ViewSpaceBox.Min.z);
                            PrimitiveInfo->SceneProxy->GetDynamicMeshElements({SceneView}, 0x1, LightCollector);
                        }
                    }
                    // if (near == -DBL_MAX) near = 0;
                    // if (far == DBL_MAX) far = 0;
                    near = Radius;
                    far = -Radius;

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

                    dvec3 Position = Center - Direction * (near);
                    ViewMatrix = glm::lookAt(
                        Position, 
                        Center, 
                        glm::dvec3(LightInfo->SceneProxy->Up));
                        
                    dmat4 ProjectionMatrix = glm::ortho(
                        -Sphere.Radius, 
                        Sphere.Radius,
                        -Sphere.Radius,
                        Sphere.Radius,
                        0.0, glm::abs(far-near));

                    auto UniformBuffer = FShadowMapUniformBuffers::Cast<CASCADED_SHADOWMAP_SPLIT_COUNT>(Light.ShadowMapUniformBuffers[ViewIndex]);
                    UniformBuffer->Data.Frustums[SplitIndex].WorldToClip = ProjectionMatrix * ViewMatrix;
                    UniformBuffer->Data.Frustums[SplitIndex].FrustumFar = SplitFar;
                    UniformBuffer->Data.Frustums[SplitIndex].Resolution = Light.LightSceneInfo->SceneProxy->ShadowMapResolution;
                    UniformBuffer->UpdateUniformBuffer();
                
                    for (FMeshBatch &Mesh : MeshBatches)
                    {
                        if (!Mesh.CastShadow)   
                            continue;
                        FVertexFactoryPermutationParameters VertexFactoryParams(Mesh.Element.VertexFactory->GetType(), Mesh.Element.VertexFactory->GetPermutationId());
                        FShadowDepthVS::FPermutationDomain PermutationVector;
                        PermutationVector.Set<FShadowDepthVS::FDimensionFrustumCount>(CASCADED_SHADOWMAP_SPLIT_COUNT);
                        FShaderPermutationParameters PermutationParametersVS(&FShadowDepthVS::StaticType, PermutationVector.ToDimensionValueId());
                        FShaderPermutationParameters PermutationParametersPS(&FShadowDepthPS::StaticType, 0);
                        FMeshDrawCommand MeshDrawCommand;
                        std::vector<FRHIVertexInput> VertexInputs;
                        Mesh.Element.VertexFactory->GetVertexInputList(VertexInputs);
                        FInputShaderBindings InputBindings = Mesh.Element.Bindings;
                        InputBindings.SetElementShaderBinding("FShadowMappingBlock", 
                            UniformBuffer);
                        InputBindings.SetElementShaderBinding("FShadowMapFrustumIndex", 
                            SplitIndexUBO.UBOs[SplitIndex].get());
                        InputBindings.SetElementShaderBinding("FViewShaderParameters", 
                            ViewInfo->SceneProxy->GetViewUniformBuffer());
                        BuildMeshDrawCommand(
                            RHICmdList,
                            VertexFactoryParams,
                            Mesh.MaterialRenderProxy.get(),
                            PermutationParametersVS,
                            PermutationParametersPS,
                            Mesh.MaterialRenderProxy->DepthStencilState,
                            Mesh.MaterialRenderProxy->RasterizerState,
                            Mesh.MaterialRenderProxy->BlendState,
                            InputBindings,
                            VertexInputs,
                            Mesh.Element,
                            MeshDrawCommand);
                        DrawCommands.AddMeshDrawCommand(MeshDrawCommand);
                    }
                }
            }
        }

        for (int LightIndex = 0; LightIndex < Lights.size(); LightIndex++)
        {
            if (Lights[LightIndex].LightSceneInfo->SceneProxy->LightType != ELightType::LT_Directional)
                continue;

            auto &Light = Lights[LightIndex];
            for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
            {
                auto &ShadowMapTexture = Light.ShadowMapTextures[ViewIndex];
                auto &ShadowMapUniformBuffer = Light.ShadowMapUniformBuffers[ViewIndex];
                auto &ShadowMapMeshBatch = Light.ShadowMapMeshBatches[ViewIndex];
                auto &ShadowMapMeshDrawCommand = Light.ShadowMapMeshDrawCommands[ViewIndex];
                for (int FrustumIndex : FrustumsToBeUpdated)
                {
                    FRHIRenderPassInfo PassInfo(
                        ShadowMapTexture.FrameBuffers[FrustumIndex].get(), 
                        ShadowMapTexture.DepthArray->GetSizeXYZ(), true, true, true);
                    RHICmdList->RHIBeginRenderPass(PassInfo);
                    ShadowMapMeshDrawCommand.DrawCommands[FrustumIndex].DispatchDraw(RHICmdList);
                    RHICmdList->RHIEndRenderPass();
                }
            }
        }
    }

}