#include "RenderGraphUtils.h"

namespace nilou {

struct FScreenQuadVertex
{
    vec4 Position;
    vec2 TexCoord;
};

FRHIVertexDeclaration* RDGGetScreenQuadVertexDeclaration()
{
    static FRHIVertexDeclaration* ScreenQuadVertexDeclaration = nullptr;
    if (!ScreenQuadVertexDeclaration)
    {
        FVertexDeclarationElementList Elements;
        Elements[0] = FVertexElement(0, 0, EVertexElementType::Float4, 0, sizeof(FScreenQuadVertex));
        Elements[1] = FVertexElement(0, sizeof(vec4), EVertexElementType::Float2, 1, sizeof(FScreenQuadVertex));
        ScreenQuadVertexDeclaration = RHICreateVertexDeclaration(Elements);
    }
    return ScreenQuadVertexDeclaration;
}

RDGBuffer* RDGGetScreenQuadVertexBuffer(RenderGraph& Graph)
{
    static RDGBufferRef ScreenQuadVertexBuffer = nullptr;
    if (!ScreenQuadVertexBuffer)
    {
        RDGBufferDesc Desc(sizeof(FScreenQuadVertex) * 4, sizeof(FScreenQuadVertex), EBufferUsageFlags::VertexBuffer);
        ScreenQuadVertexBuffer = RenderGraph::CreateExternalBuffer("ScreenQuadVertexBuffer", Desc);
        FScreenQuadVertex Data[4] = {
            {vec4(-1.0f, -1.0f, 0.0f, 1.0f), vec2(0.0f, 0.0f)},
            {vec4(1.0f, -1.0f, 0.0f, 1.0f), vec2(1.0f, 0.0f)},
            {vec4(-1.0f, 1.0f, 0.0f, 1.0f), vec2(0.0f, 1.0f)},
            {vec4(1.0f, 1.0f, 0.0f, 1.0f), vec2(1.0f, 1.0f)},
        };
        Graph.QueueBufferUpload(ScreenQuadVertexBuffer.GetReference(), Data, sizeof(Data));
    }
    return ScreenQuadVertexBuffer.GetReference();
}

RDGBuffer* RDGGetScreenQuadIndexBuffer(RenderGraph& Graph)
{
    static RDGBufferRef ScreenQuadIndexBuffer = nullptr;
    if (!ScreenQuadIndexBuffer)
    {
        RDGBufferDesc Desc(sizeof(uint16) * 6, sizeof(uint16), EBufferUsageFlags::IndexBuffer);
        ScreenQuadIndexBuffer = RenderGraph::CreateExternalBuffer("ScreenQuadIndexBuffer", Desc);
        uint16 Data[6] = {
            0, 1, 2,
            1, 3, 2
        };
        Graph.QueueBufferUpload(ScreenQuadIndexBuffer.GetReference(), Data, sizeof(Data));
    }
    return ScreenQuadIndexBuffer.GetReference();
}

}