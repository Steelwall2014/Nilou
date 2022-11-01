// #pragma once
// #include <glad/glad.h>
// #include <memory>

// #include "RHI.h"
// #include "RHIResources.h"

// namespace nilou {
//     class OpenGLRasterizerState : public RHIRasterizerState
//     {
//     public:
//         GLenum FillMode;
//         GLenum CullMode;
//         float DepthBias;
//         float SlopeScaleDepthBias;

//         OpenGLRasterizerState()
//             : FillMode(GL_FILL)
//             , CullMode(GL_NONE)
//             , DepthBias(0.0f)
//             , SlopeScaleDepthBias(0.0f)
//         {
//         }
//     };
//     using OpenGLRasterizerStateRef = std::shared_ptr<OpenGLRasterizerState>;
// }