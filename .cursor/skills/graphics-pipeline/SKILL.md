---
name: graphics-pipeline
description: Handle Nilou graphics pipeline type declaration, permutation, compilation, and lookup workflows for both global and material pipelines. Use when editing DECLARE_GRAPHICS_PIPELINE, IMPLEMENT_GRAPHICS_PIPELINE, CompileMaterialShader, CompileGlobalGraphicsPipelines, CompileComputeShaders, FMaterialShaderMap, FMaterialPipelineMap, RHIGraphicsPipelineShaders, GetGlobalGraphicsPipeline, GetPipeline, or when users mention FGraphicsPipeline, pipeline registration, pipeline compile, VS/PS pairing, or mixed MetaType.
---

# Graphics Pipeline Workflow

## Quick Start

Material shaders are **only compiled and cached as part of a pipeline** — no standalone VS/PS shader cache exists.

1. Confirm pipeline declaration/definition pairing and MetaType consistency (Global or Material, not mixed).
2. Verify VS/PS shader types are declared with the correct MetaType.
3. Check permutation domain and environment hooks.
4. Verify compile order and retrieval path.

## Execution Checklist

- [ ] Header: `DECLARE_GRAPHICS_PIPELINE(Pipeline, VS, PS)`; source: `IMPLEMENT_GRAPHICS_PIPELINE(Pipeline)`.
- [ ] VS/PS are declared with `DECLARE_GLOBAL_SHADER`/`DECLARE_MATERIAL_SHADER` + `IMPLEMENT_SHADER_TYPE`.
- [ ] If either VS or PS is Material, the pipeline participates in material compilation automatically.
- [ ] For mixed MetaType (one Global, one Material): Global stage is not specialized; Material stage is.
- [ ] VS=Global + PS=Material pipelines: query with sentinel VF key `(&FVertexFactory::StaticType, 0)`.
- [ ] `ShouldCompilePermutation` and `ModifyCompilationEnvironment` are coherent.
- [ ] Query uses `GetGlobalGraphicsPipeline<>()` (both Global) or `RenderProxy->GetPipeline()` (any Material stage).
- [ ] Validity checked before dereferencing `RHIGraphicsPipelineShaders`.

## Compile Order

**Global pipelines** — explicit call at startup (after RHI init):

```cpp
FShaderCompiler::CompileComputeShaders();    // Compute only
FShaderCompiler::CompileGlobalGraphicsPipelines(); // VS+PS both Global
```

**Material pipelines** — automatic. `FShaderCompiler::CompileMaterialShader` handles it internally (triggers when any stage is Material).

## Additional Resources

- Detailed type system, API, and constraints: [reference.md](reference.md)
