#include <iostream>
#include <glad/glad.h>
#include <memory>
#include <numeric>
#include <tinygltf/stb_image_write.h>
#include <tuple>
#include <vector>

#include "Common/EnumClassFlags.h"
// #include "Common/QuadTree/QuadTree.h"
#include "Common/BaseApplication.h"
// #include "Common/SceneManager.h"
// #include "Common/SceneNode/SceneGeometryNode.h"
// #include "Common/ShaderManager.h"
#include "Common/Log.h"
#include "Common/AssetLoader.h"
// #include "Common/QuadTree/QuadTreeStructures.h"


#include "Modules/ModuleManager.h"
#include "OpenGL/OpenGLBuffer.h"
#include "OpenGL/OpenGLFramebuffer.h"
#include "OpenGL/OpenGLResources.h"
#include "OpenGL/OpenGLTexture.h"
// #include "OpenGL/OpenGLUtils.h"
// #include "OpenGL/OpenGLVertexArrayObject.h"
// #include "OpenGL/OpenGLDepthStencilState.h"
// #include "OpenGL/OpenGLRasterizerState.h"
#include "OpenGL/OpenGLState.h"
#include "OpenGL/OpenGLGraphicsPipelineState.h"
#include "RHIDefinitions.h"
#include "OpenGLDynamicRHI.h"
#include "RHIResources.h"
#include "Templates/ObjectMacros.h"
#include "ShaderInstance.h"


#ifdef _DEBUG
#include "Common/InputManager.h"
#endif // _DEBUG

//#define _DEBUG_SHOWNORMAL
namespace nilou {
    FDynamicRHI *GDynamicRHI = nullptr;
}

namespace nilou {

    static GLenum TranslateCompareFunction(ECompareFunction CompareFunction)
    {
        switch(CompareFunction)
        {
        case CF_Less: return GL_LESS;
        case CF_LessEqual: return GL_LEQUAL;
        case CF_Greater: return GL_GREATER;
        case CF_GreaterEqual: return GL_GEQUAL;
        case CF_Equal: return GL_EQUAL;
        case CF_NotEqual: return GL_NOTEQUAL;
        case CF_Never: return GL_NEVER;
        default: return GL_ALWAYS;
        };
    }

    static GLenum TranslateStencilOp(EStencilOp StencilOp)
    {
        switch(StencilOp)
        {
        case EStencilOp::SO_Zero: return GL_ZERO;
        case EStencilOp::SO_Replace: return GL_REPLACE;
        case EStencilOp::SO_SaturatedIncrement: return GL_INCR;
        case EStencilOp::SO_SaturatedDecrement: return GL_DECR;
        case EStencilOp::SO_Invert: return GL_INVERT;
        case EStencilOp::SO_Increment: return GL_INCR_WRAP;
        case EStencilOp::SO_Decrement: return GL_DECR_WRAP;
        default: return GL_KEEP;
        };
    }

    static GLenum TranslateCullMode(ERasterizerCullMode CullMode)
    {
        switch(CullMode)
        {
        case CM_CW: return GL_BACK;
        case CM_CCW: return GL_FRONT;
        default: return GL_NONE;
        };
    }

    static GLenum TranslateBlendOp(EBlendOperation BlendOp)
    {
        switch(BlendOp)
        {
        case BO_Subtract: return GL_FUNC_SUBTRACT;
        case BO_Min: return GL_MIN;
        case BO_Max: return GL_MAX;
        case BO_ReverseSubtract: return GL_FUNC_REVERSE_SUBTRACT;
        default: return GL_FUNC_ADD;
        };
    }

    static GLenum TranslateBlendFactor(EBlendFactor BlendFactor)
    {
        switch(BlendFactor)
        {
        case BF_One: return GL_ONE;
        case BF_SourceColor: return GL_SRC_COLOR;
        case BF_InverseSourceColor: return GL_ONE_MINUS_SRC_COLOR;
        case BF_SourceAlpha: return GL_SRC_ALPHA;
        case BF_InverseSourceAlpha: return GL_ONE_MINUS_SRC_ALPHA;
        case BF_DestAlpha: return GL_DST_ALPHA;
        case BF_InverseDestAlpha: return GL_ONE_MINUS_DST_ALPHA;
        case BF_DestColor: return GL_DST_COLOR;
        case BF_InverseDestColor: return GL_ONE_MINUS_DST_COLOR;
        case BF_ConstantBlendFactor: return GL_CONSTANT_COLOR;
        case BF_InverseConstantBlendFactor: return GL_ONE_MINUS_CONSTANT_COLOR;
        default: return GL_ZERO;
        };
    }

    static GLenum TranslateFillMode(ERasterizerFillMode FillMode)
    {
        switch(FillMode)
        {
            case FM_Point: return GL_POINT;
            case FM_Wireframe: return GL_LINE;
            default: return GL_FILL;
        };
    }

    static GLenum TranslatePrimitiveMode(EPrimitiveMode PrimitiveMode)
    {
        switch(PrimitiveMode)
        {
            case EPrimitiveMode::PM_Points : return GL_POINTS;
            case EPrimitiveMode::PM_Lines : return GL_LINES;
            case EPrimitiveMode::PM_Line_Loop : return GL_LINE_LOOP;
            case EPrimitiveMode::PM_Line_Strip : return GL_LINE_STRIP;
            case EPrimitiveMode::PM_Triangles : return GL_TRIANGLES;
            case EPrimitiveMode::PM_Triangle_Strip : return GL_TRIANGLE_STRIP;
            case EPrimitiveMode::PM_Triangle_Fan : return GL_TRIANGLE_FAN;
            default: return GL_TRIANGLES;
        };
    }

    static GLenum TranslateIndexBufferStride(uint32 Stride)
    {
        switch(Stride)
        {
            case 1 : return GL_UNSIGNED_BYTE;
            case 2 : return GL_UNSIGNED_SHORT;
            case 4 : return GL_UNSIGNED_INT;
            default: NILOU_LOG(Error, "Invalid index buffer stride: " + std::to_string(Stride)); return GL_UNSIGNED_INT;
        };
    }

    static std::tuple<GLenum, GLint, bool, bool> TranslateVertexElementType(EVertexElementType VertexElementType)
    {
        GLenum DataType;
        GLint Size;
        bool bNormalized;
        bool bShouldConvertToFloat;
        switch(VertexElementType)
        {
            case EVertexElementType::VET_Float1:	DataType = GL_FLOAT;			Size = 1;	bNormalized = false;	bShouldConvertToFloat = true; break;
            case EVertexElementType::VET_Float2:	DataType = GL_FLOAT;			Size = 2;	bNormalized = false;	bShouldConvertToFloat = true; break;
            case EVertexElementType::VET_Float3:	DataType = GL_FLOAT;			Size = 3;	bNormalized = false;	bShouldConvertToFloat = true; break;
            case EVertexElementType::VET_Float4:	DataType = GL_FLOAT;			Size = 4;	bNormalized = false;	bShouldConvertToFloat = true; break;
            case EVertexElementType::VET_UByte4:	DataType = GL_UNSIGNED_BYTE;	Size = 4;	bNormalized = false;	bShouldConvertToFloat = false; break;
            case EVertexElementType::VET_UByte4N:	DataType = GL_UNSIGNED_BYTE;	Size = 4;	bNormalized = true;	    bShouldConvertToFloat = true; break;
            case EVertexElementType::VET_Short2:	DataType = GL_SHORT;			Size = 2;	bNormalized = false;	bShouldConvertToFloat = false; break;
            case EVertexElementType::VET_Short4:	DataType = GL_SHORT;			Size = 4;	bNormalized = false;	bShouldConvertToFloat = false; break;
            case EVertexElementType::VET_Short2N:	DataType = GL_SHORT;			Size = 2;	bNormalized = true;	    bShouldConvertToFloat = true; break;
            case EVertexElementType::VET_Half2:		DataType = GL_HALF_FLOAT;	    Size = 2;	bNormalized = false;	bShouldConvertToFloat = true; break;
            case EVertexElementType::VET_Half4:		DataType = GL_HALF_FLOAT;	    Size = 4;	bNormalized = false;	bShouldConvertToFloat = true; break;
            case EVertexElementType::VET_Short4N:	DataType = GL_SHORT;			Size = 4;	bNormalized = true;	    bShouldConvertToFloat = true; break;
            case EVertexElementType::VET_UShort2:	DataType = GL_UNSIGNED_SHORT;   Size = 2;   bNormalized = false;    bShouldConvertToFloat = false; break;
            case EVertexElementType::VET_UShort4:	DataType = GL_UNSIGNED_SHORT;   Size = 4;   bNormalized = false;    bShouldConvertToFloat = false; break;
            case EVertexElementType::VET_UShort2N:	DataType = GL_UNSIGNED_SHORT;   Size = 2;   bNormalized = true;     bShouldConvertToFloat = true; break;
            case EVertexElementType::VET_UShort4N:	DataType = GL_UNSIGNED_SHORT;   Size = 4;   bNormalized = true;     bShouldConvertToFloat = true; break;
            case EVertexElementType::VET_UInt:		DataType = GL_UNSIGNED_INT;		Size = 1;	bNormalized = false;	bShouldConvertToFloat = false; break;
            default: std::cout << "Unknown RHI vertex element type " << (int)VertexElementType;
        };

        return { DataType, Size, bNormalized, bShouldConvertToFloat };
    }

    static GLenum TranslateTextureFilter(ETextureFilters TextureFilter)
    {
        switch (TextureFilter) 
        {
            case ETextureFilters::TF_Linear: return GL_LINEAR;
            case ETextureFilters::TF_Nearest: return GL_NEAREST;
            case ETextureFilters::TF_Nearest_Mipmap_Nearest: return GL_NEAREST_MIPMAP_NEAREST;
            case ETextureFilters::TF_Linear_Mipmap_Nearest: return GL_LINEAR_MIPMAP_NEAREST;
            case ETextureFilters::TF_Nearest_Mipmap_Linear: return GL_NEAREST_MIPMAP_LINEAR;
            case ETextureFilters::TF_Linear_Mipmap_Linear: return GL_LINEAR_MIPMAP_LINEAR;
            default: NILOU_LOG(Error, "Unknown TextureFilter: " + std::to_string((int)TextureFilter)) return GL_LINEAR;
        }
    }

