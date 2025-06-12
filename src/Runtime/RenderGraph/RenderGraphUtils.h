#include "RenderGraph.h"

namespace nilou {

FRHIVertexDeclaration* GetScreenQuadVertexDeclaration();

RDGBuffer* GetScreenQuadVertexBuffer(RenderGraph& Graph);

RDGBuffer* GetScreenQuadIndexBuffer(RenderGraph& Graph);

template<typename T>
RDGBuffer* CreateUniformBuffer(RenderGraph& Graph, const T& Data)
{
    RDGBuffer* Buffer = Graph.CreateUniformBuffer<T>("UniformBuffer");
    Graph.QueueBufferUpload(Buffer, &Data, sizeof(Data));
    return Buffer;
}

}
