#pragma once

#include "Platform.h"

namespace nilou {

enum class ERefCountingMode : uint8
{
	/** Forced to be not thread-safe. */
	NotThreadSafe = 0,

	/** Thread-safe, never spin locks, but slower */
	ThreadSafe = 1
};

template <ERefCountingMode Mode=ERefCountingMode::ThreadSafe>
class TRefCountedObject
{
	using RefCountType = std::conditional_t<Mode == ERefCountingMode::ThreadSafe, std::atomic<uint32>, uint32>;

public:

    TRefCountedObject()
        : NumRefs(0)
    {
    }

    uint32 AddRef()
    {
        ++NumRefs;
        return NumRefs;
    }

    uint32 Release()
    {
        --NumRefs;
        if (NumRefs == 0)
        {
            delete this;
        }
        return NumRefs;
    }

    uint32 GetRefCount() const
    {
        return NumRefs;
    }

private:

	mutable RefCountType NumRefs = 0;
};

/**
 * A smart pointer to an object which implements AddRef/Release.
 */
template<typename ReferencedType>
class TRefCountPtr
{
	typedef ReferencedType* ReferenceType;

public:

    TRefCountPtr()
        : Reference(nullptr)
    {
    }

    TRefCountPtr(ReferencedType* InReference)
        : Reference(InReference)
    {
        if (Reference)
        {
            Reference->AddRef();
        }
    }

    TRefCountPtr(const TRefCountPtr& InCopy)
        : Reference(InCopy.Reference)
    {
        if (Reference)
        {
            Reference->AddRef();
        }
    }

    TRefCountPtr(TRefCountPtr&& InMove)
        : Reference(InMove.Reference)
    {
        InMove.Reference = nullptr;
    }

    ~TRefCountPtr()
    {
        if (Reference)
        {
            Reference->Release();
        }
    }

    TRefCountPtr& operator=(ReferencedType* InReference)
    {
        if (Reference != InReference)
        {
            if (Reference)
            {
                Reference->Release();
            }

            Reference = InReference;

            if (Reference)
            {
                Reference->AddRef();
            }
        }

        return *this;
    }

    TRefCountPtr& operator=(const TRefCountPtr& InCopy)
    {
        return *this = InCopy.Reference;
    }

    TRefCountPtr& operator=(TRefCountPtr&& InMove)
    {
        if (Reference != InMove.Reference)
        {
            if (Reference)
            {
                Reference->Release();
            }

            Reference = InMove.Reference;
            InMove.Reference = nullptr;
        }

        return *this;
    }

    ReferencedType* operator->() const
    {
        return Reference;
    }

    operator ReferenceType() const
	{
		return Reference;
	}

    ReferencedType* GetReference() const
    {
        return Reference;
    }

    operator bool() const
    {
        return Reference != nullptr;
    }

	bool operator==(const TRefCountPtr& B) const
	{
		return GetReference() == B.GetReference();
	}

	bool operator==(ReferencedType* B) const
	{
		return GetReference() == B;
	}

private:

    ReferencedType* Reference;

};

}

template<typename ReferencedType>
class std::hash<nilou::TRefCountPtr<ReferencedType>>
{
public:
    size_t operator()(const nilou::TRefCountPtr<ReferencedType>& RefPtr) const noexcept
    {
        return std::hash<ReferencedType*>{}(RefPtr.GetReference());
    }
};