    static GLenum TranslateWrapMode(ETextureWrapModes TextureWrapMode)
    {
        switch (TextureWrapMode) {
		case ETextureWrapModes::TW_Repeat: return GL_REPEAT;
		case ETextureWrapModes::TW_Clamp: return GL_CLAMP_TO_EDGE;
		case ETextureWrapModes::TW_Mirrored_Repeat: return GL_MIRRORED_REPEAT;
        }
    }

    /* Format, InternalFormat, Type */
    static std::tuple<GLenum, GLenum, GLenum> TranslatePixelFormat(EPixelFormat PixelFormat)
    {
        GLenum Format;
        GLenum InternalFormat;
        GLenum Type;
        switch (PixelFormat) {
            case EPixelFormat::PF_UNKNOWN:      Format = 0;         InternalFormat = 0;                 Type = 0; break;
            case EPixelFormat::PF_R8:           Format = GL_RED;    InternalFormat = GL_R8;             Type = GL_UNSIGNED_BYTE; break;
            case EPixelFormat::PF_R8G8:         Format = GL_RG;     InternalFormat = GL_RG8;            Type = GL_UNSIGNED_BYTE; break;
            case EPixelFormat::PF_R8G8B8:       Format = GL_RGB;    InternalFormat = GL_RGB8;           Type = GL_UNSIGNED_BYTE; break;
            case EPixelFormat::PF_R8G8B8_sRGB:  Format = GL_RGB;    InternalFormat = GL_SRGB8;          Type = GL_UNSIGNED_BYTE; break;
            case EPixelFormat::PF_B8G8R8:       Format = GL_BGR;    InternalFormat = GL_RGB8;           Type = GL_UNSIGNED_BYTE; break;
            case EPixelFormat::PF_B8G8R8_sRGB:  Format = GL_BGR;    InternalFormat = GL_SRGB8;          Type = GL_UNSIGNED_BYTE; break;
            case EPixelFormat::PF_R8G8B8A8:     Format = GL_RGBA;   InternalFormat = GL_RGBA8;          Type = GL_UNSIGNED_BYTE; break;
            case EPixelFormat::PF_R8G8B8A8_sRGB:Format = GL_RGBA;   InternalFormat = GL_SRGB8_ALPHA8;   Type = GL_UNSIGNED_BYTE; break;
            case EPixelFormat::PF_B8G8R8A8:     Format = GL_BGRA;   InternalFormat = GL_RGBA8;          Type = GL_UNSIGNED_BYTE; break;
            case EPixelFormat::PF_B8G8R8A8_sRGB:Format = GL_BGRA;   InternalFormat = GL_SRGB8_ALPHA8;   Type = GL_UNSIGNED_BYTE; break;

            case EPixelFormat::PF_D24S8:        Format = GL_DEPTH_STENCIL;  InternalFormat = GL_DEPTH24_STENCIL8;   Type = GL_UNSIGNED_INT_24_8; break;
            case EPixelFormat::PF_D32F:         Format = GL_DEPTH_COMPONENT;InternalFormat = GL_DEPTH_COMPONENT32F; Type = GL_FLOAT; break;

            case EPixelFormat::PF_DXT1:         Format = GL_RGBA;   InternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;          Type = GL_UNSIGNED_BYTE; break;
            case EPixelFormat::PF_DXT1_sRGB:    Format = GL_RGBA;   InternalFormat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;    Type = GL_UNSIGNED_BYTE; break;
            case EPixelFormat::PF_DXT5:         Format = GL_RGBA;   InternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;          Type = GL_UNSIGNED_BYTE; break;
            case EPixelFormat::PF_DXT5_sRGB:    Format = GL_RGBA;   InternalFormat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;    Type = GL_UNSIGNED_BYTE; break;

            case EPixelFormat::PF_R16F:         Format = GL_RED;    InternalFormat = GL_R16F;   Type = GL_HALF_FLOAT; break;
            case EPixelFormat::PF_R16G16F:      Format = GL_RG;     InternalFormat = GL_RG16F;  Type = GL_HALF_FLOAT; break;
            case EPixelFormat::PF_R16G16B16F:   Format = GL_RGB;     InternalFormat = GL_RGB16F;  Type = GL_HALF_FLOAT; break;
            case EPixelFormat::PF_R16G16B16A16F:Format = GL_RGBA;   InternalFormat = GL_RGBA16F;Type = GL_HALF_FLOAT; break;
            case EPixelFormat::PF_R32F:         Format = GL_RED;    InternalFormat = GL_R32F;   Type = GL_FLOAT; break;
            case EPixelFormat::PF_R32G32F:      Format = GL_RG;     InternalFormat = GL_RG32F;  Type = GL_FLOAT; break;
            case EPixelFormat::PF_R32G32B32F:   Format = GL_RGB;     InternalFormat = GL_RGB32F;  Type = GL_FLOAT; break;
            case EPixelFormat::PF_R32G32B32A32F:Format = GL_RGBA;   InternalFormat = GL_RGBA32F;Type = GL_FLOAT; break;
            default: NILOU_LOG(Error, "Unknown PixelFormat: " + std::to_string((int)PixelFormat)) break;
        }

        return { Format, InternalFormat, Type };
    }

    static uint8 TranslatePixelFormatToBytePerPixel(EPixelFormat PixelFormat)
    {
        switch (PixelFormat) {
		    case EPixelFormat::PF_UNKNOWN: return 0;
		    case EPixelFormat::PF_R8: return 1;
		    case EPixelFormat::PF_R8G8: return 2;
		    case EPixelFormat::PF_R8G8B8: return 3;
		    case EPixelFormat::PF_R8G8B8_sRGB: return 3;
		    case EPixelFormat::PF_B8G8R8: return 3;
		    case EPixelFormat::PF_B8G8R8_sRGB: return 3;
		    case EPixelFormat::PF_R8G8B8A8: return 4;
		    case EPixelFormat::PF_R8G8B8A8_sRGB: return 4;
		    case EPixelFormat::PF_B8G8R8A8: return 4;
		    case EPixelFormat::PF_B8G8R8A8_sRGB: return 4;

		    case EPixelFormat::PF_D24S8: return 4;
		    case EPixelFormat::PF_D32F: return 4;

		    case EPixelFormat::PF_DXT1: return 4;
		    case EPixelFormat::PF_DXT1_sRGB: return 4;
		    case EPixelFormat::PF_DXT5: return 4;
		    case EPixelFormat::PF_DXT5_sRGB: return 4;

		    case EPixelFormat::PF_R16F: return 2;
		    case EPixelFormat::PF_R16G16F: return 4;
		    case EPixelFormat::PF_R16G16B16F: return 6;
		    case EPixelFormat::PF_R16G16B16A16F: return 8;
		    case EPixelFormat::PF_R32F: return 4;
		    case EPixelFormat::PF_R32G32F: return 8;
		    case EPixelFormat::PF_R32G32B32F: return 12;
		    case EPixelFormat::PF_R32G32B32A32F: return 16;
            default: NILOU_LOG(Error, "Unknown PixelFormat: " + std::to_string((int)PixelFormat)) return 0;
        }
    }

}

namespace nilou {

    FOpenGLDynamicRHI::TextureUnitManager::TextureUnitManager()
    {
        Size = 0;
        pool.resize(UnitCapacity);
        std::iota(pool.begin(), pool.end(), 0);
        indices.resize(UnitCapacity);
        std::iota(indices.begin(), indices.end(), 0);
    }

    int FOpenGLDynamicRHI::TextureUnitManager::AllocUnit()
    {
        if (Size >= UnitCapacity)
            throw("run out of texture unit"); 
        int unit_id = pool[Size++];
        allocated_units.push_back(unit_id);
        return unit_id;
    }

    void FOpenGLDynamicRHI::TextureUnitManager::FreeUnit(int unit)
    {
        Size--;
        int last_unit = pool[Size];
        std::swap(pool[indices[unit]], pool[Size]);
        std::swap(indices[unit], indices[last_unit]);
    }

    void FOpenGLDynamicRHI::TextureUnitManager::FreeAllUnit()
    {
        for (int unit_id : allocated_units)
            FreeUnit(unit_id);
        allocated_units.clear();
    }
}


namespace nilou {
    void nilou::FOpenGLDynamicRHI::GLDEBUG()
    {
#ifdef _DEBUG
        auto res = glGetError();
        if (res != 0)
           std::cout << res << std::endl;
#endif // _DEBUG
    }

	void FOpenGLDynamicRHI::RHIBeginRenderPass(const FRHIRenderPassInfo &InInfo)
    {
        RHIBindFramebuffer(InInfo.Framebuffer);

        int ClearBits = 0;
        if (InInfo.bClearColorBuffer)
        {
            // This is used to ensure that the clear operation is correctly excuted
            glColorMask(true, true, true, true);

            glClearColor(InInfo.ClearColor.x, InInfo.ClearColor.y, InInfo.ClearColor.z, InInfo.ClearColor.w);
            ClearBits = ClearBits | GL_COLOR_BUFFER_BIT;
        }

        if (InInfo.bClearDepthBuffer)
        {
            // This is used to ensure that the clear operation is correctly excuted
            glDepthMask(true);

            glClearDepth(InInfo.ClearDepth);
            ClearBits = ClearBits | GL_DEPTH_BUFFER_BIT;
        }

        if (InInfo.bClearStencilBuffer)
        {
            // This is used to ensure that the clear operation is correctly excuted
            glStencilMask(0xff);

            glClearStencil(InInfo.ClearStencil);
            ClearBits = ClearBits | GL_STENCIL_BUFFER_BIT;
        }

        if (InInfo.bClearColorBuffer || InInfo.bClearDepthBuffer || InInfo.bClearStencilBuffer)
        {
            glClear(ClearBits);
        }
    }

    void FOpenGLDynamicRHI::RHISetShaderUniformBuffer(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, RHIUniformBuffer *UniformBufferRHI)
    {
        RHISetShaderUniformBuffer(
            BoundPipelineState, PipelineStage, 
            BoundPipelineState->GetBaseIndexByName(PipelineStage, ParameterName), UniformBufferRHI);
    }

