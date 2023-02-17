#include "StaticMeshIndexBuffer.h"
#include "DynamicRHI.h"
#include "RHIDefinitions.h"

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

    void FStaticMeshIndexBuffer::InitRHI()
    {
        FRenderResource::InitRHI();
        IndexBufferRHI = CreateRHIBuffer_RenderThread();
    }

    RHIBufferRef FStaticMeshIndexBuffer::CreateRHIBuffer_RenderThread()
    {
        return FDynamicRHI::GetDynamicRHI()->RHICreateBuffer(Stride, NumIndices * Stride, 
            EBufferUsageFlags::IndexBuffer | EBufferUsageFlags::Static, Data);
    }
}