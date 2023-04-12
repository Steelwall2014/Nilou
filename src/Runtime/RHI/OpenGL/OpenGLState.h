#pragma once
#include <glad/glad.h>
#include <memory>
#include <array>

#include "RHIResources.h"

namespace nilou {

	class OpenGLDepthStencilState : public RHIDepthStencilState
	{
	public:
        bool bZEnable;
        bool bZWriteEnable;
        GLenum ZFunc;
        

        bool bStencilEnable;
        bool bTwoSidedStencilMode;
        GLenum StencilFunc;
        GLenum StencilFail;
        GLenum StencilZFail;
        GLenum StencilPass;
        GLenum BackStencilFunc;
        GLenum BackStencilFail;
        GLenum BackStencilZFail;
        GLenum BackStencilPass;
        uint32 StencilReadMask;
        uint32 StencilWriteMask;
    
		inline OpenGLDepthStencilState()
            : bZEnable(false)
            , bZWriteEnable(true)
            , ZFunc(GL_LESS)
            , bStencilEnable(false)
            , bTwoSidedStencilMode(false)
            , StencilFunc(GL_ALWAYS)
            , StencilFail(GL_KEEP)
            , StencilZFail(GL_KEEP)
            , StencilPass(GL_KEEP)
            , BackStencilFunc(GL_ALWAYS)
            , BackStencilFail(GL_KEEP)
            , BackStencilZFail(GL_KEEP)
            , BackStencilPass(GL_KEEP)
            , StencilReadMask(0xFFFFFFFF)
            , StencilWriteMask(0xFFFFFFFF)
        {

        }
        
		// inline virtual bool Equals(RHIDepthStencilState *Other) 
        // { 
        //     OpenGLDepthStencilState *GLOther = static_cast<OpenGLDepthStencilState *>(Other);
        //     return bZEnable == GLOther->bZEnable && 
        //            bZWriteEnable == GLOther->bZWriteEnable &&
        //            ZFunc == GLOther->ZFunc &&
        //            bStencilEnable == GLOther->bStencilEnable &&
        //            bTwoSidedStencilMode == GLOther->bTwoSidedStencilMode &&
        //            StencilFunc == GLOther->StencilFunc &&
        //            StencilFail == GLOther->StencilFail &&
        //            StencilZFail == GLOther->StencilZFail &&
        //            StencilPass == GLOther->StencilPass &&
        //            BackStencilFunc == GLOther->BackStencilFunc &&
        //            BackStencilFail == GLOther->BackStencilFail &&
        //            BackStencilZFail == GLOther->BackStencilZFail &&
        //            BackStencilPass == GLOther->BackStencilPass &&
        //            StencilReadMask == GLOther->StencilReadMask &&
        //            StencilWriteMask == GLOther->StencilWriteMask;
        // }
	};
	using OpenGLDepthStencilStateRef = std::shared_ptr<OpenGLDepthStencilState>;

    class OpenGLRasterizerState : public RHIRasterizerState
    {
    public:
        GLenum FillMode;
        GLenum CullMode;
        float DepthBias;
        float SlopeScaleDepthBias;

        OpenGLRasterizerState()
            : FillMode(GL_FILL)
            , CullMode(GL_NONE)
            , DepthBias(0.0f)
            , SlopeScaleDepthBias(0.0f)
        {
        }
        
		// inline virtual bool Equals(OpenGLRasterizerState *Other) 
        // { 
        //     OpenGLRasterizerState *GLOther = static_cast<OpenGLRasterizerState *>(Other);
        //     return FillMode == GLOther->FillMode && 
        //            CullMode == GLOther->CullMode &&
        //            DepthBias == GLOther->DepthBias &&
        //            SlopeScaleDepthBias == GLOther->SlopeScaleDepthBias;
        // }
    };
    using OpenGLRasterizerStateRef = std::shared_ptr<OpenGLRasterizerState>;

    class OpenGLBlendState : public RHIBlendState
    {
    public:

        struct FRenderTarget
        {
            bool bAlphaBlendEnable = false;
            GLenum ColorBlendOperation = GL_ADD;
            GLenum ColorSourceBlendFactor = GL_ONE;
            GLenum ColorDestBlendFactor = GL_ZERO;
            bool bSeparateAlphaBlendEnable = false;
            GLenum AlphaBlendOperation = GL_ADD;
            GLenum AlphaSourceBlendFactor = GL_ONE;
            GLenum AlphaDestBlendFactor = GL_ZERO;
            uint32 ColorWriteMaskR : 1 = 1;
            uint32 ColorWriteMaskG : 1 = 1;
            uint32 ColorWriteMaskB : 1 = 1;
            uint32 ColorWriteMaskA : 1 = 1;
        };
        
        std::array<FRenderTarget, MAX_SIMULTANEOUS_RENDERTARGETS> RenderTargets;

        
		// inline virtual bool Equals(OpenGLBlendState *Other) 
        // { 
        //     OpenGLBlendState *GLOther = static_cast<OpenGLBlendState *>(Other);
        //     return RenderTargets == GLOther->RenderTargets;
        // }
        // bool bUseAlphaToCoverage;
    };
    using OpenGLBlendStateRef = std::shared_ptr<OpenGLBlendState>;
}
