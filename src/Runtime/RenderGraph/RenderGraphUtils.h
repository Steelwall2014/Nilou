#include "RenderGraph.h"

namespace nilou {

FRHIVertexDeclaration* RDGGetScreenQuadVertexDeclaration();

RDGBuffer* RDGGetScreenQuadVertexBuffer(RenderGraph& Graph);

RDGBuffer* RDGGetScreenQuadIndexBuffer(RenderGraph& Graph);

template<typename T>
RDGBuffer* RDGCreateUniformBuffer(RenderGraph& Graph, const T& Data, const std::string& Name)
{
    RDGBuffer* Buffer = Graph.CreateUniformBuffer<T>(Name);
    Graph.QueueBufferUpload(Buffer, &Data, sizeof(Data));
    return Buffer;
}

}
