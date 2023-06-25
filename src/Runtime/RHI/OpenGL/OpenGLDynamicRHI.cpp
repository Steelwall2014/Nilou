#include <iostream>
#include <glad/glad.h>
#include <memory>
#include <numeric>
#include <tinygltf/stb_image_write.h>
#include <tuple>
#include <vector>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/MachineIndependent/localintermediate.h>

// #include "Common/QuadTree/QuadTree.h"
#include "BaseApplication.h"
// #include "Common/SceneManager.h"
// #include "Common/SceneNode/SceneGeometryNode.h"
// #include "Common/ShaderManager.h"
#include "Common/Log.h"
#include "Common/Asset/AssetLoader.h"
// #include "Common/QuadTree/QuadTreeStructures.h"


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
#include "PipelineStateCache.h"


#ifdef NILOU_DEBUG
#include "Common/InputManager.h"
#endif // NILOU_DEBUG


using namespace std::literals;  // For the use of operator""s


/**
* State translation
*/
namespace nilou {

	// void FDynamicRHI::CreateDynamicRHI_RenderThread()
	// {
	// 	FDynamicRHI::DynamicRHI = new FOpenGLDynamicRHI;
	// }

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
            case EPrimitiveMode::PM_PointList : return GL_POINTS;
            case EPrimitiveMode::PM_LineList : return GL_LINES;
            case EPrimitiveMode::PM_TriangleList : return GL_TRIANGLES;
            case EPrimitiveMode::PM_TriangleStrip : return GL_TRIANGLE_STRIP;
            default: 
                NILOU_LOG(Error, "Unsupported primitive type {}", magic_enum::enum_name(PrimitiveMode));
                return GL_TRIANGLES;
        };
    }

    static GLenum TranslateIndexBufferStride(uint32 Stride)
    {
        switch(Stride)
        {
            case 1 : return GL_UNSIGNED_BYTE;
            case 2 : return GL_UNSIGNED_SHORT;
            case 4 : return GL_UNSIGNED_INT;
            default: NILOU_LOG(Error, "Invalid index buffer stride: {}", Stride); return GL_UNSIGNED_INT;
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
            default: NILOU_LOG(Error, "Unknown TextureFilter: {}", (int)TextureFilter) return GL_LINEAR;
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
            case EPixelFormat::PF_R8UI:         Format = GL_RED_INTEGER;InternalFormat = GL_R8UI;       Type = GL_UNSIGNED_BYTE; break;
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
            case EPixelFormat::PF_D32FS8:       Format = GL_DEPTH_STENCIL;  InternalFormat = GL_DEPTH32F_STENCIL8;  Type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV; break;

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
            default: NILOU_LOG(Error, "Unknown PixelFormat: {}", (int)PixelFormat) break;
        }

        return { Format, InternalFormat, Type };
    }

}

/**
* Texture unit
*/
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

/**
* Get error
*/
namespace nilou {
    void nilou::FOpenGLDynamicRHI::GetError(const char *file, int line)
    {
#ifdef NILOU_DEBUG
        GLenum errorCode;
        while ((errorCode = glGetError()) != GL_NO_ERROR)
        {
            std::string error_msg;
            switch (errorCode) 
            {
            case GL_INVALID_ENUM:
                error_msg = "An unacceptable value is specified for an enumerated argument.";
                break;
            case GL_INVALID_VALUE:
                error_msg = "A numeric argument is out of range.";
                break;
            case GL_INVALID_OPERATION:
                error_msg = "The specified operation is not allowed in the current state.";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error_msg = "The framebuffer object is not complete.";
                break;
            case GL_OUT_OF_MEMORY:
                error_msg = "There is not enough memory left to execute the command.";
                break;
            case GL_STACK_UNDERFLOW:
                error_msg = "An attempt has been made to perform an operation that would cause an internal stack to underflow.";
                break;
            case GL_STACK_OVERFLOW:
                error_msg = "An attempt has been made to perform an operation that would cause an internal stack to overflow.";
                break;
            }
            error_msg += " | "s + file + " ("s + std::to_string(line) + ")"s;
        }
#endif // NILOU_DEBUG
    }
}

/**
* Set state
*/
namespace nilou {

    void FOpenGLDynamicRHI::RHISetViewport(int32 Width, int32 Height)
    {
        if (Width != ContextState.ViewportWidth || Height != ContextState.ViewportHeight)
        {
            ContextState.ViewportWidth = Width;
            ContextState.ViewportHeight = Height;
            glViewport(0, 0, Width, Height);
        }
    }

    bool FOpenGLDynamicRHI::RHISetShaderUniformBuffer(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, RHIUniformBuffer *UniformBufferRHI)
    {
        return RHISetShaderUniformBuffer(
            BoundPipelineState, PipelineStage, 
            BoundPipelineState->GetBaseIndexByName(PipelineStage, ParameterName), UniformBufferRHI);
    }

    bool FOpenGLDynamicRHI::RHISetShaderUniformBuffer(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, RHIUniformBuffer *UniformBufferRHI)
    {
        if (BoundPipelineState != ContextState.GraphicsPipelineState)
        {
            NILOU_LOG(Error, "RHISetShaderUniformBuffer BoundPipelineState parameter is different from ContextState.GraphicsPipelineState");
            return false;
        }

        if (BaseIndex < 0)
        {
            NILOU_LOG(Error, "RHISetShaderUniformBuffer Invalid BaseIndex: {}", BaseIndex);
            return false;
        }
        OpenGLUniformBuffer *GLUniformBuffer = static_cast<OpenGLUniformBuffer*>(UniformBufferRHI);
        OpenGLGraphicsPipelineState *GLPipelineState = static_cast<OpenGLGraphicsPipelineState*>(BoundPipelineState);
        
        // if (ContextState.GraphicsPipelineState != GLPipelineState)
        //     RHIUseShaderProgram(GLPipelineState->Program.get());

        glBindBufferBase(GL_UNIFORM_BUFFER, BaseIndex, GLUniformBuffer->Resource);
        return true;
        // if (ContextState.GraphicsPipelineState != GLPipelineState)
        //     RHIUseShaderProgram(ContextState.GraphicsPipelineState->Program.get());
    }
    
    bool FOpenGLDynamicRHI::RHISetShaderSampler(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, const FRHISampler &SamplerRHI)
    {
        return RHISetShaderSampler(
            BoundPipelineState, PipelineStage, 
            BoundPipelineState->GetBaseIndexByName(PipelineStage, ParameterName), SamplerRHI);
    }

