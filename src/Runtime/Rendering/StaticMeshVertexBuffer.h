#pragma once

#include <vector>
#include "DynamicRHI.h"
#include "RenderResource.h"

namespace nilou {

    template<class VertexType>
    class FStaticMeshVertexBuffer : public FVertexBuffer
    {
    public:

        FStaticMeshVertexBuffer()
            : Data(nullptr)
            , Stride(0)
            , NumVertices(0)
        { }

        virtual ~FStaticMeshVertexBuffer()
        {
            CleanUp();
            FVertexBuffer::~FVertexBuffer();
        }

        void Init(uint32 InNumVertices, bool bInNeedsCPUAccess = true);

        void Init(const std::vector<VertexType>& InVertices, bool bInNeedsCPUAccess = true);

        void CleanUp();

        virtual void InitRHI() override;

        RHIBufferRef CreateRHIBuffer_RenderThread();

        inline uint32 GetStride() const
        {
            return Stride;
        }

        inline uint32 GetNumVertices() const
        {
            return NumVertices;
        }

        void* GetVertexData() { return Data; }

        const void* GetVertexData() const { return Data; }

        VertexType GetVertexValue(int index)
        {
            VertexType *ptr = reinterpret_cast<VertexType *>(Data) + index;
            return *ptr;
        }

        void SetVertexValue(int index, const VertexType &Value)
        {
            VertexType *ptr = reinterpret_cast<VertexType *>(Data) + index;
            *ptr = Value;
        }

        void BindToVertexFactoryData(FVertexStreamComponent &OutStreamComponent, uint32 Offset = 0)
        {
            if constexpr (std::is_same<VertexType, vec2>::value)
            {
                OutStreamComponent = FVertexStreamComponent(
                    this, 
                    Offset, 
                    GetStride(), 
                    EVertexElementType::VET_Float2);
            }
            else if constexpr (std::is_same<VertexType, vec3>::value)
            {
                OutStreamComponent = FVertexStreamComponent(
                    this, 
                    Offset, 
                    GetStride(), 
                    EVertexElementType::VET_Float3);
            }
            else if constexpr (std::is_same<VertexType, vec4>::value)
            {
                OutStreamComponent = FVertexStreamComponent(
                    this, 
                    Offset, 
                    GetStride(), 
                    EVertexElementType::VET_Float4);
            }
        }

    private:

        uint8 *Data;

        uint32 Stride;

        uint32 NumVertices;

        bool bNeedsCPUAccess = true;

    };

    template<class VertexType>
    void FStaticMeshVertexBuffer<VertexType>::Init(uint32 InNumVertices, bool bInNeedsCPUAccess)
    {
        Stride = sizeof(VertexType);
        NumVertices = InNumVertices;
        if (NumVertices > 0)
            Data = (uint8 *)std::realloc(Data, NumVertices * sizeof(VertexType));
    }

    template<class VertexType>
    void FStaticMeshVertexBuffer<VertexType>::Init(const std::vector<VertexType>& InPositions, bool bInNeedsCPUAccess)
    {
        Stride = sizeof(VertexType);
        NumVertices = InPositions.size();
        if (NumVertices > 0)
        {
            Data = (uint8 *)std::realloc(Data, NumVertices * sizeof(VertexType));
            memcpy(Data, InPositions.data(), NumVertices * sizeof(VertexType));
        }
    }

    template<class VertexType>
    void FStaticMeshVertexBuffer<VertexType>::CleanUp()
    {
        if (Data)
        {
            delete[] Data;
            Data = nullptr;
        }
    }

    template<class VertexType>
    void FStaticMeshVertexBuffer<VertexType>::InitRHI()
    {
        FRenderResource::InitRHI();
        VertexBufferRHI = CreateRHIBuffer_RenderThread();
    }

    template<class VertexType>
    RHIBufferRef FStaticMeshVertexBuffer<VertexType>::CreateRHIBuffer_RenderThread()
    {
        return FDynamicRHI::GetDynamicRHI()->RHICreateBuffer(Stride, NumVertices * sizeof(VertexType), 
            EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::Static, Data);
    }

}