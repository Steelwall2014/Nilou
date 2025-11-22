#include "RenderGraph.h"

namespace nilou {

RENDERCORE_API FRHIVertexDeclaration* RDGGetScreenQuadVertexDeclaration();

RENDERCORE_API RDGBuffer* RDGGetScreenQuadVertexBuffer(RenderGraph& Graph);

RENDERCORE_API RDGBuffer* RDGGetScreenQuadIndexBuffer(RenderGraph& Graph);

template<typename T>
RDGBuffer* RDGCreateUniformBuffer(RenderGraph& Graph, const T& Data, const std::string& Name)
{
    RDGBuffer* Buffer = Graph.CreateUniformBuffer<T>(Name);
    Graph.QueueBufferUpload(Buffer, &Data, sizeof(Data));
    return Buffer;
}

}