    void FOpenGLDynamicRHI::RHISetShaderUniformBuffer(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, RHIUniformBuffer *UniformBufferRHI)
    {
        if (BoundPipelineState != ContextState.GraphicsPipelineState)
        {
            NILOU_LOG(Error, "RHISetShaderUniformBuffer BoundPipelineState parameter is different from ContextState.GraphicsPipelineState");
            return;
        }

        if (BaseIndex < 0)
        {
            NILOU_LOG(Error, "RHISetShaderUniformBuffer Invalid BaseIndex: " + std::to_string(BaseIndex));
            return;
        }
        OpenGLUniformBuffer *GLUniformBuffer = static_cast<OpenGLUniformBuffer*>(UniformBufferRHI);
        OpenGLGraphicsPipelineState *GLPipelineState = static_cast<OpenGLGraphicsPipelineState*>(BoundPipelineState);
        
        // if (ContextState.GraphicsPipelineState != GLPipelineState)
        //     RHIUseShaderProgram(GLPipelineState->Program.get());

        glBindBufferBase(GL_UNIFORM_BUFFER, BaseIndex, GLUniformBuffer->Resource);

        // if (ContextState.GraphicsPipelineState != GLPipelineState)
        //     RHIUseShaderProgram(ContextState.GraphicsPipelineState->Program.get());
    }
    
    void FOpenGLDynamicRHI::RHISetShaderSampler(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, const FRHISampler &SamplerRHI)
    {
        RHISetShaderSampler(
            BoundPipelineState, PipelineStage, 
            BoundPipelineState->GetBaseIndexByName(PipelineStage, ParameterName), SamplerRHI);
    }

    void FOpenGLDynamicRHI::RHISetShaderSampler(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, const FRHISampler &SamplerRHI)
    {
        if (BoundPipelineState != ContextState.GraphicsPipelineState)
        {
            NILOU_LOG(Error, "RHISetShaderSampler BoundPipelineState parameter is different from ContextState.GraphicsPipelineState");
            return;
        }

        if (BaseIndex < 0)
        {
            NILOU_LOG(Error, "RHISetShaderSampler Invalid BaseIndex: " + std::to_string(BaseIndex));
            return;
        }
        // if (ContextState.GraphicsPipelineState != BoundPipelineState)
        // {
        //     OpenGLGraphicsPipelineState *GLPipelineState = static_cast<OpenGLGraphicsPipelineState*>(BoundPipelineState);
        //     RHIUseShaderProgram(GLPipelineState->Program.get());
        // }

        {
            OpenGLTextureResource GLTexture = TextureResourceCast(SamplerRHI.Texture.get());
            int unit_id = TexMngr.AllocUnit();
            glActiveTexture(GL_TEXTURE0 + unit_id);
            glBindTexture(GLTexture.Target, GLTexture.Resource);
            glTexParameteri(GLTexture.Target, GL_TEXTURE_MAG_FILTER, TranslateTextureFilter(SamplerRHI.Params.Mag_Filter));
            glTexParameteri(GLTexture.Target, GL_TEXTURE_MIN_FILTER, TranslateTextureFilter(SamplerRHI.Params.Min_Filter));
            glTexParameteri(GLTexture.Target, GL_TEXTURE_WRAP_S, TranslateWrapMode(SamplerRHI.Params.Wrap_S));
            glTexParameteri(GLTexture.Target, GL_TEXTURE_WRAP_T, TranslateWrapMode(SamplerRHI.Params.Wrap_T));
            if (GLTexture.Target == GL_TEXTURE_CUBE_MAP || GLTexture.Target == GL_TEXTURE_3D)
                glTexParameteri(GLTexture.Target, GL_TEXTURE_WRAP_R, TranslateWrapMode(SamplerRHI.Params.Wrap_R));
            glUniform1i(BaseIndex, unit_id);
        }

        // if (ContextState.GraphicsPipelineState != BoundPipelineState)
        // {
        //     RHIUseShaderProgram(ContextState.GraphicsPipelineState->Program.get());
        // }
    }

	void FOpenGLDynamicRHI::RHISetShaderImage(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, RHITexture *Image, EDataAccessFlag AccessFlag)
    {
        RHISetShaderImage(
            BoundPipelineState, PipelineStage, 
            BoundPipelineState->GetBaseIndexByName(PipelineStage, ParameterName), Image, AccessFlag);
    }

	void FOpenGLDynamicRHI::RHISetShaderImage(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, RHITexture *Image, EDataAccessFlag AccessFlag)
    {
        if (BoundPipelineState != ContextState.GraphicsPipelineState)
        {
            NILOU_LOG(Error, "RHISetShaderImage BoundPipelineState parameter is different from ContextState.GraphicsPipelineState");
            return;
        }

        GLenum GLAccess;
        switch (AccessFlag) 
        {
            case EDataAccessFlag::DA_ReadOnly: GLAccess = GL_READ_ONLY; break;
            case EDataAccessFlag::DA_WriteOnly: GLAccess = GL_WRITE_ONLY; break;
            case EDataAccessFlag::DA_ReadWrite: GLAccess = GL_READ_WRITE; break;
            default: std::cout << "Invalid Data access flag" << std::endl; break;
        }
        OpenGLTextureResource GLTexture = TextureResourceCast(Image);
        GLDEBUG();
        // auto a = GLTexture->Resource;
        // if (ContextState.GraphicsPipelineState->Program != GLShader->Program)
        //     RHIUseShaderProgram(GLShader->Program.get());
        glBindImageTexture(BaseIndex, GLTexture.Resource, 0, false, 0, GLAccess, GLTexture.InternalFormat);
        // if (ContextState.GraphicsPipelineState->Program != GLShader->Program)
        //     RHIUseShaderProgram(ContextState.GraphicsPipelineState->Program.get());
        GLDEBUG();
    }

	void FOpenGLDynamicRHI::RHISetVertexBuffer(FRHIGraphicsPipelineState *BoundPipelineState, FRHIVertexInput *VertexInput)
    {
        OpenGLBuffer *GLBuffer = static_cast<OpenGLBuffer*>(VertexInput->VertexBuffer);

        auto [DataType, Size, bNormalized, bShouldConvertToFloat] = TranslateVertexElementType(VertexInput->Type);

        glBindBuffer(GL_ARRAY_BUFFER, GLBuffer->Resource);
        glVertexAttribPointer(VertexInput->Location, Size, DataType, bNormalized, GLBuffer->GetStride(), (void*)0);
        glEnableVertexAttribArray(VertexInput->Location);
        ContextState.VertexAttributeEnabled[VertexInput->Location] = true;
    }

	FRHIGraphicsPipelineState *FOpenGLDynamicRHI::RHIGetBoundPipelineState()
    {
        return ContextState.GraphicsPipelineState;
    }

    void AllocateParameterBindingPoint(FRHIPipelineLayout &PipelineLayout, int PipelineResource, const std::set<FShaderParameterInfo> &ParsedParameters, EPipelineStage PipelineStage)
    {
        FRHIDescriptorSet DescriptorSet;
        
        int max_sampler_binding_point = -1;
        for (const FShaderParameterInfo &ParamInfo : ParsedParameters)
        {
            if (ParamInfo.ParameterType == EShaderParameterType::SPT_Sampler) 
            {
                FRHIDescriptorSetLayoutBinding binding;
                binding.Name = ParamInfo.ParameterName;
                binding.ParameterType = ParamInfo.ParameterType;
                int binding_point = glGetUniformLocation(PipelineResource, ParamInfo.ParameterName.c_str());
                if (binding_point == -1)
                    NILOU_LOG(Warning, "Shader parameter " + ParamInfo.ParameterName + " is omitted in glsl")
                GDynamicRHI->GLDEBUG();
                binding.BindingPoint = binding_point;
                DescriptorSet.Bindings[binding.Name] = binding;
                max_sampler_binding_point = std::max(max_sampler_binding_point, binding_point);
            }
            else if (ParamInfo.ParameterType == EShaderParameterType::SPT_Image) 
            {
                FRHIDescriptorSetLayoutBinding binding;
                binding.Name = ParamInfo.ParameterName;
                binding.ParameterType = ParamInfo.ParameterType;
                binding.BindingPoint = ParamInfo.BindingPoint;
                DescriptorSet.Bindings[binding.Name] = binding;
            }
        }

        int uniform_buffer_binding_point = max_sampler_binding_point+1;
        for (const FShaderParameterInfo &ParamInfo : ParsedParameters)
        {
            if (ParamInfo.ParameterType == EShaderParameterType::SPT_UniformBuffer)
            {
                FRHIDescriptorSetLayoutBinding binding;
                binding.Name = ParamInfo.ParameterName;
                binding.ParameterType = ParamInfo.ParameterType;
                int block_index = glGetUniformBlockIndex(PipelineResource, ParamInfo.ParameterName.c_str());
                GDynamicRHI->GLDEBUG();
                glUniformBlockBinding(PipelineResource, block_index, uniform_buffer_binding_point);
                GDynamicRHI->GLDEBUG();
                binding.BindingPoint = uniform_buffer_binding_point;
                DescriptorSet.Bindings[binding.Name] = binding;
                uniform_buffer_binding_point += 1;
            }
            // else if (ParamInfo.ParameterType == EShaderParameterType::SPT_ShaderStructureBuffer)
            // {
            //     FRHIDescriptorSetLayoutBinding binding;
            //     binding.Name = ParamInfo.ParameterName;
            //     binding.ParameterType = ParamInfo.ParameterType;
            //     int block_index = glGetProgramResourceIndex(PipelineResource, GL_SHADER_STORAGE_BLOCK, ParamInfo.ParameterName.c_str());
            //     GDynamicRHI->GLDEBUG();
            //     glShaderStorageBlockBinding(PipelineResource, block_index, uniform_buffer_binding_point);
            //     GDynamicRHI->GLDEBUG();
            //     binding.BindingPoint = uniform_buffer_binding_point;
            //     DescriptorSet.Bindings[binding.Name] = binding;
            //     uniform_buffer_binding_point += 1;
            // }
        }

        PipelineLayout.DescriptorSets[PipelineStage] = DescriptorSet;
    }

