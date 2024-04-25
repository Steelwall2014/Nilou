#pragma once

#include <vector>
#include "RenderResource.h"

namespace nilou {

    enum class EIndexBufferStride
    {
		/** Forces all indices to be 8-bit. */
		Force8Bit = 0,
		/** Forces all indices to be 16-bit. */
		Force16Bit = 1,
		/** Forces all indices to be 32-bit. */
		Force32Bit = 2,
    };

    class FStaticMeshIndexBuffer : public FIndexBuffer
    {
    public:

        FStaticMeshIndexBuffer()
            : Data(nullptr)
        { }

        ~FStaticMeshIndexBuffer()
        {
            CleanUp();
        }

        void Init(uint32 InNumIndices, EIndexBufferStride DesiredStride);

        void Init(void *Data, uint32 InStride, uint32 InNumIndices);

        void Init(const std::vector<uint8>& InIndices);
        void Init(const std::vector<uint16>& InIndices);
        void Init(const std::vector<uint32>& InIndices);

        void CleanUp();

        virtual void InitRHI() override;

        RHIBufferRef CreateRHIBuffer_RenderThread();

        inline uint32 GetNumIndices() const
        {
            return NumIndices;
        }

        inline uint32 GetStride() const
        {
            return Stride;
        }

        void* GetIndiceData() { return Data; }

        const void* GetIndiceData() const { return Data; }

    private:

        uint8 *Data;

        // uint32 Stride;

        // uint32 NumIndices;

        // bool bNeedsCPUAccess = true;

    };


}