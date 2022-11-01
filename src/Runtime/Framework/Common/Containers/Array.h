#pragma once
#include "Platform.h"

namespace nilou {
    template <typename InElementType, uint32 NumElements, uint32 Alignment = alignof(InElementType)>
    class alignas(Alignment) TAlignedStaticArray
    {
    public:
        TAlignedStaticArray() { }

        InElementType &operator[](size_t index)
        {
            return Elements[index].Element;
        }

        const InElementType &operator[](size_t index) const
        {
            return Elements[index].Element;
        }

        friend bool operator==(const TAlignedStaticArray& A,const TAlignedStaticArray& B)
        {
            for(uint32 ElementIndex = 0;ElementIndex < NumElements;++ElementIndex)
            {
                if(!(A[ElementIndex] == B[ElementIndex]))
                {
                    return false;
                }
            }
            return true;
        }

        friend bool operator!=(const TAlignedStaticArray& A,const TAlignedStaticArray& B)
        {
            for(uint32 ElementIndex = 0;ElementIndex < NumElements;++ElementIndex)
            {
                if(!(A[ElementIndex] == B[ElementIndex]))
                {
                    return true;
                }
            }
            return false;
        }

        bool IsEmpty() const
        {
            return NumElements == 0;
        }

        int32 Num() const { return NumElements; }



    private:
        struct alignas(Alignment) TArrayStorageElementAligned
        {
            TArrayStorageElementAligned() {}

            InElementType Element;
        };
        TArrayStorageElementAligned Elements[NumElements];
    };
}