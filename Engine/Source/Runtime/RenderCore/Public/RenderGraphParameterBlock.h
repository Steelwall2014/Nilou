#pragma once
#include <string>

#include "ShaderParameter.h"
#include "RenderGraphDescriptorSet.h"

namespace nilou {

class RenderGraph;

struct FBaseParameterBlock : public TRefCountedObject<ERefCountingMode::NotThreadSafe>
{
    friend class RenderGraph;

    RDGDescriptorSet* GetDescriptorSet() const { return DescriptorSet; }

    virtual ~FBaseParameterBlock()
    {
        // Pooled blocks own their DescriptorSet ref (AddRef'd in UpdateParameterBlock).
        // Transient blocks' DescriptorSet lifetime is managed by RenderGraph::DescriptorSets.
        if (!bTransient && DescriptorSet)
            DescriptorSet->Release();
    }

protected:
    std::string Name;

    FShaderParametersMetadata2* Metadata = nullptr;

    // Transient blocks: non-owning pointer; DS lifetime managed by RenderGraph::DescriptorSets.
    // Pooled blocks: owning pointer; one extra AddRef is held and released in the destructor.
    RDGDescriptorSet* DescriptorSet = nullptr;

    bool bTransient = false;
};

template <template <EShaderDataLayout> typename T>
struct TParameterBlock
    : public T<Std140Layout>
    , public T<OpaqueLayout>
    , public FBaseParameterBlock
{
    friend class RenderGraph;

    TParameterBlock()
    {
        Metadata = GetShaderParametersMetadata<T>();
    }

    T<Std140Layout>& GetNonOpaqueFields()
    {
        return *static_cast<T<Std140Layout>*>(this);
    }
    T<EShaderDataLayout::Opaque>& GetOpaqueFields()
    {
        return *static_cast<T<OpaqueLayout>*>(this);
    }
    const T<Std140Layout>& GetNonOpaqueFields() const
    {
        return *static_cast<const T<Std140Layout>*>(this);
    }
    const T<EShaderDataLayout::Opaque>& GetOpaqueFields() const
    {
        return *static_cast<const T<OpaqueLayout>*>(this);
    }
};

template <template <EShaderDataLayout> typename T>
using TParameterBlockRef = TRefCountPtr<TParameterBlock<T>>;

} // namespace nilou