    bool FOpenGLDynamicRHI::RHISetShaderSampler(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, const FRHISampler &SamplerRHI)
    {
        if (BoundPipelineState != ContextState.GraphicsPipelineState)
        {
            NILOU_LOG(Error, "RHISetShaderSampler BoundPipelineState parameter is different from ContextState.GraphicsPipelineState");
            return false;
        }

        if (BaseIndex < 0)
        {
            NILOU_LOG(Error, "RHISetShaderSampler Invalid BaseIndex: {}", BaseIndex);
            return false;
        }

        {
            OpenGLTextureResource GLTexture = TextureResourceCast(SamplerRHI.Texture);
            // int unit_id = TexMngr.AllocUnit();
            glActiveTexture(GL_TEXTURE0 + BaseIndex);
            glBindTexture(GLTexture.Target, GLTexture.Resource);
            glTexParameteri(GLTexture.Target, GL_TEXTURE_MAG_FILTER, TranslateTextureFilter(SamplerRHI.Params.Mag_Filter));
            glTexParameteri(GLTexture.Target, GL_TEXTURE_MIN_FILTER, TranslateTextureFilter(SamplerRHI.Params.Min_Filter));
            glTexParameteri(GLTexture.Target, GL_TEXTURE_WRAP_S, TranslateWrapMode(SamplerRHI.Params.Wrap_S));
            glTexParameteri(GLTexture.Target, GL_TEXTURE_WRAP_T, TranslateWrapMode(SamplerRHI.Params.Wrap_T));
            if (GLTexture.Target == GL_TEXTURE_CUBE_MAP || GLTexture.Target == GL_TEXTURE_3D)
                glTexParameteri(GLTexture.Target, GL_TEXTURE_WRAP_R, TranslateWrapMode(SamplerRHI.Params.Wrap_R));
            glUniform1i(BaseIndex, BaseIndex);
        }
        return true;
    }

	bool FOpenGLDynamicRHI::RHISetShaderImage(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, RHITexture *Image, EDataAccessFlag AccessFlag)
    {
        return RHISetShaderImage(
            BoundPipelineState, PipelineStage, 
            BoundPipelineState->GetBaseIndexByName(PipelineStage, ParameterName), Image, AccessFlag);
    }

