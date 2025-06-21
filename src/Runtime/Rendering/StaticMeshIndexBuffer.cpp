#include "StaticMeshIndexBuffer.h"
#include "DynamicRHI.h"
#include "RHIDefinitions.h"
#include "RenderGraph.h"

namespace nilou {

    void FStaticMeshIndexBuffer::Init(uint32 InNumIndices, EIndexBufferStride DesiredStride)
    {
        NumIndices = InNumIndices;
        switch (DesiredStride) 
        {
            case EIndexBufferStride::Force8Bit:
                Stride = sizeof(uint8);
                Data = new uint8[NumIndices * Stride];
                break;
            case EIndexBufferStride::Force16Bit:
                Stride = sizeof(uint16);
                Data = new uint8[NumIndices * Stride];
                break;
            case EIndexBufferStride::Force32Bit:
                Stride = sizeof(uint32);
                Data = new uint8[NumIndices * Stride];
                break;
            default:
                Data = nullptr;
        }
        
    }

    void FStaticMeshIndexBuffer::Init(void *InData, uint32 InStride, uint32 InNumIndices)
    {
        Stride = InStride;
        NumIndices = InNumIndices;
        Data = new uint8[NumIndices * Stride];
        std::memcpy(Data, InData, NumIndices * Stride);
    }

    void FStaticMeshIndexBuffer::Init(const std::vector<uint8>& InIndices)
    {
        Stride = sizeof(uint8);
        NumIndices = InIndices.size();
        Data = new uint8[NumIndices * Stride];
        memcpy(Data, InIndices.data(), NumIndices * Stride);
    }
    void FStaticMeshIndexBuffer::Init(const std::vector<uint16>& InIndices)
    {
        Stride = sizeof(uint16);
        NumIndices = InIndices.size();
        Data = new uint8[NumIndices * Stride];
        memcpy(Data, InIndices.data(), NumIndices * Stride);
    }
    void FStaticMeshIndexBuffer::Init(const std::vector<uint32>& InIndices)
    {
        Stride = sizeof(uint32);
        NumIndices = InIndices.size();
        Data = new uint8[NumIndices * Stride];
        memcpy(Data, InIndices.data(), NumIndices * Stride);
    }

    void FStaticMeshIndexBuffer::CleanUp()
    {
        if (Data)
        {
            delete[] Data;
            Data = nullptr;
        }
    }

    void FStaticMeshIndexBuffer::InitRHI(RenderGraph& Graph)
    {
        FRenderResource::InitRHI(Graph);
        RDGBufferDesc Desc(NumIndices * Stride, Stride, EBufferUsageFlags::IndexBuffer);
        IndexBufferRDG = RenderGraph::CreateExternalBuffer("", Desc);
        // Graph.AddUploadPass(IndexBufferRDG.get(), Data, Desc.Size);
    }
}