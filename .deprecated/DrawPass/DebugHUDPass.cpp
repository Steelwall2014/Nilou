#include "Interface/IApplication.h"
#include "DebugHUDPass.h"
#include "Common/ShaderManager.h"
#include "Common/TextureManager.h"
#include "DynamicRHI.h"

namespace und {
    void drawTexture(RHITextureRef texture, int vertices_size, float *vertices, int uv_size, float *uv, int i)
    {
        _GM->RHIBindTexture("Texture", texture);
        _GM->RHISetShaderParameter("layer_index", (float)i);
        RHIVertexArrayObjectRef VAO = _GM->RHICreateVertexArrayObject(EPrimitiveMode::PM_Triangle_Strip);
        auto VBO = _GM->RHICreateVertexAttribBuffer(0, vertices_size, vertices, EVertexElementTypeFlags::VET_Float | EVertexElementTypeFlags::VET_3, EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::Static);
        auto UV = _GM->RHICreateVertexAttribBuffer(1, uv_size, uv, EVertexElementTypeFlags::VET_Float | EVertexElementTypeFlags::VET_2, EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::Static);
        VAO->AddVertexAttribBuffer(VBO.first, VBO.second);
        VAO->AddVertexAttribBuffer(UV.first, UV.second);
        _GM->RHIInitializeVertexArrayObject(VAO);
        _GM->RHIDrawVertexArray(VAO);
    }

    void drawTexture(RHITextureRef texture, int vertices_size, float *vertices, int uv_size, float *uv)
    {
        _GM->RHIBindTexture("Texture", texture);
        RHIVertexArrayObjectRef VAO = _GM->RHICreateVertexArrayObject(EPrimitiveMode::PM_Triangle_Strip);
        auto VBO = _GM->RHICreateVertexAttribBuffer(0, vertices_size, vertices, EVertexElementTypeFlags::VET_Float | EVertexElementTypeFlags::VET_3, EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::Static);
        auto UV = _GM->RHICreateVertexAttribBuffer(1, uv_size, uv, EVertexElementTypeFlags::VET_Float | EVertexElementTypeFlags::VET_2, EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::Static);
        VAO->AddVertexAttribBuffer(VBO.first, VBO.second);
        VAO->AddVertexAttribBuffer(UV.first, UV.second);
        _GM->RHIInitializeVertexArrayObject(VAO);
        _GM->RHIDrawVertexArray(VAO);
    }

    void createVertices(float left, float top, float ndc_width, float ndc_height, float *vertices)
    {
        vertices[0] = left; vertices[1] = top; vertices[2] = 0.0f;
        vertices[3] = left; vertices[4] = top - ndc_height; vertices[5] = 0.0f;
        vertices[6] = left + ndc_width; vertices[7] = top; vertices[8] = 0.0f;
        vertices[9] = left + ndc_width; vertices[10] = top - ndc_height; vertices[11] = 0.0f;
    }
}


void und::DebugHUDPass::Draw(FrameVariables &frame)
{


#ifdef _DEBUG
    float top = 0.95f;
    float size = 160;
    float ndc_width = size / g_pApp->GetConfiguration().screenWidth * 2;
    float ndc_height = size / g_pApp->GetConfiguration().screenHeight * 2;
    float left = 0.95 - ndc_width;

    float uv[] = {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 0.0f
    };
    float vertices[12];
    RHIDepthStencilState state;
    state.EnableDepthTest = false;
    _GM->RHISetDepthStencilState(state);
    _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Debug HUD Texture2DArray"));
    for (uint32_t i = 0; i < frame.frameContext.shadowMapArray->GetSizeZ(); i++)
    {
        createVertices(left, top, ndc_width, ndc_height, vertices);
        drawTexture(frame.frameContext.shadowMapArray, sizeof(vertices), vertices, sizeof(uv), uv, i);
        top -= ndc_height + 0.05;
    }
    // OceanMultiScatteringLUT[1]  OceanScatteringLUT
    // _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Debug HUD Texture2D"));
    // _GM->RHISetShaderParameter("scale", 1.f);
    // createVertices(left, top, ndc_width, ndc_height, vertices);
    // drawTexture(g_pTextureManager->GetGlobalTexture("PerlinNoise"), sizeof(vertices), vertices, sizeof(uv), uv);
    // top -= ndc_height + 0.05;

    // ndc_width = 256.0 / g_pApp->GetConfiguration().screenWidth * 2;
    // ndc_height = 128.0 / g_pApp->GetConfiguration().screenHeight * 2;
    // left = 0.95 - ndc_width;
    // _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Debug HUD Texture3D"));
    // _GM->RHISetShaderParameter("scale", 10.0f);
    // createVertices(left, top, ndc_width, ndc_height, vertices);
    // drawTexture(g_pTextureManager->GetGlobalTexture("OceanScatteringLUT"), sizeof(vertices), vertices, sizeof(uv), uv);
    // top -= ndc_height + 0.05;

    //_GM->RHISetShaderParameter("scale", 1.f);
    //OpenGLTexture2DRef ColorBuffer = std::dynamic_pointer_cast<OpenGLTexture2D>(frame.frameContext.renderTarget->Attachments[GL_COLOR_ATTACHMENT1]);
    //ndc_width = 0.3;
    //ndc_height = 0.3;
    //createVertices(left-0.1, top, ndc_width, ndc_height, vertices);
    //drawTexture(ColorBuffer, sizeof(vertices), vertices, sizeof(uv), uv);
    //top -= ndc_height + 0.05;

    state.EnableDepthTest = true;
    _GM->RHISetDepthStencilState(state);
    //GDynamicRHI->DrawOverlayOceanDisplacement(left, top, ndc_width, ndc_height);
#endif
}