	bool FOpenGLDynamicRHI::RHISetShaderImage(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, RHITexture *Image, EDataAccessFlag AccessFlag)
    {
        if (BoundPipelineState != ContextState.GraphicsPipelineState)
        {
            NILOU_LOG(Error, "RHISetShaderImage BoundPipelineState parameter is different from ContextState.GraphicsPipelineState");
            return false;
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
        RHIGetError();
        // auto a = GLTexture->Resource;
        // if (ContextState.GraphicsPipelineState->Program != GLShader->Program)
        //     RHIUseShaderProgram(GLShader->Program.get());
        glBindImageTexture(BaseIndex, GLTexture.Resource, 0, false, 0, GLAccess, GLTexture.InternalFormat);
        // if (ContextState.GraphicsPipelineState->Program != GLShader->Program)
        //     RHIUseShaderProgram(ContextState.GraphicsPipelineState->Program.get());
        RHIGetError();
        return true;
    }
    
    // bool FOpenGLDynamicRHI::RHISetShaderUniformValue(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, int32 Value)
    // {
    //     return RHISetShaderUniformValue(
    //         BoundPipelineState, PipelineStage, 
    //         BoundPipelineState->GetBaseIndexByName(PipelineStage, ParameterName), Value);
    // }

    // bool FOpenGLDynamicRHI::RHISetShaderUniformValue(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, int32 Value)
    // {
    //     if (BoundPipelineState != ContextState.GraphicsPipelineState)
    //     {
    //         NILOU_LOG(Error, "RHISetShaderSampler BoundPipelineState parameter is different from ContextState.GraphicsPipelineState");
    //         return false;
    //     }
    //     glUniform1i(BaseIndex, Value);
    //     return true;
    // }

    // bool FOpenGLDynamicRHI::RHISetShaderUniformValue(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, float Value)
    // {
    //     return RHISetShaderUniformValue(
    //         BoundPipelineState, PipelineStage, 
    //         BoundPipelineState->GetBaseIndexByName(PipelineStage, ParameterName), Value);
    // }

    // bool FOpenGLDynamicRHI::RHISetShaderUniformValue(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, float Value)
    // {
    //     if (BoundPipelineState != ContextState.GraphicsPipelineState)
    //     {
    //         NILOU_LOG(Error, "RHISetShaderSampler BoundPipelineState parameter is different from ContextState.GraphicsPipelineState");
    //         return false;
    //     }
    //     glUniform1f(BaseIndex, Value);
    //     return true;
    // }

    // bool FOpenGLDynamicRHI::RHISetShaderUniformValue(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, uint32 Value)
    // {
    //     return RHISetShaderUniformValue(
    //         BoundPipelineState, PipelineStage, 
    //         BoundPipelineState->GetBaseIndexByName(PipelineStage, ParameterName), Value);
    // }

    // bool FOpenGLDynamicRHI::RHISetShaderUniformValue(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, uint32 Value)
    // {
    //     if (BoundPipelineState != ContextState.GraphicsPipelineState)
    //     {
    //         NILOU_LOG(Error, "RHISetShaderSampler BoundPipelineState parameter is different from ContextState.GraphicsPipelineState");
    //         return false;
    //     }
    //     glUniform1ui(BaseIndex, Value);
    //     return true;
    // }


	void FOpenGLDynamicRHI::RHISetVertexBuffer(const FRHIVertexInput *VertexInput)
    {
        OpenGLBuffer *GLBuffer = static_cast<OpenGLBuffer*>(VertexInput->VertexBuffer);

        auto [DataType, Size, bNormalized, bShouldConvertToFloat] = TranslateVertexElementType(VertexInput->Type);

        glBindBuffer(GL_ARRAY_BUFFER, GLBuffer->Resource);
        glVertexAttribPointer(VertexInput->Location, Size, DataType, bNormalized, GLBuffer->GetStride(), (void*)0);
        glEnableVertexAttribArray(VertexInput->Location);
        ContextState.VertexAttributeEnabled[VertexInput->Location] = true;
    }

    void FOpenGLDynamicRHI::RHISetRasterizerState(RHIRasterizerState *newState)
    {
        if (!newState)
            return;
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
        if (!newState)
            return;
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

        // If only two-sided <-> one-sided stencil mode changes, and nothing else, we need to call full set of functions
        // to ensure all drivers handle this correctly - some of them might keep those states in different variables.
        if (ContextState.DepthStencilState.bTwoSidedStencilMode != DepthStencilState->bTwoSidedStencilMode)
        {
            // Invalidate cache to enforce update of part of stencil state that needs to be set with different functions, when needed next
            // Values below are all invalid, but they'll never be used, only compared to new values to be set.
            ContextState.DepthStencilState.StencilFunc = 0xFFFF;
            ContextState.DepthStencilState.StencilFail = 0xFFFF;
            ContextState.DepthStencilState.StencilZFail = 0xFFFF;
            ContextState.DepthStencilState.StencilPass = 0xFFFF;
            ContextState.DepthStencilState.BackStencilFunc = 0xFFFF;
            ContextState.DepthStencilState.BackStencilFail = 0xFFFF;
            ContextState.DepthStencilState.BackStencilZFail = 0xFFFF;
            ContextState.DepthStencilState.BackStencilPass = 0xFFFF;
            ContextState.DepthStencilState.StencilReadMask = 0xFFFF;

            ContextState.DepthStencilState.bTwoSidedStencilMode = DepthStencilState->bTwoSidedStencilMode;
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
        if (!newState)
            return;
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
        // FGraphicsPipelineStateInitializer &Initializer = GLState->Initializer;
        RHIUseShaderProgram(GLState->Program.get());
        ContextState.GraphicsPipelineState = GLState;
        RHISetDepthStencilState(NewState->Initializer.DepthStencilState);
        RHISetRasterizerState(NewState->Initializer.RasterizerState);
        RHISetBlendState(NewState->Initializer.BlendState);
        if (NewState->Initializer.VertexInputList)
            for (auto& VertexInput : *NewState->Initializer.VertexInputList)
                RHISetVertexBuffer(&VertexInput);
    }

	FRHIGraphicsPipelineState *FOpenGLDynamicRHI::RHISetComputeShader(RHIComputeShader *ComputeShader)
    {
        RHIGetError();
        FGraphicsPipelineStateInitializer Initializer;
        Initializer.ComputeShader = ComputeShader;
        FRHIGraphicsPipelineState *PSO = RHIGetOrCreatePipelineStateObject(Initializer);
        RHISetGraphicsPipelineState(PSO);
        RHIGetError();
        return PSO;
    }
}

/**
* Binding buffers
*/
namespace nilou {

    void FOpenGLDynamicRHI::RHIBindComputeBuffer(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, RHIBuffer* buffer)
    {
        RHIBindComputeBuffer(
            BoundPipelineState, PipelineStage, 
            BoundPipelineState->GetBaseIndexByName(PipelineStage, ParameterName), buffer);
    }

    void FOpenGLDynamicRHI::RHIBindComputeBuffer(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, RHIBuffer* buffer)
    {
        if (BoundPipelineState != ContextState.GraphicsPipelineState)
        {
            NILOU_LOG(Error, "RHIBindComputeBuffer BoundPipelineState parameter is different from ContextState.GraphicsPipelineState");
            return;
        }
        OpenGLBuffer* GLBuffer = static_cast<OpenGLBuffer*>(buffer);
        glBindBuffer(GLBuffer->Target, GLBuffer->Resource);
        glBindBufferBase(GLBuffer->Target, BaseIndex, GLBuffer->Resource);
    }

    void FOpenGLDynamicRHI::RHIBindBufferData(RHIBuffer* buffer, unsigned int size, void *data, EBufferUsageFlags usage)
    {
        OpenGLBuffer* GLBuffer = static_cast<OpenGLBuffer*>(buffer);
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
}

/**
* Create/Update data
*/
namespace nilou {

	// FRHIGraphicsPipelineState *FOpenGLDynamicRHI::RHIGetBoundPipelineState()
    // {
    //     return ContextState.GraphicsPipelineState;
    // }

    // EShaderParameterType TranslateToShaderParameterType(const glslang::TType *Type)
    // {
    //     EShaderParameterType type = EShaderParameterType::SPT_None;
    //     glslang::TBasicType BasicType = Type->getBasicType();
    //     switch (BasicType)
    //     {
    //     case glslang::TBasicType::EbtBlock:
    //         type = EShaderParameterType::SPT_UniformBuffer;
    //         break;
    //     case glslang::TBasicType::EbtSampler:
    //         type = EShaderParameterType::SPT_Sampler;
    //         break;
    //     case glslang::TBasicType::EbtAtomicUint:
    //         type = EShaderParameterType::SPT_AtomicUint;
    //         break;
    //     case glslang::TBasicType::EbtUint:
    //         type = EShaderParameterType::SPT_Uint;
    //         break;
    //     case glslang::TBasicType::EbtFloat:
    //         type = EShaderParameterType::SPT_Float;
    //         break;
    //     case glslang::TBasicType::EbtInt:
    //         type = EShaderParameterType::SPT_Int;
    //         break;
    //     }  
    //     if (Type->isImage())
    //         type = EShaderParameterType::SPT_Image;
        
    //     return type;
    // }

    // void AllocateParameterBindingPoint(FRHIPipelineLayout &PipelineLayout, int PipelineResource, glslang::TProgram &ProgramGlsl/*, const std::set<FShaderParameterInfo> &ParsedParameters, *//*EPipelineStage PipelineStage*/)
    // {
    //     FRHIDescriptorSet &VertexDescriptorSet = PipelineLayout.DescriptorSets[EPipelineStage::PS_Vertex];
    //     FRHIDescriptorSet &PixelDescriptorSet = PipelineLayout.DescriptorSets[EPipelineStage::PS_Pixel];
    //     FRHIDescriptorSet &ComputeDescriptorSet = PipelineLayout.DescriptorSets[EPipelineStage::PS_Compute];

        
    //     int Buffer_block_num = ProgramGlsl.getNumBufferBlocks();
    //     if (Buffer_block_num > 0)
    //         NILOU_LOG(Warning, "")

            
    //     int max_sampler_binding_point = -1;
    //     int NumUniformVariable = ProgramGlsl.getNumUniformVariables();
    //     for (int i = 0; i < NumUniformVariable; i++)
    //     {
    //         const glslang::TObjectReflection &refl = ProgramGlsl.getUniform(i);
    //         FRHIDescriptorSetLayoutBinding binding;
    //         binding.Name = refl.name;
    //         binding.ParameterType = TranslateToShaderParameterType(refl.getType());
    //         binding.BindingPoint = refl.getBinding();
    //         if (binding.ParameterType == EShaderParameterType::SPT_Image)
    //         {
    //             if (binding.BindingPoint == -1)
    //             {
    //                 NILOU_LOG(Error, "image1/2/3D variables must have an explicit binding point");
    //                 continue;
    //             }
    //         }
    //         else if (binding.ParameterType == EShaderParameterType::SPT_Sampler)
    //         {
    //             binding.BindingPoint = glGetUniformLocation(PipelineResource, binding.Name.c_str());
    //             if (binding.BindingPoint == -1)
    //             {
    //                 NILOU_LOG(Warning, "Shader parameter {} is omitted in glsl", binding.Name)
    //                 continue;
    //             }
    //             max_sampler_binding_point = std::max(max_sampler_binding_point, binding.BindingPoint);
    //         }
    //         else if (binding.ParameterType == EShaderParameterType::SPT_AtomicUint)
    //         {
    //             if (binding.BindingPoint == -1)
    //             {
    //                 NILOU_LOG(Error, "Atomic uint variables must have an explicit binding point");
    //                 continue;
    //             }
    //         }
    //         else if (binding.ParameterType == EShaderParameterType::SPT_Float || 
    //                  binding.ParameterType == EShaderParameterType::SPT_Int || 
    //                  binding.ParameterType == EShaderParameterType::SPT_Uint)
    //         {
    //             binding.BindingPoint = glGetUniformLocation(PipelineResource, binding.Name.c_str());
    //             if (binding.BindingPoint == -1)
    //             {
    //                 continue;
    //             }
    //         }
    //         else 
    //         {
    //             continue;
    //         }

    //         if (refl.stages & EShLangVertexMask)
    //         {
    //             VertexDescriptorSet.Bindings[binding.Name] = binding;
    //         }
    //         if (refl.stages & EShLangFragmentMask)
    //         {
    //             PixelDescriptorSet.Bindings[binding.Name] = binding;
    //         }
    //         if (refl.stages & EShLangComputeMask)
    //         {
    //             ComputeDescriptorSet.Bindings[binding.Name] = binding;
    //         }
    //     }

    //     int NumUniformBlock = ProgramGlsl.getNumUniformBlocks();
    //     for (int i = 0; i < NumUniformBlock; i++)
    //     {
    //         const glslang::TObjectReflection &refl = ProgramGlsl.getUniformBlock(i);
    //         FRHIDescriptorSetLayoutBinding binding;
    //         binding.Name = refl.name;
    //         binding.ParameterType = TranslateToShaderParameterType(refl.getType());
    //         binding.BindingPoint = refl.getBinding();
    //         if (binding.BindingPoint == -1)
    //         {
    //             if (binding.ParameterType == EShaderParameterType::SPT_UniformBuffer)
    //             {
    //                 int block_index = glGetUniformBlockIndex(PipelineResource, binding.Name.c_str());
    //                 if (block_index == -1)
    //                 {
    //                     NILOU_LOG(Warning, "Shader parameter {} is omitted in glsl", binding.Name)
    //                     continue;
    //                 }
    //                 glUniformBlockBinding(PipelineResource, block_index, ++max_sampler_binding_point);
    //                 binding.BindingPoint = max_sampler_binding_point;
    //             }
    //             else if (binding.ParameterType == EShaderParameterType::SPT_AtomicUint)
    //             {
    //                 NILOU_LOG(Error, "Atomic uint variables must have an explicit binding point");
    //                 continue;
    //             }
    //         }

    //         if (refl.stages & EShLangVertexMask)
    //         {
    //             VertexDescriptorSet.Bindings[binding.Name] = binding;
    //         }
    //         if (refl.stages & EShLangFragmentMask)
    //         {
    //             PixelDescriptorSet.Bindings[binding.Name] = binding;
    //         }
    //         if (refl.stages & EShLangComputeMask)
    //         {
    //             ComputeDescriptorSet.Bindings[binding.Name] = binding;
    //         }
    //     }
        
    // }

    FRHIGraphicsPipelineState *FOpenGLDynamicRHI::RHIGetOrCreatePipelineStateObject(const FGraphicsPipelineStateInitializer &Initializer)
    {
        RHIGetError();
        FRHIGraphicsPipelineState* OutPSO = FPipelineStateCache::FindCachedGraphicsPSO(Initializer);
        if (OutPSO)
            return OutPSO;

        OpenGLGraphicsPipelineStateRef PSO = std::make_shared<OpenGLGraphicsPipelineState>();
        PSO->Initializer = Initializer;
        PSO->PipelineLayout = std::make_shared<FRHIPipelineLayout>();
        RHIGetError();
        if (Initializer.ComputeShader != nullptr)
        {
            OpenGLComputeShader *comp = static_cast<OpenGLComputeShader *>(Initializer.ComputeShader);
            PSO->Program = RHICreateLinkedProgram(comp);
        }
        else if (
            Initializer.VertexShader != nullptr && 
            Initializer.PixelShader != nullptr)
        {
            OpenGLVertexShader *vert = static_cast<OpenGLVertexShader *>(Initializer.VertexShader);
            OpenGLPixelShader *frag = static_cast<OpenGLPixelShader *>(Initializer.PixelShader);

            PSO->Program = RHICreateLinkedProgram(vert, frag);
        }
        AllocateParameterBindingPoint(PSO->PipelineLayout.get(), Initializer);
        for (int PipelineStage = 0; PipelineStage < EPipelineStage::PipelineStageNum; PipelineStage++)
        {
            for (auto& [Name, Binding] : PSO->PipelineLayout->DescriptorSets[PipelineStage].Bindings)
            {
                if (Binding.ParameterType == EShaderParameterType::SPT_Sampler)
                {
                    Binding.BindingPoint = glGetUniformLocation(PSO->Program->Resource, Name.c_str());
                }
            }
        }

        FPipelineStateCache::CacheGraphicsPSO(Initializer, PSO);

        RHIGetError();
        return PSO.get();
    }

    RHIVertexShaderRef FOpenGLDynamicRHI::RHICreateVertexShader(const std::string& code)
    {
        nilou::OpenGLVertexShaderRef vert = std::make_shared<nilou::OpenGLVertexShader>(code.c_str());

        if (!vert->Success())
        {
            NILOU_LOG(Info, "{}", code)
            return nullptr;
        }

        return vert;
    }

    RHIPixelShaderRef FOpenGLDynamicRHI::RHICreatePixelShader(const std::string& code)
    {
        nilou::OpenGLPixelShaderRef pixel = std::make_shared<nilou::OpenGLPixelShader>(code.c_str());

        if (!pixel->Success())
        {
            NILOU_LOG(Info, "{}", code)
            return nullptr;
        }

        return pixel;
    }

    RHIComputeShaderRef FOpenGLDynamicRHI::RHICreateComputeShader(const std::string& code)
    {
        nilou::OpenGLComputeShaderRef comp = std::make_shared<nilou::OpenGLComputeShader>(code.c_str());

        if (!comp->Success())
        {
            NILOU_LOG(Info, "{}", code)
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

    RHIBufferRef FOpenGLDynamicRHI::RHICreateShaderStorageBuffer(unsigned int DataByteLength, void *Data)
    {
        return RHICreateBuffer(DataByteLength, DataByteLength, EBufferUsageFlags::StructuredBuffer | EBufferUsageFlags::Dynamic, Data);
    }
    RHIBufferRef FOpenGLDynamicRHI::RHICreateDispatchIndirectBuffer(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z)
    {
        DispatchIndirectCommand command{ num_groups_x, num_groups_y, num_groups_z };
        return RHICreateBuffer(sizeof(command), sizeof(command), EBufferUsageFlags::DispatchIndirect | EBufferUsageFlags::Dynamic, &command);
    }
    RHIBufferRef FOpenGLDynamicRHI::RHICreateDrawElementsIndirectBuffer(
        int32	Count,
        uint32 	instanceCount,
        uint32 	firstIndex,
        uint32 	baseVertex,
        uint32 	baseInstance)
    {
        DrawElementsIndirectCommand command{ Count, instanceCount, firstIndex, baseVertex, baseInstance };
        return RHICreateBuffer(sizeof(command), sizeof(command), EBufferUsageFlags::DrawIndirect | EBufferUsageFlags::Dynamic, &command);
    }

    RHITexture2DRef FOpenGLDynamicRHI::RHICreateTexture2D(
        const std::string &name, EPixelFormat InFormat, 
        int32 NumMips, uint32 InSizeX, uint32 InSizeY)
    {
        OpenGLTexture2DRef Texture = std::make_shared<OpenGLTexture2D>(0, GL_TEXTURE_2D, InSizeX, InSizeY, 1, NumMips, InFormat, name);
        glGenTextures(1, &Texture->Resource);
        glBindTexture(Texture->Target, Texture->Resource);
        // Texture->Handle = glGetTextureHandleARB(Texture->Resource);
        // glMakeTextureHandleResidentARB(Texture->Handle);
        auto [Format, InternalFormat, Type] = TranslatePixelFormat(Texture->GetFormat());
        uvec3 sizexyz = Texture->GetSizeXYZ();
        glTexStorage2D(Texture->Target, NumMips, InternalFormat, sizexyz.x, sizexyz.y);
        // glTexImage2D_usingTexStorage(Texture->Target, 0, InternalFormat, sizexyz.x, sizexyz.y, NumMips, Format, Type, data);
        // if (NumMips > 1)
        //     RHIGenerateMipmap(Texture);
        return Texture;
    }
    RHITexture2DArrayRef FOpenGLDynamicRHI::RHICreateTexture2DArray(
        const std::string &name, EPixelFormat InFormat, int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ
    )
    {
        OpenGLTexture2DArrayRef Texture = std::make_shared<OpenGLTexture2DArray>(0, GL_TEXTURE_2D_ARRAY, InSizeX, InSizeY, InSizeZ, NumMips, InFormat, name);
        glGenTextures(1, &Texture->Resource);
        glBindTexture(Texture->Target, Texture->Resource);
        // Texture->Handle = glGetTextureHandleARB(Texture->Resource);
        // glMakeTextureHandleResidentARB(Texture->Handle);
        auto [Format, InternalFormat, Type] = TranslatePixelFormat(Texture->GetFormat());
        uvec3 sizexyz = Texture->GetSizeXYZ();
        glTexStorage3D(Texture->Target, NumMips, InternalFormat, sizexyz.x, sizexyz.y, sizexyz.z);
        // glTexImage3D_usingTexStorage(Texture->Target, 0, InternalFormat, sizexyz.x, sizexyz.y, sizexyz.z, NumMips, Format, Type, data);
        // if (NumMips > 1)
        //     RHIGenerateMipmap(Texture);
        return Texture;
    }
    RHITexture3DRef FOpenGLDynamicRHI::RHICreateTexture3D(
        const std::string &name, EPixelFormat InFormat, int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ
    )
    {
        OpenGLTexture3DRef Texture = std::make_shared<OpenGLTexture3D>(0, GL_TEXTURE_3D, InSizeX, InSizeY, InSizeZ, NumMips, InFormat, name);
        glGenTextures(1, &Texture->Resource);
        glBindTexture(Texture->Target, Texture->Resource);
        // Texture->Handle = glGetTextureHandleARB(Texture->Resource);
        // glMakeTextureHandleResidentARB(Texture->Handle);
        auto [Format, InternalFormat, Type] = TranslatePixelFormat(Texture->GetFormat());
        uvec3 sizexyz = Texture->GetSizeXYZ();
        glTexStorage3D(Texture->Target, NumMips, InternalFormat, sizexyz.x, sizexyz.y, sizexyz.z);
        // glTexImage3D_usingTexStorage(Texture->Target, 0, InternalFormat, sizexyz.x, sizexyz.y, sizexyz.z, NumMips, Format, Type, data);
        // if (NumMips > 1)
        //     RHIGenerateMipmap(Texture);
        return Texture;
    }
    RHITextureCubeRef FOpenGLDynamicRHI::RHICreateTextureCube(
        const std::string &name, EPixelFormat InFormat, int32 NumMips, uint32 InSizeX, uint32 InSizeY
    )
    {
        RHIGetError();
        OpenGLTextureCubeRef Texture = std::make_shared<OpenGLTextureCube>(0, GL_TEXTURE_CUBE_MAP, InSizeX, InSizeY, 1, NumMips, InFormat, name);
        glGenTextures(1, &Texture->Resource);
        glBindTexture(Texture->Target, Texture->Resource);
        // Texture->Handle = glGetTextureHandleARB(Texture->Resource);
        // glMakeTextureHandleResidentARB(Texture->Handle);
        RHIGetError();
        auto [Format, InternalFormat, Type] = TranslatePixelFormat(Texture->GetFormat());
        uvec3 sizexyz = Texture->GetSizeXYZ();
        glTexStorage2D(GL_TEXTURE_CUBE_MAP, NumMips, InternalFormat, sizexyz.x, sizexyz.y);
        // if (data) 
        // {
        //     for (int i = 0; i < 6; i++)
        //         glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, 0, 0, sizexyz.x, sizexyz.y, Format, Type, data[i]);
        // }
        // glBindTexture(Texture->Target, 0);
        return Texture;
    }
    RHITexture2DRef FOpenGLDynamicRHI::RHICreateSparseTexture2D(
        const std::string &name, EPixelFormat InFormat, int32 NumMips, uint32 InSizeX, uint32 InSizeY
    )
    {
        OpenGLTexture2DRef Texture = std::make_shared<OpenGLTexture2D>(0, GL_TEXTURE_2D, InSizeX, InSizeY, 1, NumMips, InFormat, name);
        glGenTextures(1, &Texture->Resource);
        // glActiveTexture(GL_TEXTURE0);
        glBindTexture(Texture->Target, Texture->Resource);
        // Texture->Handle = glGetTextureHandleARB(Texture->Resource);
        // glMakeTextureHandleResidentARB(Texture->Handle);
        auto [Format, InternalFormat, Type] = TranslatePixelFormat(Texture->GetFormat());
        uvec3 sizexyz = Texture->GetSizeXYZ();
		glTexParameteri(Texture->Target, GL_TEXTURE_SPARSE_ARB, GL_TRUE);
        glTexStorage2D(Texture->Target, NumMips, InternalFormat, sizexyz.x, sizexyz.y);
        

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

    void FOpenGLDynamicRHI::RHIUpdateUniformBuffer(RHIUniformBufferRef UniformBuffer, void *Data)
    {
        OpenGLUniformBufferRef GLBuffer = std::static_pointer_cast<OpenGLUniformBuffer>(UniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, GLBuffer->Resource);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, GLBuffer->GetSize(), Data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void FOpenGLDynamicRHI::RHIUpdateBuffer(RHIBuffer* Buffer, uint32 Offset, uint32 Size, void *Data)
    {
        OpenGLBuffer* GLBuffer = static_cast<OpenGLBuffer*>(Buffer);
        glBindBuffer(GLBuffer->Target, GLBuffer->Resource);
        glBufferSubData(GLBuffer->Target, 0, Size, Data);
        glBindBuffer(GLBuffer->Target, 0);
    }

    void FOpenGLDynamicRHI::RHIGenerateMipmap(RHITextureRef texture)
    {
        auto GLTexture = TextureResourceCast(texture.get());
        // int unit_id = TexMngr.AllocUnit();
        // glActiveTexture(GL_TEXTURE0 + unit_id);
        glBindTexture(GLTexture.Target, GLTexture.Resource);
        glGenerateMipmap(GLTexture.Target);
        // TexMngr.FreeUnit(unit_id);
    }

    RHITexture2DRef FOpenGLDynamicRHI::RHICreateTextureView2D(
        RHITexture* OriginTexture, EPixelFormat InFormat, uint32 MinMipLevel, uint32 NumMipLevels, uint32 LevelIndex)
    {
        auto GLTexture = TextureResourceCast(OriginTexture);
        uvec2 size = uvec2(OriginTexture->GetSizeXYZ()) / uvec2(glm::pow(2, MinMipLevel));
        OpenGLTexture2DRef OutTexture = std::make_shared<OpenGLTexture2D>(
            0, GL_TEXTURE_2D, 
            size.x, size.y, 1, 
            NumMipLevels, InFormat, OriginTexture->GetName() + "_View");
        glGenTextures(1, &OutTexture->Resource);
        glTextureView(OutTexture->Resource, GL_TEXTURE_2D, 
            GLTexture.Resource, GLTexture.InternalFormat, 
            MinMipLevel, NumMipLevels, LevelIndex, 1);
        return OutTexture;
    }

    RHITextureCubeRef FOpenGLDynamicRHI::RHICreateTextureViewCube(
        RHITexture* OriginTexture, EPixelFormat InFormat, uint32 MinMipLevel, uint32 NumMipLevels)
    {
        auto GLTexture = TextureResourceCast(OriginTexture);
        uvec2 size = uvec2(OriginTexture->GetSizeXYZ()) / uvec2(glm::pow(2, MinMipLevel));
        OpenGLTextureCubeRef OutTexture = std::make_shared<OpenGLTextureCube>(
            0, GL_TEXTURE_CUBE_MAP, 
            size.x, size.y, 1, 
            NumMipLevels, InFormat, OriginTexture->GetName() + "_View");
        glGenTextures(1, &OutTexture->Resource);
        glTextureView(OutTexture->Resource, GL_TEXTURE_CUBE_MAP, 
            GLTexture.Resource, GLTexture.InternalFormat, 
            MinMipLevel, NumMipLevels, 0, 6);
        return OutTexture;
    }

    void FOpenGLDynamicRHI::RHIUpdateTexture2D(RHITexture2D* Texture, 
        int32 Xoffset, int32 Yoffset, 
        int32 Width, int32 Height, 
        int32 MipmapLevel, void* Data)
    {
        if (Data == nullptr)
            return;
        OpenGLTexture2D* GLTexture = static_cast<OpenGLTexture2D*>(Texture);
        auto [Format, InternalFormat, Type] = TranslatePixelFormat(GLTexture->GetFormat());
        
        glBindTexture(GLTexture->Target, GLTexture->Resource);
        glTexSubImage2D(GLTexture->Target, MipmapLevel,
					Xoffset, Yoffset,
					Width, Height,
					Format, Type,
					Data);
    }

    void FOpenGLDynamicRHI::RHIUpdateTexture3D(RHITexture3D* Texture, 
        int32 Xoffset, int32 Yoffset, int32 Zoffset,
        int32 Width, int32 Height, int32 Depth, 
        int32 MipmapLevel, void* Data)
    {
        if (Data == nullptr)
            return;
        OpenGLTexture3D* GLTexture = static_cast<OpenGLTexture3D*>(Texture);
        auto [Format, InternalFormat, Type] = TranslatePixelFormat(GLTexture->GetFormat());
        glBindTexture(GLTexture->Target, GLTexture->Resource);
        glTexSubImage3D(GLTexture->Target, MipmapLevel,
					Xoffset, Yoffset, Zoffset,
					Width, Height, Depth,
					Format, Type,
					Data);
    }

    void FOpenGLDynamicRHI::RHIUpdateTexture2DArray(RHITexture2DArray* Texture, 
        int32 Xoffset, int32 Yoffset, int32 LayerIndex,
        int32 Width, int32 Height,
        int32 MipmapLevel, void* Data)
    {
        if (Data == nullptr)
            return;
        OpenGLTexture2DArray* GLTexture = static_cast<OpenGLTexture2DArray*>(Texture);
        auto [Format, InternalFormat, Type] = TranslatePixelFormat(GLTexture->GetFormat());

        glBindTexture(GLTexture->Target, GLTexture->Resource);
        glTexSubImage3D(GLTexture->Target, MipmapLevel,
					Xoffset, Yoffset, LayerIndex,
					Width, Height, 1,
					Format, Type,
					Data);
    }

    void FOpenGLDynamicRHI::RHIUpdateTextureCube(RHITextureCube* Texture, 
        int32 Xoffset, int32 Yoffset, int32 LayerIndex,
        int32 Width, int32 Height,
        int32 MipmapLevel, void* Data)
    {
        if (Data == nullptr)
            return;
        OpenGLTextureCube* GLTexture = static_cast<OpenGLTextureCube*>(Texture);
        auto [Format, InternalFormat, Type] = TranslatePixelFormat(GLTexture->GetFormat());

        glBindTexture(GLTexture->Target, GLTexture->Resource);
        glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+LayerIndex, MipmapLevel,
					Xoffset, Yoffset,
					Width, Height,
					Format, Type,
					Data);
    }

}

/**
* Render pass
*/
namespace nilou {

	void FOpenGLDynamicRHI::RHIBeginRenderPass(const FRHIRenderPassInfo &InInfo)
    {
        RHIBindFramebuffer(InInfo.Framebuffer);
        RHISetViewport(InInfo.Viewport.x, InInfo.Viewport.y);

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
    
	void FOpenGLDynamicRHI::RHIDrawArrays(uint32 First, uint32 Count, int32 InstanceCount)
    {
        glDrawArraysInstanced(
            TranslatePrimitiveMode(ContextState.GraphicsPipelineState->Initializer.PrimitiveMode), 
            First, 
            Count,
            InstanceCount
        );
    }
    
	void FOpenGLDynamicRHI::RHIDrawIndexed(RHIBuffer *IndexBuffer, int32 InstanceCount)
    {
        OpenGLBuffer *GLIndexBuffer = static_cast<OpenGLBuffer*>(IndexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GLIndexBuffer->Resource);
        RHIGetError();
        glDrawElementsInstanced(
            TranslatePrimitiveMode(ContextState.GraphicsPipelineState->Initializer.PrimitiveMode), 
            IndexBuffer->GetCount(), 
            TranslateIndexBufferStride(IndexBuffer->GetStride()),
            0,
            InstanceCount
        );
        RHIGetError();
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
    }

    void FOpenGLDynamicRHI::RHIDispatch(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z)
    {
        glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
    }

    void FOpenGLDynamicRHI::RHIDispatchIndirect(RHIBuffer *indirectArgs)
    {
        OpenGLBuffer *GLIndirectArgs = static_cast<OpenGLBuffer*>(indirectArgs);

        assert(GLIndirectArgs->Target == GL_DISPATCH_INDIRECT_BUFFER);

        glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, GLIndirectArgs->Resource);
        glDispatchComputeIndirect(0);
    }

    void FOpenGLDynamicRHI::RHIEndRenderPass()
    {
    }

    void FOpenGLDynamicRHI::EndDraw()
    {
        // TexMngr.FreeAllUnit();
        for (int i = 0; i < MAX_VERTEX_ATTRIBUTE_COUNT; i++)
        {
            if (ContextState.VertexAttributeEnabled[i] == true)
            {
                ContextState.VertexAttributeEnabled[i] = false;
                glDisableVertexAttribArray(i);
                RHIGetError();
            }
        }
    }

}

/**
* Utils
*/
namespace nilou {

    FRHIRenderQueryRef FOpenGLDynamicRHI::RHICreateRenderQuery()
    {
        FOpenGLRenderQueryRef GLQuery = std::make_shared<FOpenGLRenderQuery>();
        return GLQuery;
    }

    void FOpenGLDynamicRHI::RHIBeginRenderQuery(FRHIRenderQuery *RenderQuery)
    {
        FOpenGLRenderQuery *Query = static_cast<FOpenGLRenderQuery*>(RenderQuery);
        glBeginQuery(GL_SAMPLES_PASSED, Query->Resource);
    }

    void FOpenGLDynamicRHI::RHIEndRenderQuery(FRHIRenderQuery *RenderQuery)
    {
        FOpenGLRenderQuery *Query = static_cast<FOpenGLRenderQuery*>(RenderQuery);
        glEndQuery(GL_SAMPLES_PASSED);
    }

    void FOpenGLDynamicRHI::RHIGetRenderQueryResult(FRHIRenderQuery *RenderQuery)
    {
        FOpenGLRenderQuery *Query = static_cast<FOpenGLRenderQuery*>(RenderQuery);
        glGetQueryObjectui64v(Query->Resource, GL_QUERY_RESULT, &Query->Result);
    }

    void FOpenGLDynamicRHI::RHIUseShaderProgram(OpenGLLinkedProgram *program)
    {
        OpenGLLinkedProgram *GLProgram = static_cast<OpenGLLinkedProgram*>(program);
        glUseProgram(GLProgram->Resource);
    }

    void *FOpenGLDynamicRHI::RHILockBuffer(RHIBufferRef buffer, EResourceLockMode LockMode)
    {
        GLenum GLAccess;
        switch (LockMode) {
            case EResourceLockMode::RLM_ReadOnly: GLAccess = GL_READ_ONLY; break;
            case EResourceLockMode::RLM_WriteOnly: GLAccess = GL_WRITE_ONLY; break;
            default: std::cout << "Invalid Data access flag" << std::endl; break;
        }
        OpenGLBufferRef GLBuffer = std::static_pointer_cast<OpenGLBuffer>(buffer);
        glBindBuffer(GLBuffer->Target, GLBuffer->Resource);
        return glMapBuffer(GLBuffer->Target, GLAccess);
    }

    void FOpenGLDynamicRHI::RHIUnlockBuffer(RHIBufferRef buffer)
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

    void FOpenGLDynamicRHI::RHICopyBufferSubData(RHIBufferRef readBuffer, RHIBufferRef writeBuffer, int32 readOffset, int32 writeOffset, int32 size)
    {
        OpenGLBufferRef GLReadBuffer = std::static_pointer_cast<OpenGLBuffer>(readBuffer);
        OpenGLBufferRef GLWriteBuffer = std::static_pointer_cast<OpenGLBuffer>(writeBuffer);
        glCopyNamedBufferSubData(GLReadBuffer->Resource, GLWriteBuffer->Resource, (GLintptr)readOffset, (GLintptr)writeOffset, size);
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

	void FOpenGLDynamicRHI::RHISparseTextureUnloadTile(RHITexture* Texture, uint32 TileX, uint32 TileY, uint32 MipmapLevel)
    {
        auto [Format, InternalFormat, Type] = TranslatePixelFormat(Texture->GetFormat());
        OpenGLTextureResource GLResource = TextureResourceCast(Texture);
		ivec3 PageSize = FDynamicRHI::RHIGetSparseTexturePageSize(Texture->GetTextureType(), Texture->GetFormat());
  
		glBindTexture(GLResource.Target, GLResource.Resource);
        glTexPageCommitmentARB(GLResource.Target, MipmapLevel,
					PageSize.x * TileX, PageSize.y * TileY, 0,
					PageSize.x, PageSize.y, 1,
					GL_FALSE);
    }

	void FOpenGLDynamicRHI::RHISparseTextureUpdateTile(RHITexture* Texture, uint32 TileX, uint32 TileY, uint32 MipmapLevel, void* Data)
    {
        RHIGetError();
        auto [Format, InternalFormat, Type] = TranslatePixelFormat(Texture->GetFormat());
        OpenGLTextureResource GLResource = TextureResourceCast(Texture);
		ivec3 PageSize = FDynamicRHI::RHIGetSparseTexturePageSize(Texture->GetTextureType(), Texture->GetFormat());
		glBindTexture(GLResource.Target, GLResource.Resource);
        glTexPageCommitmentARB(GLResource.Target, MipmapLevel,
					PageSize.x * TileX, PageSize.y * TileY, 0,
					PageSize.x, PageSize.y, 1,
					GL_TRUE);
        RHIGetError();
        glTexSubImage2D(GLResource.Target, MipmapLevel,
					PageSize.x * TileX, PageSize.y * TileY,
					PageSize.x, PageSize.y,
					Format, Type,
					Data);
        RHIGetError();

    }


    int nilou::FOpenGLDynamicRHI::Initialize()
    {
        FDynamicRHI::Initialize();
        //bool ret = Initialize();
        //if (!ret)
        //    return ret;
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return false;
        }

        bool ret = gladLoadGL();
        if (!ret)
        {
            std::cerr << "RHI load failed!" << std::endl;
        }
        else
        {
            std::cout << "RHI Version " << GLVersion.major << "." 
                    << GLVersion.minor << " loaded" << std::endl;
            RHIGetError();
            auto config = GetAppication()->GetConfiguration();
            RHIGetError();
            RHISetViewport(config.screenWidth, config.screenHeight);
            RHIGetError();
            glGenVertexArrays(1, &ContextState.VertexArrayObject);
            glBindVertexArray(ContextState.VertexArrayObject);
            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);  

            magic_enum::enum_for_each<ETextureType>([](ETextureType TextureType) {
                magic_enum::enum_for_each<EPixelFormat>([TextureType](EPixelFormat PixelFormat) {
                    
                    ivec3 &PageSize = FDynamicRHI::SparseTextureTileSizes[(int)TextureType][(int)PixelFormat];
                    if (PixelFormat == EPixelFormat::PF_UNKNOWN)
                    {
                        PageSize = ivec3(1);
                        return;
                    }

                    auto [Format, InternalFormat, Type] = TranslatePixelFormat(PixelFormat);
                    GLenum Target;
                    switch (TextureType) 
                    {
                    case ETextureType::TT_Texture2D:
                        Target = GL_TEXTURE_2D;
                        break;
                    case ETextureType::TT_Texture2DArray:
                        Target = GL_TEXTURE_2D_ARRAY;
                        break;
                    case ETextureType::TT_Texture3D:
                        Target = GL_TEXTURE_3D;
                        break;
                    case ETextureType::TT_TextureCube:
                        Target = GL_TEXTURE_CUBE_MAP;
                        break;
                    }
                    glGetInternalformativ(Target, InternalFormat, GL_VIRTUAL_PAGE_SIZE_X_ARB, 1, &PageSize.x);
                    glGetInternalformativ(Target, InternalFormat, GL_VIRTUAL_PAGE_SIZE_Y_ARB, 1, &PageSize.y);
                    glGetInternalformativ(Target, InternalFormat, GL_VIRTUAL_PAGE_SIZE_Z_ARB, 1, &PageSize.z); 
                    
                });
            }); 

        }
        return ret;
    }

    void FOpenGLDynamicRHI::Finalize()
    {
        FDynamicRHI::Finalize();
    }

    FOpenGLDynamicRHI::OpenGLTextureResource FOpenGLDynamicRHI::TextureResourceCast(RHITexture *texture)
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
        RHIGetError();
        glTexStorage2D(target, nummips, internalformat, width, height);    
        RHIGetError();
        if (pixels)
            glTexSubImage2D(target, 0, 0, 0, width, height, format, type, pixels);
        RHIGetError();
    }
    void FOpenGLDynamicRHI::glTexImage3D_usingTexStorage(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint nummips, GLenum format, GLenum type, const void *pixels)
    {
        glTexStorage3D(target, nummips, internalformat, width, height, depth);
        if (pixels)
            glTexSubImage3D(target, 0, 0, 0, 0, width, height, depth, format, type, pixels);
    }
}