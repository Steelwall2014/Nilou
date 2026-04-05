# Graphics Pipeline Reference

## Core Types

| Type | Description |
|------|-------------|
| `FGraphicsPipeline` | Pipeline type descriptor: name, permutation count, VS/PS `FShaderType*`, compile callbacks |
| `FGraphicsPipelinePermutationParameters` | Pipeline permutation parameters (parallel to `FShaderPermutationParameters`) |
| `RHIGraphicsPipelineShaders` (RHI) | VS/PS handles + optional Slang session/component; `IsValid()` |
| `FGraphicsPipelineStateInitializer` (RHI) | Fixed-function + RT state; **`Shaders`** is `RHIGraphicsPipelineShaders` (no separate VS/PS/Slang fields) |
| `FMaterialPipelineMap` | Material pipeline storage keyed by: Pipeline × Pipeline perm × VF × VF perm |
| `FMaterialShaderMap` | Material shader data; now holds only `FMaterialPipelineMap` |

## Declaration and Definition

### Basic (no permutation)

```cpp
// MyPipeline.h
DECLARE_GRAPHICS_PIPELINE(FMyPipeline, FMyVertexShader, FMyPixelShader)

// MyPipeline.cpp
IMPLEMENT_GRAPHICS_PIPELINE(FMyPipeline)
```

- `DECLARE_GRAPHICS_PIPELINE(Pipeline, VS, PS)` defines the class, binds VS/PS via `using VertexShaderType` / `using PixelShaderType`, and sets `FPermutationDomain = FShaderPermutationNone`.
- `IMPLEMENT_GRAPHICS_PIPELINE(Pipeline)` constructs `StaticType` (reading `VertexShaderType::StaticType` / `PixelShaderType::StaticType`) and registers into `GetAllGraphicsPipelines()`.
- VS/PS must already be declared with `DECLARE_MATERIAL_SHADER`/`DECLARE_GLOBAL_SHADER` + `IMPLEMENT_SHADER_TYPE`.
- VS and PS may have **different** `EShaderMetaType` (see Mixed MetaType below).

### Custom permutation domain and environment

```cpp
// MyPipeline.h
class FMyPipeline
{
public:
    using VertexShaderType = FMyVertexShader;
    using PixelShaderType  = FMyPixelShader;
    static FGraphicsPipeline StaticType;

    class FPermutationDomain : public TShaderPermutationDomain<
        class FUseSomeFeature : SHADER_PERMUTATION_BOOL("USE_SOME_FEATURE")
    > {};

    static bool ShouldCompilePermutation(const FGraphicsPipelinePermutationParameters& Params)
    {
        return true;
    }

    static void ModifyCompilationEnvironment(
        const FGraphicsPipelinePermutationParameters& Params,
        FShaderCompilerEnvironment& Env)
    {
        FPermutationDomain(Params.PermutationId).ModifyCompilationEnvironment(Env);
    }
};

// MyPipeline.cpp
IMPLEMENT_GRAPHICS_PIPELINE(FMyPipeline)
```

## Compilation

### Global pipelines (VS/PS both `EShaderMetaType::Global`)

Explicit call at engine startup, after RHI initialisation:

```cpp
FShaderCompiler::CompileComputeShaders();    // compile global compute shaders first
FShaderCompiler::CompileGlobalGraphicsPipelines(); // then compile global pipelines
```

Process per pipeline permutation:
1. Merge pipeline + VS + PS `ModifyCompilationEnvironment` into one `FShaderCompilerEnvironment`.
2. Load VS and PS modules in one Slang session.
3. Extract VS SPIR-V (entry index 0) and PS SPIR-V (entry index 1).
4. Create `RHIVertexShader` / `RHIPixelShader` from bytecode.
5. Insert into global `FGlobalGraphicsPipelineMap`.

> `CompileGlobalGraphicsPipelines` skips any pipeline whose VS or PS has `EShaderMetaType::Material`.

### Material pipelines (at least one stage is `EShaderMetaType::Material`)

No extra call needed. `FShaderCompiler::CompileMaterialShader` automatically:

1. Iterates all pipelines where **any** stage is Material (`ForEachMaterialGraphicsPipeline`).
2. Compilation per stage is conditional on its MetaType:

| Stage MetaType | Compilation |
|---|---|
| `Material` (VS) | Specialize with `(VF, Material, VFInput)` |
| `Material` (PS) | Specialize with `(Material)` |
| `Global` | No specialization; entry point used directly |

3. When VS is Material, the pipeline is compiled per `(Pipeline perm × VF type × VF perm)`.  
   When VS is Global, the pipeline is compiled once per Pipeline perm; stored using `FVertexFactory::StaticType` as the VF key (sentinel).
4. Result stored in `FMaterialShaderMap::PipelineMap`.

## Query

### Global pipeline

```cpp
RHIGraphicsPipelineShaders* P = GetGlobalGraphicsPipeline<FMyPipeline>();
RHIGraphicsPipelineShaders* P = GetGlobalGraphicsPipeline<FMyPipeline>(PermutationId);
```

### Material pipeline (via `FMaterialRenderProxy`)

```cpp
// VS=Material: pass actual VF params
RHIGraphicsPipelineShaders* P = RenderProxy->GetPipeline(PipelineParams, VFParams);

// VS=Global: use sentinel VF key
FVertexFactoryPermutationParameters NoVF(&FVertexFactory::StaticType, 0);
RHIGraphicsPipelineShaders* P = RenderProxy->GetPipeline(PipelineParams, NoVF);
```

### Validity check before use

```cpp
if (P && P->IsValid())
{
    RHIVertexShader* VS = static_cast<RHIVertexShader*>(P->VertexShader.GetReference());
    RHIPixelShader*  PS = static_cast<RHIPixelShader*>(P->PixelShader.GetReference());
}
```

## Constraints

- **Mixed MetaType is supported**: if either VS or PS is Material, the pipeline is treated as a material pipeline and compiled by `CompileMaterialShader`. Both stages being Global is the only case handled by `CompileGlobalGraphicsPipelines`.
- Global pipeline VS/PS must have non-empty `FileAbsolutePath` (set via `IMPLEMENT_SHADER_TYPE`).
- Pipeline display name removes the `F` prefix; `HashedName` is derived from the class name only (not from a file path).
- Pipeline permutation controls the shared compile environment for VS and PS; independent per-stage permutation must be layered manually inside `ModifyCompilationEnvironment`.
- Material shaders are **not compiled standalone** — they are only compiled as part of a registered pipeline. `DECLARE_MATERIAL_SHADER` is still required to set `EShaderMetaType::Material` for filter logic.
