#include <ctime>
#include <map>
#include <unordered_map>
#include <iostream>
#include <array>

using uint8 = unsigned char;
using uint32 = unsigned int;

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
    {
        this->CopyConstructFrom(InReference);
    }

    template<typename OtherReferencedType, std::enable_if_t<std::is_convertible_v<OtherReferencedType*, ReferencedType*>, int> = 0>
    TRefCountPtr(OtherReferencedType* InReference)
    {
        this->CopyConstructFrom(InReference);
    }

    TRefCountPtr(const TRefCountPtr& InCopy)
    {
        this->CopyConstructFrom(InCopy.Reference);
    }

    template<typename OtherReferencedType, std::enable_if_t<std::is_convertible_v<OtherReferencedType*, ReferencedType*>, int> = 0>
    TRefCountPtr(const TRefCountPtr<OtherReferencedType>& InCopy)
    {
        this->CopyConstructFrom(InCopy.Reference);
    }

    TRefCountPtr(TRefCountPtr&& InMove)
    {
        this->MoveConstructFrom(std::move(InMove));
    }

    template<typename OtherReferencedType, std::enable_if_t<std::is_convertible_v<OtherReferencedType*, ReferencedType*>, int> = 0>
    TRefCountPtr(TRefCountPtr<OtherReferencedType>&& InMove)
    {
        this->MoveConstructFrom(std::move(InMove));
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

    template<typename OtherReferencedType, std::enable_if_t<std::is_convertible_v<OtherReferencedType*, ReferencedType*>, int> = 0>
    TRefCountPtr& operator=(const TRefCountPtr<OtherReferencedType>& InCopy)
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

    template<typename OtherReferencedType, std::enable_if_t<std::is_convertible_v<OtherReferencedType*, ReferencedType*>, int> = 0>
    TRefCountPtr& operator=(TRefCountPtr<OtherReferencedType>&& InMove)
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

    template <typename OtherReferencedType>
    friend class TRefCountPtr;

    void CopyConstructFrom(ReferencedType* InReference)
    {
        Reference = InReference;
        if (Reference)
        {
            Reference->AddRef();
        }
    }

    template <typename OtherReferencedType>
    void MoveConstructFrom(TRefCountPtr<OtherReferencedType>&& InMove)
    {
        Reference = InMove.Reference;
        InMove.Reference = nullptr;
    }

};

class A : public TRefCountedObject<> { };
class B : public A { };
class C { };

int main()
{
    {
        TRefCountPtr<A> pa = new A;
        TRefCountPtr<A> pb = new B;
    }
    {
        TRefCountPtr<A> pa = new A;
        TRefCountPtr<B> pb = new B;
        pa = pb;
    }
    {
        // TRefCountPtr<A> pa = new C;
    }
    {
        std::vector<TRefCountPtr<A>> v;
        TRefCountPtr<B> pb = new B;
        v.push_back(pb);
    }
    {
        std::vector<std::shared_ptr<A>> v;
        std::shared_ptr<B> pb = std::shared_ptr<B>(new B);
        v.push_back(pb);
    }
    return 0;
}