    FRHIGraphicsPipelineState *FOpenGLDynamicRHI::RHIGetOrCreatePipelineStateObject(const FRHIGraphicsPipelineInitializer &Initializer)
    {
        GLDEBUG();
        if (CachedPipelineStateObjects.find(Initializer) != CachedPipelineStateObjects.end())
        {
            return CachedPipelineStateObjects[Initializer].get();
        }

        OpenGLGraphicsPipelineStateRef PSO = std::make_shared<OpenGLGraphicsPipelineState>();
        PSO->Initializer = Initializer;
        if (Initializer.ComputeShader != nullptr)
        {
            OpenGLComputeShader *comp = static_cast<OpenGLComputeShader *>(Initializer.ComputeShader->Shader.get());
            PSO->Program = RHICreateLinkedProgram(comp);
            FRHIPipelineLayout &PipelineLayout = PSO->PipelineLayout;
            const std::set<FShaderParameterInfo> &ComputeShaderParams = Initializer.ComputeShader->Parameters;
            AllocateParameterBindingPoint(PSO->PipelineLayout, PSO->Program->Resource, ComputeShaderParams, EPipelineStage::PS_Compute);
        }
        else if (
            Initializer.VertexShader != nullptr && 
            Initializer.PixelShader != nullptr)
        {
            GLDEBUG();
            OpenGLVertexShader *vert = static_cast<OpenGLVertexShader *>(Initializer.VertexShader->Shader.get());
            OpenGLPixelShader *frag = static_cast<OpenGLPixelShader *>(Initializer.PixelShader->Shader.get());

            PSO->Program = RHICreateLinkedProgram(vert, frag);
            GLDEBUG();

            FRHIPipelineLayout &PipelineLayout = PSO->PipelineLayout;
            const std::set<FShaderParameterInfo> &VertexShaderParams = Initializer.VertexShader->Parameters;
            const std::set<FShaderParameterInfo> &PixelShaderParams = Initializer.PixelShader->Parameters;
            AllocateParameterBindingPoint(PSO->PipelineLayout, PSO->Program->Resource, VertexShaderParams, EPipelineStage::PS_Vertex);
            AllocateParameterBindingPoint(PSO->PipelineLayout, PSO->Program->Resource, PixelShaderParams, EPipelineStage::PS_Pixel);

        }

        CachedPipelineStateObjects[Initializer] = PSO;

        GLDEBUG();
        return PSO.get();
    }

    RHIVertexShaderRef FOpenGLDynamicRHI::RHICreateVertexShader(const char *code)
    {
        nilou::OpenGLVertexShaderRef vert = std::make_shared<nilou::OpenGLVertexShader>(code);

        if (!vert->Success())
        {
            NILOU_LOG(Info, code)
            return nullptr;
        }

        return vert;
    }
    RHIPixelShaderRef FOpenGLDynamicRHI::RHICreatePixelShader(const char *code)
    {
        nilou::OpenGLPixelShaderRef pixel = std::make_shared<nilou::OpenGLPixelShader>(code);

        if (!pixel->Success())
        {
            NILOU_LOG(Info, code)
            return nullptr;
        }

        return pixel;
    }
    RHIComputeShaderRef FOpenGLDynamicRHI::RHICreateComputeShader(const char *code)
    {
        nilou::OpenGLComputeShaderRef comp = std::make_shared<nilou::OpenGLComputeShader>(code);

        if (!comp->Success())
        {
            NILOU_LOG(Info, code)
            return nullptr;
        }

        return comp;
    }
    OpenGLLinkedProgramRef FOpenGLDynamicRHI::RHICreateLinkedProgram(OpenGLVertexShader *vert, OpenGLPixelShader *pixel)
    {
        nilou::OpenGLLinkedProgramRef program = std::make_shared<nilou::OpenGLLinkedProgram>(vert, pixel);

        if (!program->Success())
            return nullptr;

        return program;
    }
    OpenGLLinkedProgramRef FOpenGLDynamicRHI::RHICreateLinkedProgram(OpenGLComputeShader *comp)
    {
        nilou::OpenGLLinkedProgramRef program = std::make_shared<nilou::OpenGLLinkedProgram>(comp);

        if (!program->Success())
            return nullptr;

        return program;
    }

    void FOpenGLDynamicRHI::RHIUseShaderProgram(OpenGLLinkedProgram *program)
    {
        OpenGLLinkedProgram *GLProgram = static_cast<OpenGLLinkedProgram*>(program);
        glUseProgram(GLProgram->Resource);
    }

    void FOpenGLDynamicRHI::RHISetRasterizerState(RHIRasterizerState *newState)
    {
        OpenGLRasterizerState* RasterizerState = static_cast<OpenGLRasterizerState*>(newState);
        if (ContextState.RasterizerState.FillMode != RasterizerState->FillMode)
        {
            glPolygonMode(GL_FRONT_AND_BACK, RasterizerState->FillMode);
            ContextState.RasterizerState.FillMode = RasterizerState->FillMode;
        }
        if (ContextState.RasterizerState.CullMode != RasterizerState->CullMode)
        {        
            if (RasterizerState->CullMode != GL_NONE)
            {
                glEnable(GL_CULL_FACE);
                glCullFace(RasterizerState->CullMode);
            }
            else 
            {
                glDisable(GL_CULL_FACE);
            }
            ContextState.RasterizerState.CullMode = RasterizerState->CullMode;
        }

        // glFrontFace(map_RasterizerFrontFace_GLenum[(uint8)newState.FrontFace]);
        // glCullFace(map_RasterizerCullMode_GLenum[(uint8)newState.CullMode]);
        // glPolygonMode(map_RasterizerPolyMode_Face_GLenum[(uint8)newState.PolyMode_Face], 
        //     map_RasterizerPolyMode_Mode_GLenum[(uint8)newState.PolyMode_Mode]);

        // if (newState.EnableCull)
        //     glEnable(GL_CULL_FACE);
        // else
        //     glDisable(GL_CULL_FACE);
    }

    void FOpenGLDynamicRHI::RHISetDepthStencilState(RHIDepthStencilState *newState, uint32 StencilRef)
    {
        OpenGLDepthStencilState* DepthStencilState = static_cast<OpenGLDepthStencilState*>(newState);
        if (ContextState.DepthStencilState.bZEnable != DepthStencilState->bZEnable)
        {
            if (DepthStencilState->bZEnable)
                glEnable(GL_DEPTH_TEST);
            else
                glDisable(GL_DEPTH_TEST);
            ContextState.DepthStencilState.bZEnable = DepthStencilState->bZEnable;
        }

        if (ContextState.DepthStencilState.ZFunc != DepthStencilState->ZFunc)
        {
            glDepthFunc(DepthStencilState->ZFunc);
            ContextState.DepthStencilState.ZFunc = DepthStencilState->ZFunc;
        }
        if (ContextState.DepthStencilState.bZWriteEnable != DepthStencilState->bZWriteEnable)
        {
		    glDepthMask(DepthStencilState->bZWriteEnable);
            ContextState.DepthStencilState.bZWriteEnable = DepthStencilState->bZWriteEnable;
        }

        if (ContextState.DepthStencilState.bStencilEnable != DepthStencilState->bStencilEnable)
        {
            if (DepthStencilState->bStencilEnable)
            {
                glEnable(GL_STENCIL_TEST);
            }
            else
            {
                glDisable(GL_STENCIL_TEST);
            }
            ContextState.DepthStencilState.bStencilEnable = DepthStencilState->bStencilEnable;
        }

		if (DepthStencilState->bStencilEnable)
		{
            if (DepthStencilState->bTwoSidedStencilMode)
            {
                if (ContextState.DepthStencilState.StencilFunc != DepthStencilState->StencilFunc
                    || ContextState.StencilRef != StencilRef
                    || ContextState.DepthStencilState.StencilReadMask != DepthStencilState->StencilReadMask)
                {
                    glStencilFuncSeparate(GL_FRONT, DepthStencilState->StencilFunc, StencilRef, DepthStencilState->StencilReadMask);
                    ContextState.DepthStencilState.StencilFunc = DepthStencilState->StencilFunc;
                }

                if (ContextState.DepthStencilState.StencilFail != DepthStencilState->StencilFail
                    || ContextState.DepthStencilState.StencilZFail != DepthStencilState->StencilZFail
                    || ContextState.DepthStencilState.StencilPass != DepthStencilState->StencilPass)
                {
                    glStencilOpSeparate(GL_FRONT, DepthStencilState->StencilFail, DepthStencilState->StencilZFail, DepthStencilState->StencilPass);
                    ContextState.DepthStencilState.StencilFail = DepthStencilState->StencilFail;
                    ContextState.DepthStencilState.StencilZFail = DepthStencilState->StencilZFail;
                    ContextState.DepthStencilState.StencilPass = DepthStencilState->StencilPass;
                }
                
                if (ContextState.DepthStencilState.BackStencilFunc != DepthStencilState->BackStencilFunc
                    || ContextState.StencilRef != StencilRef
                    || ContextState.DepthStencilState.StencilReadMask != DepthStencilState->StencilReadMask)
                {
                    glStencilFuncSeparate(GL_BACK, DepthStencilState->BackStencilFunc, StencilRef, DepthStencilState->StencilReadMask);
                    ContextState.DepthStencilState.BackStencilFunc = DepthStencilState->BackStencilFunc;
                }

                if (ContextState.DepthStencilState.BackStencilFail != DepthStencilState->BackStencilFail
                    || ContextState.DepthStencilState.BackStencilZFail != DepthStencilState->BackStencilZFail
                    || ContextState.DepthStencilState.BackStencilPass != DepthStencilState->BackStencilPass)
                {
                    glStencilOpSeparate(GL_BACK, DepthStencilState->BackStencilFail, DepthStencilState->BackStencilZFail, DepthStencilState->BackStencilPass);
                    ContextState.DepthStencilState.BackStencilFail = DepthStencilState->BackStencilFail;
                    ContextState.DepthStencilState.BackStencilZFail = DepthStencilState->BackStencilZFail;
                    ContextState.DepthStencilState.BackStencilPass = DepthStencilState->BackStencilPass;
                }

                ContextState.StencilRef = StencilRef;
                ContextState.DepthStencilState.StencilReadMask = DepthStencilState->StencilReadMask;
            }
            else
            {
                
                if (ContextState.DepthStencilState.StencilFunc != DepthStencilState->StencilFunc
                    || ContextState.StencilRef != StencilRef
                    || ContextState.DepthStencilState.StencilReadMask != DepthStencilState->StencilReadMask)
                {
                    glStencilFunc(DepthStencilState->StencilFunc, StencilRef, DepthStencilState->StencilReadMask);
                    ContextState.DepthStencilState.StencilFunc = DepthStencilState->StencilFunc;
                    ContextState.StencilRef = StencilRef;
                    ContextState.DepthStencilState.StencilReadMask = DepthStencilState->StencilReadMask;
                }

                if (ContextState.DepthStencilState.StencilFail != DepthStencilState->StencilFail
                    || ContextState.DepthStencilState.StencilZFail != DepthStencilState->StencilZFail
                    || ContextState.DepthStencilState.StencilPass != DepthStencilState->StencilPass)
                {
                    glStencilOp(DepthStencilState->StencilFail, DepthStencilState->StencilZFail, DepthStencilState->StencilPass);
                    ContextState.DepthStencilState.StencilFail = DepthStencilState->StencilFail;
                    ContextState.DepthStencilState.StencilZFail = DepthStencilState->StencilZFail;
                    ContextState.DepthStencilState.StencilPass = DepthStencilState->StencilPass;
                }
            }

            if (ContextState.DepthStencilState.StencilWriteMask != DepthStencilState->StencilWriteMask)
            {
                glStencilMask(DepthStencilState->StencilWriteMask);
                ContextState.DepthStencilState.StencilWriteMask = DepthStencilState->StencilWriteMask;
            }
        }
    }

