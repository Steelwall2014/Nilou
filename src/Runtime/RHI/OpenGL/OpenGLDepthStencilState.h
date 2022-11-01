// #pragma once
// #include <glad/glad.h>
// #include <memory>

// #include "RHIResources.h"

// namespace nilou {

// 	class OpenGLDepthStencilState : public RHIDepthStencilState
// 	{
// 	public:
//         bool bZEnable;
//         bool bZWriteEnable;
//         GLenum ZFunc;
        

//         bool bStencilEnable;
//         bool bTwoSidedStencilMode;
//         GLenum StencilFunc;
//         GLenum StencilFail;
//         GLenum StencilZFail;
//         GLenum StencilPass;
//         GLenum BackStencilFunc;
//         GLenum BackStencilFail;
//         GLenum BackStencilZFail;
//         GLenum BackStencilPass;
//         uint32 StencilReadMask;
//         uint32 StencilWriteMask;
    
// 		inline OpenGLDepthStencilState()
//             : bZEnable(false)
//             , bZWriteEnable(true)
//             , ZFunc(GL_LESS)
//             , bStencilEnable(false)
//             , bTwoSidedStencilMode(false)
//             , StencilFunc(GL_ALWAYS)
//             , StencilFail(GL_KEEP)
//             , StencilZFail(GL_KEEP)
//             , StencilPass(GL_KEEP)
//             , BackStencilFunc(GL_ALWAYS)
//             , BackStencilFail(GL_KEEP)
//             , BackStencilZFail(GL_KEEP)
//             , BackStencilPass(GL_KEEP)
//             , StencilReadMask(0xFFFFFFFF)
//             , StencilWriteMask(0xFFFFFFFF)
//         {

//         }
        
// 		inline virtual bool Equals(RHIDepthStencilState *Other) 
//         { 
//             OpenGLDepthStencilState *GLOther = static_cast<OpenGLDepthStencilState *>(Other);
//             return bZEnable == GLOther->bZEnable && 
//                    bZWriteEnable == GLOther->bZWriteEnable &&
//                    ZFunc == GLOther->ZFunc &&
//                    bStencilEnable == GLOther->bStencilEnable &&
//                    bTwoSidedStencilMode == GLOther->bTwoSidedStencilMode &&
//                    StencilFunc == GLOther->StencilFunc &&
//                    StencilFail == GLOther->StencilFail &&
//                    StencilZFail == GLOther->StencilZFail &&
//                    StencilPass == GLOther->StencilPass &&
//                    BackStencilFunc == GLOther->BackStencilFunc &&
//                    BackStencilFail == GLOther->BackStencilFail &&
//                    BackStencilZFail == GLOther->BackStencilZFail &&
//                    BackStencilPass == GLOther->BackStencilPass &&
//                    StencilReadMask == GLOther->StencilReadMask &&
//                    StencilWriteMask == GLOther->StencilWriteMask;
//         }
// 	};
// 	using OpenGLDepthStencilStateRef = std::shared_ptr<OpenGLDepthStencilState>;
// }
