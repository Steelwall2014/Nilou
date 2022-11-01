#pragma once
// #include "OpenGL/OpenGLDynamicRHI.h"
#include "Common/GfxStructures.h"
#include "RHIResources.h"
namespace und {
    class WaterbodyParameters;
    DrawBatchContext CreateDBCFromMesh(const und::SceneObjectMesh &mesh);
    void SetPerFrameShaderParameters(const DrawFrameContext &context);
    void SetAtmosphereParameters(std::shared_ptr<SceneObjectAtmosphere> atmosphere);
    void SetWaterbodyParameters(std::shared_ptr<SceneObjectWaterbody> waterbody);
    RHITexture2DRef UploadTexture(std::shared_ptr<SceneObjectTexture> tex, bool EnableMipmap=false);
    //void CalculateLightParams(FrameVariables &frame);

}