    void FOpenGLDynamicRHI::RHISetBlendState(RHIBlendState *newState)
    {
        OpenGLBlendState* BlendState = static_cast<OpenGLBlendState*>(newState);
	    bool bABlendWasSet = false;
        for (uint32 RenderTargetIndex = 0;RenderTargetIndex < MAX_SIMULTANEOUS_RENDERTARGETS; ++RenderTargetIndex)
        {
            // if(PendingState.RenderTargets[RenderTargetIndex] == 0)
            // {
            //     // Even if on this stage blend states are incompatible with other stages, we can disregard it, as no render target is assigned to it.
            //     continue;
            // }
            // else if (RenderTargetIndex == 0)
            // {
            //     FOpenGLTexture2D* RenderTarget2D = (FOpenGLTexture2D*)PendingState.RenderTargets[RenderTargetIndex];
            //     bMSAAEnabled = RenderTarget2D->GetNumSamples() > 1 || RenderTarget2D->GetNumSamplesTileMem() > 1;
            // }

            const OpenGLBlendState::FRenderTarget& RenderTargetBlendState = BlendState->RenderTargets[RenderTargetIndex];
            OpenGLBlendState::FRenderTarget& CachedRenderTargetBlendState = ContextState.BlendState.RenderTargets[RenderTargetIndex];

            if (CachedRenderTargetBlendState.bAlphaBlendEnable != RenderTargetBlendState.bAlphaBlendEnable)
            {
                if (RenderTargetBlendState.bAlphaBlendEnable)
                {
                    glEnablei(GL_BLEND, RenderTargetIndex);
                }
                else
                {
                    glDisablei(GL_BLEND, RenderTargetIndex);
                }
			    CachedRenderTargetBlendState.bAlphaBlendEnable = RenderTargetBlendState.bAlphaBlendEnable;
            }

            if (RenderTargetBlendState.bAlphaBlendEnable)
            {
                if ( true/*FOpenGL::SupportsSeparateAlphaBlend()*/ )
                {
                    // Set current blend per stage
                    if (RenderTargetBlendState.bSeparateAlphaBlendEnable)
                    {
                        if (CachedRenderTargetBlendState.ColorSourceBlendFactor != RenderTargetBlendState.ColorSourceBlendFactor
                            || CachedRenderTargetBlendState.ColorDestBlendFactor != RenderTargetBlendState.ColorDestBlendFactor
                            || CachedRenderTargetBlendState.AlphaSourceBlendFactor != RenderTargetBlendState.AlphaSourceBlendFactor
                            || CachedRenderTargetBlendState.AlphaDestBlendFactor != RenderTargetBlendState.AlphaDestBlendFactor)
                        {
                            glBlendFuncSeparatei(
                                RenderTargetIndex, 
                                RenderTargetBlendState.ColorSourceBlendFactor, RenderTargetBlendState.ColorDestBlendFactor,
                                RenderTargetBlendState.AlphaSourceBlendFactor, RenderTargetBlendState.AlphaDestBlendFactor
                                );
                        }

                        if (CachedRenderTargetBlendState.ColorBlendOperation != RenderTargetBlendState.ColorBlendOperation
                            || CachedRenderTargetBlendState.AlphaBlendOperation != RenderTargetBlendState.AlphaBlendOperation)
                        {
                            glBlendEquationSeparatei(
                                RenderTargetIndex, 
                                RenderTargetBlendState.ColorBlendOperation,
                                RenderTargetBlendState.AlphaBlendOperation
                                );
                        }
                    }
                    else
                    {
                        if (CachedRenderTargetBlendState.ColorSourceBlendFactor != RenderTargetBlendState.ColorSourceBlendFactor
                            || CachedRenderTargetBlendState.ColorDestBlendFactor != RenderTargetBlendState.ColorDestBlendFactor
                            || CachedRenderTargetBlendState.AlphaSourceBlendFactor != RenderTargetBlendState.AlphaSourceBlendFactor
                            || CachedRenderTargetBlendState.AlphaDestBlendFactor != RenderTargetBlendState.AlphaDestBlendFactor)
                        {
                            glBlendFunci(RenderTargetIndex, RenderTargetBlendState.ColorSourceBlendFactor, RenderTargetBlendState.ColorDestBlendFactor);
                        }

                        if (CachedRenderTargetBlendState.ColorBlendOperation != RenderTargetBlendState.ColorBlendOperation)
                        {
                            glBlendEquationi(RenderTargetIndex, RenderTargetBlendState.ColorBlendOperation);
                        }
                    }

                    CachedRenderTargetBlendState.bSeparateAlphaBlendEnable = RenderTargetBlendState.bSeparateAlphaBlendEnable;
                    CachedRenderTargetBlendState.ColorBlendOperation = RenderTargetBlendState.ColorBlendOperation;
                    CachedRenderTargetBlendState.ColorSourceBlendFactor = RenderTargetBlendState.ColorSourceBlendFactor;
                    CachedRenderTargetBlendState.ColorDestBlendFactor = RenderTargetBlendState.ColorDestBlendFactor;
                    if( RenderTargetBlendState.bSeparateAlphaBlendEnable )
                    { 
                        CachedRenderTargetBlendState.AlphaSourceBlendFactor = RenderTargetBlendState.AlphaSourceBlendFactor;
                        CachedRenderTargetBlendState.AlphaDestBlendFactor = RenderTargetBlendState.AlphaDestBlendFactor;
                    }
                    else
                    {
                        CachedRenderTargetBlendState.AlphaSourceBlendFactor = RenderTargetBlendState.ColorSourceBlendFactor;
                        CachedRenderTargetBlendState.AlphaDestBlendFactor = RenderTargetBlendState.ColorDestBlendFactor;
                    }
                }
                else
                {
                    if( bABlendWasSet )
                    {
                        // Detect the case of subsequent render target needing different blend setup than one already set in this call.
                        if( CachedRenderTargetBlendState.bSeparateAlphaBlendEnable != RenderTargetBlendState.bSeparateAlphaBlendEnable
                            || CachedRenderTargetBlendState.ColorBlendOperation != RenderTargetBlendState.ColorBlendOperation
                            || CachedRenderTargetBlendState.ColorSourceBlendFactor != RenderTargetBlendState.ColorSourceBlendFactor
                            || CachedRenderTargetBlendState.ColorDestBlendFactor != RenderTargetBlendState.ColorDestBlendFactor
                            || ( RenderTargetBlendState.bSeparateAlphaBlendEnable && 
                                ( CachedRenderTargetBlendState.AlphaSourceBlendFactor != RenderTargetBlendState.AlphaSourceBlendFactor
                                || CachedRenderTargetBlendState.AlphaDestBlendFactor != RenderTargetBlendState.AlphaDestBlendFactor
                                )
                                )
                            )
                            NILOU_LOG(Error, "OpenGL state on draw requires setting different blend operation or factors to different render targets. This is not supported on Mac OS X!");
                    }
                    else
                    {
                        // Set current blend to all stages
                        if (RenderTargetBlendState.bSeparateAlphaBlendEnable)
                        {
                            if (CachedRenderTargetBlendState.ColorSourceBlendFactor != RenderTargetBlendState.ColorSourceBlendFactor
                                || CachedRenderTargetBlendState.ColorDestBlendFactor != RenderTargetBlendState.ColorDestBlendFactor
                                || CachedRenderTargetBlendState.AlphaSourceBlendFactor != RenderTargetBlendState.AlphaSourceBlendFactor
                                || CachedRenderTargetBlendState.AlphaDestBlendFactor != RenderTargetBlendState.AlphaDestBlendFactor)
                            {
                                glBlendFuncSeparate(
                                    RenderTargetBlendState.ColorSourceBlendFactor, RenderTargetBlendState.ColorDestBlendFactor,
                                    RenderTargetBlendState.AlphaSourceBlendFactor, RenderTargetBlendState.AlphaDestBlendFactor
                                    );
                            }

                            if (CachedRenderTargetBlendState.ColorBlendOperation != RenderTargetBlendState.ColorBlendOperation
                                || CachedRenderTargetBlendState.AlphaBlendOperation != RenderTargetBlendState.AlphaBlendOperation)
                            {
                                glBlendEquationSeparate(
                                    RenderTargetBlendState.ColorBlendOperation,
                                    RenderTargetBlendState.AlphaBlendOperation
                                    );
                            }
                        }
                        else
                        {
                            if (CachedRenderTargetBlendState.ColorSourceBlendFactor != RenderTargetBlendState.ColorSourceBlendFactor
                                || CachedRenderTargetBlendState.ColorDestBlendFactor != RenderTargetBlendState.ColorDestBlendFactor
                                || CachedRenderTargetBlendState.AlphaSourceBlendFactor != RenderTargetBlendState.AlphaSourceBlendFactor
                                || CachedRenderTargetBlendState.AlphaDestBlendFactor != RenderTargetBlendState.AlphaDestBlendFactor)
                            {
                                glBlendFunc(RenderTargetBlendState.ColorSourceBlendFactor, RenderTargetBlendState.ColorDestBlendFactor);
                            }

                            if (CachedRenderTargetBlendState.ColorBlendOperation != RenderTargetBlendState.ColorBlendOperation
                                || CachedRenderTargetBlendState.AlphaBlendOperation != RenderTargetBlendState.ColorBlendOperation)
                            {
                                glBlendEquation(RenderTargetBlendState.ColorBlendOperation);
                            }
                        }

                        // Set cached values of all stages to what they were set by global calls, common to all stages
                        for(uint32 RenderTargetIndex2 = 0; RenderTargetIndex2 < MAX_SIMULTANEOUS_RENDERTARGETS; ++RenderTargetIndex2 )
                        {
                            OpenGLBlendState::FRenderTarget& CachedRenderTargetBlendState2 = ContextState.BlendState.RenderTargets[RenderTargetIndex2];
                            CachedRenderTargetBlendState2.bSeparateAlphaBlendEnable = RenderTargetBlendState.bSeparateAlphaBlendEnable;
                            CachedRenderTargetBlendState2.ColorBlendOperation = RenderTargetBlendState.ColorBlendOperation;
                            CachedRenderTargetBlendState2.ColorSourceBlendFactor = RenderTargetBlendState.ColorSourceBlendFactor;
                            CachedRenderTargetBlendState2.ColorDestBlendFactor = RenderTargetBlendState.ColorDestBlendFactor;
                            if( RenderTargetBlendState.bSeparateAlphaBlendEnable )
                            {
                                CachedRenderTargetBlendState2.AlphaBlendOperation = RenderTargetBlendState.AlphaBlendOperation;
                                CachedRenderTargetBlendState2.AlphaSourceBlendFactor = RenderTargetBlendState.AlphaSourceBlendFactor;
                                CachedRenderTargetBlendState2.AlphaDestBlendFactor = RenderTargetBlendState.AlphaDestBlendFactor;
                                CachedRenderTargetBlendState2.AlphaBlendOperation = RenderTargetBlendState.AlphaBlendOperation;
                            }
                            else
                            {
                                CachedRenderTargetBlendState2.AlphaBlendOperation = RenderTargetBlendState.ColorBlendOperation;
                                CachedRenderTargetBlendState2.AlphaSourceBlendFactor = RenderTargetBlendState.ColorSourceBlendFactor;
                                CachedRenderTargetBlendState2.AlphaDestBlendFactor = RenderTargetBlendState.ColorDestBlendFactor;
                                CachedRenderTargetBlendState2.AlphaBlendOperation = RenderTargetBlendState.ColorBlendOperation;
                            }
                        }

                        bABlendWasSet = true;
                    }
                }
            }

            CachedRenderTargetBlendState.bSeparateAlphaBlendEnable = RenderTargetBlendState.bSeparateAlphaBlendEnable;

            if(CachedRenderTargetBlendState.ColorWriteMaskR != RenderTargetBlendState.ColorWriteMaskR
                || CachedRenderTargetBlendState.ColorWriteMaskG != RenderTargetBlendState.ColorWriteMaskG
                || CachedRenderTargetBlendState.ColorWriteMaskB != RenderTargetBlendState.ColorWriteMaskB
                || CachedRenderTargetBlendState.ColorWriteMaskA != RenderTargetBlendState.ColorWriteMaskA)
            {
                glColorMaski(
                    RenderTargetIndex,
                    RenderTargetBlendState.ColorWriteMaskR,
                    RenderTargetBlendState.ColorWriteMaskG,
                    RenderTargetBlendState.ColorWriteMaskB,
                    RenderTargetBlendState.ColorWriteMaskA
                    );

                CachedRenderTargetBlendState.ColorWriteMaskR = RenderTargetBlendState.ColorWriteMaskR;
                CachedRenderTargetBlendState.ColorWriteMaskG = RenderTargetBlendState.ColorWriteMaskG;
                CachedRenderTargetBlendState.ColorWriteMaskB = RenderTargetBlendState.ColorWriteMaskB;
                CachedRenderTargetBlendState.ColorWriteMaskA = RenderTargetBlendState.ColorWriteMaskA;
            }
        }

        // PendingState.bAlphaToCoverageEnabled = bMSAAEnabled && PendingState.BlendState.bUseAlphaToCoverage;
        // if (PendingState.bAlphaToCoverageEnabled != ContextState.bAlphaToCoverageEnabled)
        // {
        //     if (PendingState.bAlphaToCoverageEnabled)
        //     {
        //         glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        //     }
        //     else
        //     {
        //         glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        //     }

        //     ContextState.bAlphaToCoverageEnabled = PendingState.bAlphaToCoverageEnabled;
        // }
    }

    void FOpenGLDynamicRHI::RHISetGraphicsPipelineState(FRHIGraphicsPipelineState *NewState)
    {
        OpenGLGraphicsPipelineState *GLState = static_cast<OpenGLGraphicsPipelineState *>(NewState);
        // FRHIGraphicsPipelineInitializer &Initializer = GLState->Initializer;
        RHIUseShaderProgram(GLState->Program.get());
        // RHISetDepthStencilState(StateData.DepthStentilState.get());
        // RHISetRasterizerState(StateData.RasterizerState.get());
        // RHISetBlendState(StateData.BlendState.get());
        ContextState.GraphicsPipelineState = GLState;
    }

	FRHIGraphicsPipelineState *FOpenGLDynamicRHI::RHISetComputeShader(FShaderInstance *ComputeShader)
    {
        GLDEBUG();
        FRHIGraphicsPipelineInitializer Initializer;
        Initializer.ComputeShader = ComputeShader;
        FRHIGraphicsPipelineState *PSO = GDynamicRHI->RHIGetOrCreatePipelineStateObject(Initializer);
        GDynamicRHI->RHISetGraphicsPipelineState(PSO);
        GLDEBUG();
        return PSO;
    }

    RHIDepthStencilStateRef FOpenGLDynamicRHI::RHICreateDepthStencilState(const FDepthStencilStateInitializer &Initializer)
    {
        OpenGLDepthStencilStateRef DepthStencilState = std::make_shared<OpenGLDepthStencilState>();
        DepthStencilState->bZEnable = Initializer.DepthTest != CF_Always || Initializer.bEnableDepthWrite;
        DepthStencilState->bZWriteEnable = Initializer.bEnableDepthWrite;
        DepthStencilState->ZFunc = TranslateCompareFunction(Initializer.DepthTest);
        DepthStencilState->bStencilEnable = Initializer.bEnableFrontFaceStencil || Initializer.bEnableBackFaceStencil;
        DepthStencilState->bTwoSidedStencilMode = Initializer.bEnableBackFaceStencil;
        DepthStencilState->StencilFunc = TranslateCompareFunction(Initializer.FrontFaceStencilTest);
        DepthStencilState->StencilFail = TranslateStencilOp(Initializer.FrontFaceStencilFailStencilOp);
        DepthStencilState->StencilZFail = TranslateStencilOp(Initializer.FrontFaceDepthFailStencilOp);
        DepthStencilState->StencilPass = TranslateStencilOp(Initializer.FrontFacePassStencilOp);
        DepthStencilState->BackStencilFunc = TranslateCompareFunction(Initializer.BackFaceStencilTest);
        DepthStencilState->BackStencilFail = TranslateStencilOp(Initializer.BackFaceStencilFailStencilOp);
        DepthStencilState->BackStencilZFail = TranslateStencilOp(Initializer.BackFaceDepthFailStencilOp);
        DepthStencilState->BackStencilPass = TranslateStencilOp(Initializer.BackFacePassStencilOp);
        DepthStencilState->StencilReadMask = Initializer.StencilReadMask;
        DepthStencilState->StencilWriteMask = Initializer.StencilWriteMask;

        return DepthStencilState;
    }
    
	RHIRasterizerStateRef FOpenGLDynamicRHI::RHICreateRasterizerState(const FRasterizerStateInitializer &Initializer)
    {
        OpenGLRasterizerStateRef RasterizerState = std::make_shared<OpenGLRasterizerState>();

        RasterizerState->CullMode = TranslateCullMode(Initializer.CullMode);
        RasterizerState->FillMode = TranslateFillMode(Initializer.FillMode);
        
        return RasterizerState;
    }
	
    RHIBlendStateRef FOpenGLDynamicRHI::RHICreateBlendState(const FBlendStateInitializer &Initializer)
    {
        OpenGLBlendStateRef BlendState = std::make_shared<OpenGLBlendState>();
        // BlendState->bUseAlphaToCoverage = Initializer.bUseAlphaToCoverage;
        for(uint32 RenderTargetIndex = 0;RenderTargetIndex < MAX_SIMULTANEOUS_RENDERTARGETS;++RenderTargetIndex)
        {
            const FBlendStateInitializer::FRenderTarget& RenderTargetInitializer =
                Initializer.bUseIndependentRenderTargetBlendStates
                ? Initializer.RenderTargets[RenderTargetIndex]
                : Initializer.RenderTargets[0];
            OpenGLBlendState::FRenderTarget& RenderTarget = BlendState->RenderTargets[RenderTargetIndex];
            RenderTarget.bAlphaBlendEnable = 
                RenderTargetInitializer.ColorBlendOp != BO_Add || RenderTargetInitializer.ColorDestBlend != BF_Zero || RenderTargetInitializer.ColorSrcBlend != BF_One ||
                RenderTargetInitializer.AlphaBlendOp != BO_Add || RenderTargetInitializer.AlphaDestBlend != BF_Zero || RenderTargetInitializer.AlphaSrcBlend != BF_One;
            RenderTarget.ColorBlendOperation = TranslateBlendOp(RenderTargetInitializer.ColorBlendOp);
            RenderTarget.ColorSourceBlendFactor = TranslateBlendFactor(RenderTargetInitializer.ColorSrcBlend);
            RenderTarget.ColorDestBlendFactor = TranslateBlendFactor(RenderTargetInitializer.ColorDestBlend);
            RenderTarget.bSeparateAlphaBlendEnable =
                RenderTargetInitializer.AlphaDestBlend != RenderTargetInitializer.ColorDestBlend ||
                RenderTargetInitializer.AlphaSrcBlend != RenderTargetInitializer.ColorSrcBlend;
            RenderTarget.AlphaBlendOperation = TranslateBlendOp(RenderTargetInitializer.AlphaBlendOp);
            RenderTarget.AlphaSourceBlendFactor = TranslateBlendFactor(RenderTargetInitializer.AlphaSrcBlend);
            RenderTarget.AlphaDestBlendFactor = TranslateBlendFactor(RenderTargetInitializer.AlphaDestBlend);
            RenderTarget.ColorWriteMaskR = (RenderTargetInitializer.ColorWriteMask & CW_RED) != 0;
            RenderTarget.ColorWriteMaskG = (RenderTargetInitializer.ColorWriteMask & CW_GREEN) != 0;
            RenderTarget.ColorWriteMaskB = (RenderTargetInitializer.ColorWriteMask & CW_BLUE) != 0;
            RenderTarget.ColorWriteMaskA = (RenderTargetInitializer.ColorWriteMask & CW_ALPHA) != 0;
        }
        
        return BlendState;
    }

	RHIBufferRef FOpenGLDynamicRHI::RHICreateBuffer(uint32 Stride, uint32 Size, EBufferUsageFlags InUsage, void *Data)
    {
        OpenGLBufferRef Buffer = std::make_shared<OpenGLBuffer>(Stride, Size, InUsage, Data);
        return Buffer;
    }

	RHIUniformBufferRef FOpenGLDynamicRHI::RHICreateUniformBuffer(uint32 Size, EUniformBufferUsage InUsage, void *Data)
    {
        OpenGLUniformBufferRef Buffer = std::make_shared<OpenGLUniformBuffer>(Size, InUsage, Data);
        return Buffer;
    }

    void FOpenGLDynamicRHI::RHIUpdateUniformBuffer(RHIUniformBufferRef UniformBuffer, void *Data)
    {
        OpenGLUniformBufferRef GLBuffer = std::static_pointer_cast<OpenGLUniformBuffer>(UniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, GLBuffer->Resource);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, GLBuffer->GetSize(), Data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
    RHIBufferRef FOpenGLDynamicRHI::RHICreateShaderStorageBuffer(unsigned int DataByteLength, void *Data)
    {
        return RHICreateBuffer(DataByteLength, DataByteLength, EBufferUsageFlags::StructuredBuffer | EBufferUsageFlags::Dynamic, Data);
    }
    RHIBufferRef FOpenGLDynamicRHI::RHICreateAtomicCounterBuffer(unsigned int Value)
    {
        return RHICreateBuffer(4, 4, EBufferUsageFlags::AtomicCounter | EBufferUsageFlags::Dynamic, &Value);
    }
    RHIBufferRef FOpenGLDynamicRHI::RHICreateDispatchIndirectBuffer(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z)
    {
        DispatchIndirectCommand command{ num_groups_x, num_groups_y, num_groups_z };
        return RHICreateBuffer(sizeof(command), sizeof(command), EBufferUsageFlags::DispatchIndirect | EBufferUsageFlags::Dynamic, &command);
    }

    RHITexture2DRef FOpenGLDynamicRHI::RHICreateTexture2D(
        const std::string &name, EPixelFormat InFormat, int32 NumMips, uint32 InSizeX, uint32 InSizeY, void *data
    )
    {
        OpenGLTexture2DRef Texture = std::make_shared<OpenGLTexture2D>(this, 0, GL_TEXTURE_2D, GL_COLOR_ATTACHMENT0, InSizeX, InSizeY, 1, NumMips, InFormat, name);
        glGenTextures(1, &Texture->Resource);
        glBindTexture(Texture->Target, Texture->Resource);
        auto [Format, InternalFormat, Type] = TranslatePixelFormat(Texture->GetFormat());
        glm::uvec3 sizexyz = Texture->GetSizeXYZ();
        glTexImage2D_usingTexStorage(Texture->Target, 0, InternalFormat, sizexyz.x, sizexyz.y, NumMips, Format, Type, data);
        return Texture;
    }
    RHITexture2DArrayRef FOpenGLDynamicRHI::RHICreateTexture2DArray(
        const std::string &name, EPixelFormat InFormat, int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, void *data
    )
    {
        OpenGLTexture2DArrayRef Texture = std::make_shared<OpenGLTexture2DArray>(this, 0, GL_TEXTURE_2D_ARRAY, GL_COLOR_ATTACHMENT0, InSizeX, InSizeY, InSizeZ, NumMips, InFormat, name);
        glGenTextures(1, &Texture->Resource);
        glBindTexture(Texture->Target, Texture->Resource);
        auto [Format, InternalFormat, Type] = TranslatePixelFormat(Texture->GetFormat());
        glm::uvec3 sizexyz = Texture->GetSizeXYZ();
        glTexImage3D_usingTexStorage(Texture->Target, 0, InternalFormat, sizexyz.x, sizexyz.y, sizexyz.z, NumMips, Format, Type, data);
        return Texture;
    }
    RHITexture3DRef FOpenGLDynamicRHI::RHICreateTexture3D(
        const std::string &name, EPixelFormat InFormat, int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, void *data
    )
    {
        OpenGLTexture3DRef Texture = std::make_shared<OpenGLTexture3D>(this, 0, GL_TEXTURE_3D, GL_COLOR_ATTACHMENT0, InSizeX, InSizeY, InSizeZ, NumMips, InFormat, name);
        glGenTextures(1, &Texture->Resource);
        glBindTexture(Texture->Target, Texture->Resource);
        auto [Format, InternalFormat, Type] = TranslatePixelFormat(Texture->GetFormat());
        glm::uvec3 sizexyz = Texture->GetSizeXYZ();
        glTexImage3D_usingTexStorage(Texture->Target, 0, InternalFormat, sizexyz.x, sizexyz.y, sizexyz.z, NumMips, Format, Type, data);
        return Texture;
    }
    RHITextureCubeRef FOpenGLDynamicRHI::RHICreateTextureCube(
        const std::string &name, EPixelFormat InFormat, int32 NumMips, uint32 InSizeX, uint32 InSizeY, void *data[6]
    )
    {
        GLDEBUG();
        OpenGLTextureCubeRef Texture = std::make_shared<OpenGLTextureCube>(this, 0, GL_TEXTURE_CUBE_MAP, GL_COLOR_ATTACHMENT0, InSizeX, InSizeY, 1, NumMips, InFormat, name);
        glGenTextures(1, &Texture->Resource);
        glBindTexture(Texture->Target, Texture->Resource);
        GLDEBUG();
        auto [Format, InternalFormat, Type] = TranslatePixelFormat(Texture->GetFormat());
        glm::uvec3 sizexyz = Texture->GetSizeXYZ();
        glTexStorage2D(GL_TEXTURE_CUBE_MAP, NumMips, InternalFormat, sizexyz.x, sizexyz.y);
        if (data) 
        {
            for (int i = 0; i < 6; i++)
                glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, 0, 0, sizexyz.x, sizexyz.y, Format, Type, data[i]);
        }
        glBindTexture(Texture->Target, 0);
        return Texture;
    }

    RHIFramebufferRef FOpenGLDynamicRHI::RHICreateFramebuffer()
    {
        return std::make_shared<OpenGLFramebuffer>();
    }
    RHIFramebufferRef FOpenGLDynamicRHI::RHICreateFramebuffer(EFramebufferAttachment attachment, RHITexture2DRef texture)
    {
        return std::make_shared<OpenGLFramebuffer>(attachment, texture);
    }
    RHIFramebufferRef FOpenGLDynamicRHI::RHICreateFramebuffer(EFramebufferAttachment attachment, RHITexture2DArrayRef texture, unsigned int layer_index)
    {
        return std::make_shared<OpenGLFramebuffer>(attachment, texture, layer_index);
    }
    void FOpenGLDynamicRHI::RHIGenerateMipmap(RHITextureRef texture)
    {
        auto GLTexture = TextureResourceCast(texture.get());
        int unit_id = TexMngr.AllocUnit();
        glActiveTexture(GL_TEXTURE0 + unit_id);
        glBindTexture(GLTexture.Target, GLTexture.Resource);
        if (GLTexture.Target != GL_TEXTURE_CUBE_MAP)
        {
            glGenerateMipmap(GLTexture.Target);
        }
        TexMngr.FreeUnit(unit_id);
    }

    void FOpenGLDynamicRHI::RHIBindComputeBuffer(uint32 index, RHIBufferRef buffer)
    {
        OpenGLBufferRef GLBuffer = std::static_pointer_cast<OpenGLBuffer>(buffer);
        glBindBuffer(GLBuffer->Target, GLBuffer->Resource);
        glBindBufferBase(GLBuffer->Target, index, GLBuffer->Resource);
    }

    void FOpenGLDynamicRHI::RHIBindBufferData(RHIBufferRef buffer, unsigned int size, void *data, EBufferUsageFlags usage)
    {
        OpenGLBufferRef GLBuffer = std::static_pointer_cast<OpenGLBuffer>(buffer);
        GLenum GLUsage = GL_STATIC_DRAW;
        if (EnumHasAnyFlags(usage, EBufferUsageFlags::Dynamic))
            GLUsage = GL_DYNAMIC_DRAW;
        glBindBuffer(GLBuffer->Target, GLBuffer->Resource);
        glBufferData(GLBuffer->Target, size, data, GLUsage);
        glBindBuffer(GLBuffer->Target, 0);
    }
    void FOpenGLDynamicRHI::RHIBindFramebuffer(RHIFramebuffer *framebuffer)
    {
        OpenGLFramebuffer *GLFramebuffer = static_cast<OpenGLFramebuffer*>(framebuffer);
        if (ContextState.Framebuffer != GLFramebuffer)
        {
            ContextState.Framebuffer = GLFramebuffer;
            if (GLFramebuffer)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, GLFramebuffer->Resource);
                glDrawBuffers(GLFramebuffer->m_ColorAttachments.size(), GLFramebuffer->m_ColorAttachments.data());
            }
            else
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }

    void *FOpenGLDynamicRHI::RHIMapComputeBuffer(RHIBufferRef buffer, EDataAccessFlag access)
    {
        GLenum GLAccess;
        switch (access) {
            case EDataAccessFlag::DA_ReadOnly: GLAccess = GL_READ_ONLY; break;
            case EDataAccessFlag::DA_WriteOnly: GLAccess = GL_WRITE_ONLY; break;
            case EDataAccessFlag::DA_ReadWrite: GLAccess = GL_READ_WRITE; break;
            default: std::cout << "Invalid Data access flag" << std::endl; break;
        }
        OpenGLBufferRef GLBuffer = std::static_pointer_cast<OpenGLBuffer>(buffer);
        glBindBuffer(GLBuffer->Target, GLBuffer->Resource);
        return glMapBuffer(GLBuffer->Target, GLAccess);
    }
    void FOpenGLDynamicRHI::RHIUnmapComputeBuffer(RHIBufferRef buffer)
    {
        OpenGLBufferRef GLBuffer = std::static_pointer_cast<OpenGLBuffer>(buffer);
        glUnmapBuffer(GLBuffer->Target);
        glBindBuffer(GLBuffer->Target, 0);
    }
    unsigned char *FOpenGLDynamicRHI::RHIReadImagePixel(RHITexture2DRef texture)
    {
        OpenGLTexture2DRef GLTexture = std::static_pointer_cast<OpenGLTexture2D>(texture);
        int size = texture->GetSizeX() * texture->GetSizeY() * TranslatePixelFormatToBytePerPixel(texture->GetFormat());
        unsigned char *data = new unsigned char[size];
        glBindTexture(GLTexture->Target, GLTexture->Resource);
        auto [format, internal, type] = TranslatePixelFormat(GLTexture->GetFormat());
        glGetTexImage(GLTexture->Target, 0, format, type, data);
        return data;
    }
    void FOpenGLDynamicRHI::RHISetViewport(GLint x, GLint y, GLsizei width, GLsizei height)
    {
        glViewport(x, y, width, height);
    }
    void FOpenGLDynamicRHI::RHICopyBufferSubData(RHIBufferRef readBuffer, RHIBufferRef writeBuffer, int32 readOffset, int32 writeOffset, int32 size)
    {
        OpenGLBufferRef GLReadBuffer = std::static_pointer_cast<OpenGLBuffer>(readBuffer);
        OpenGLBufferRef GLWriteBuffer = std::static_pointer_cast<OpenGLBuffer>(writeBuffer);
        glCopyNamedBufferSubData(GLReadBuffer->Resource, GLWriteBuffer->Resource, (GLintptr)readOffset, (GLintptr)writeOffset, size);
    }
    
	void FOpenGLDynamicRHI::RHIDrawArrays(uint32 Count, int32 InstanceCount)
    {
        glDrawArraysInstanced(
            TranslatePrimitiveMode(ContextState.GraphicsPipelineState->Initializer.PrimitiveMode), 
            0, 
            Count,
            InstanceCount
        );
        EndDraw(); // RHIClearTextureUnit();
    }
    
	void FOpenGLDynamicRHI::RHIDrawIndexed(RHIBuffer *IndexBuffer, int32 InstanceCount)
    {
        OpenGLBuffer *GLIndexBuffer = static_cast<OpenGLBuffer*>(IndexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GLIndexBuffer->Resource);
            GLDEBUG();
        // if (InstanceCount > 1)
        // {
        glDrawElementsInstanced(
            TranslatePrimitiveMode(ContextState.GraphicsPipelineState->Initializer.PrimitiveMode), 
            IndexBuffer->GetCount(), 
            TranslateIndexBufferStride(IndexBuffer->GetStride()),
            0,
            InstanceCount
        );
            GLDEBUG();
        EndDraw(); // RHIClearTextureUnit();
    }

    void FOpenGLDynamicRHI::RHIDrawIndexedIndirect(RHIBuffer *IndexBuffer, RHIBuffer *IndirectBuffer, uint32 IndirectOffset)
    {
        OpenGLBuffer *GLIndexBuffer = static_cast<OpenGLBuffer*>(IndexBuffer);
        OpenGLBuffer *GLIndirectBuffer = static_cast<OpenGLBuffer*>(IndirectBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GLIndexBuffer->Resource);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, GLIndirectBuffer->Resource);
        glDrawElementsIndirect(
            TranslatePrimitiveMode(ContextState.GraphicsPipelineState->Initializer.PrimitiveMode),
            TranslateIndexBufferStride(IndexBuffer->GetStride()),
            reinterpret_cast<void*>(IndirectOffset)
        );
        EndDraw(); // RHIClearTextureUnit();
    }

    void FOpenGLDynamicRHI::RHIDispatch(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z)
    {
        glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
        TexMngr.FreeAllUnit(); // RHIClearTextureUnit();
    }
    void FOpenGLDynamicRHI::RHIDispatchIndirect(RHIBuffer *indirectArgs)
    {
        OpenGLBuffer *GLIndirectArgs = static_cast<OpenGLBuffer*>(indirectArgs);

        assert(GLIndirectArgs->Target == GL_DISPATCH_INDIRECT_BUFFER);

        glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, GLIndirectArgs->Resource);
        glDispatchComputeIndirect(0);
        TexMngr.FreeAllUnit(); // RHIClearTextureUnit();
    }
    void FOpenGLDynamicRHI::RHIImageMemoryBarrier()
    {
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
    void FOpenGLDynamicRHI::RHIStorageMemoryBarrier()
    {
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }
    void FOpenGLDynamicRHI::RHIClearBuffer(uint32 flagbits)
    {
        glClear(flagbits);
    }

    int nilou::FOpenGLDynamicRHI::Initialize()
    {
        //bool ret = Initialize();
        //if (!ret)
        //    return ret;

        bool ret = gladLoadGL();
        if (!ret)
        {
            std::cerr << "RHI load failed!" << std::endl;
        }
        else
        {
            std::cout << "RHI Version " << GLVersion.major << "." 
                    << GLVersion.minor << " loaded" << std::endl;
            GLDEBUG();
            if (GLAD_GL_VERSION_3_3)
            {
                glDisable(GL_BLEND);
                // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                //glClearDepth(1.0f);
                // glEnable(GL_DEPTH_TEST);
                glFrontFace(GL_CCW);
                // glDepthMask(true);
                // glEnable(GL_CULL_FACE);
                // glCullFace(GL_BACK);
                //glEnable(GL_PROGRAM_POINT_SIZE);
            }
            auto config = g_pApp->GetConfiguration();
            GLDEBUG();
            glViewport(0, 0, config.screenWidth, config.screenHeight);
            GLDEBUG();
            glGenVertexArrays(1, &ContextState.VertexArrayObject);
            glBindVertexArray(ContextState.VertexArrayObject);

            // axis = CoordinateAxis(10, 0.1);
            // InitializeDefaultMaterial();
            // InitializeDrawPasses();

        }
        return ret;
    }

    void nilou::FOpenGLDynamicRHI::Finalize()
    {
    }

    nilou::FOpenGLDynamicRHI::OpenGLTextureResource nilou::FOpenGLDynamicRHI::TextureResourceCast(nilou::RHITexture *texture)
    {
        OpenGLTextureResource resource;
        if (texture->GetTextureType() == ETextureType::TT_Texture2D)
        {
            OpenGLTexture2D *GLTexture = static_cast<OpenGLTexture2D*>(texture);
            resource.Target = GLTexture->Target;
            resource.Resource = GLTexture->Resource;
            auto [Format, InternalFormat, Type] = TranslatePixelFormat(texture->GetFormat());
            resource.InternalFormat = InternalFormat;
        }
        else if (texture->GetTextureType() == ETextureType::TT_Texture2DArray)
        {
            OpenGLTexture2DArray *GLTexture = static_cast<OpenGLTexture2DArray*>(texture);
            resource.Target = GLTexture->Target;
            resource.Resource = GLTexture->Resource;
            auto [Format, InternalFormat, Type] = TranslatePixelFormat(texture->GetFormat());
            resource.InternalFormat = InternalFormat;
        }
        else if (texture->GetTextureType() == ETextureType::TT_Texture3D)
        {
            OpenGLTexture3D *GLTexture = static_cast<OpenGLTexture3D*>(texture);
            resource.Target = GLTexture->Target;
            resource.Resource = GLTexture->Resource;
            auto [Format, InternalFormat, Type] = TranslatePixelFormat(texture->GetFormat());
            resource.InternalFormat = InternalFormat;
        }
        else if (texture->GetTextureType() == ETextureType::TT_TextureCube)
        {
            OpenGLTextureCube *GLTexture = static_cast<OpenGLTextureCube*>(texture);
            resource.Target = GLTexture->Target;
            resource.Resource = GLTexture->Resource;
            auto [Format, InternalFormat, Type] = TranslatePixelFormat(texture->GetFormat());
            resource.InternalFormat = InternalFormat;
            // resource.InternalFormat = map_Format_Internal[(uint8)GLTexture->GetFormat()];
        }
        return resource;
    }

    
    void FOpenGLDynamicRHI::glTexImage2D_usingTexStorage(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint nummips, GLenum format, GLenum type, const void *pixels)
    {    
        GLDEBUG();
        glTexStorage2D(target, nummips, internalformat, width, height);    
        GLDEBUG();
        if (pixels)
            glTexSubImage2D(target, 0, 0, 0, width, height, format, type, pixels);
            GLDEBUG();
    }
    void FOpenGLDynamicRHI::glTexImage3D_usingTexStorage(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint nummips, GLenum format, GLenum type, const void *pixels)
    {
        glTexStorage3D(target, nummips, internalformat, width, height, depth);
        if (pixels)
            glTexSubImage3D(target, 0, 0, 0, 0, width, height, depth, format, type, pixels);
    }

    void FOpenGLDynamicRHI::EndDraw()
    {
        TexMngr.FreeAllUnit();
        for (int i = 0; i < MAX_VERTEX_ATTRIBUTE_COUNT; i++)
        {
            if (ContextState.VertexAttributeEnabled[i] == true)
            {
                ContextState.VertexAttributeEnabled[i] = false;
                glDisableVertexAttribArray(i);
            GLDEBUG();
            }
        }
    }
}