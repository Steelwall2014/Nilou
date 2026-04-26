# NilouShaderTool Reference

## Purpose

`NilouShaderTool` scans `.slang` shader files before compilation and generates C++ shader parameter bindings from reflected `ParameterBlock<T>` types.

## Workflow

1. Scan `.slang` files in input directory.
2. Keep only module files (`module ...;`), skip `implementing` files.
3. Reflect modules via Slang, collect structs used by `ParameterBlock<T>`.
4. Emit `.generated.h` and `.gen.cpp` for each collected struct.

## CLI

```text
NilouShaderTool
  -InputDirectory=<shader source dir>
  -OutputHeaderDirectory=<generated header dir>
  -OutputCppDirectory=<generated cpp dir>
  [-SearchDirectories=<path1>;<path2>;...]
  [-ForceRegenerate]
```

## Generation Trigger

Only structs used by `ParameterBlock<T>` (including member structs) produce bindings.

## Layout Rules

- `EShaderDataLayout::Opaque`: texture/sampler/buffer resource fields.
- `EShaderDataLayout::Std140` and `Std430`: scalar/vector/matrix/nested struct fields.
- If a struct contains both resource and scalar data, Opaque specialization introduces:
  - `RDGBuffer* AutomaticallyIntroducedUniformBuffer`

## Generated Header Pattern

- Primary template declaration: `template <EShaderDataLayout DataLayout> struct TName {};`
- Opaque specialization for descriptors/resources.
- Std140/Std430 specialization for uploadable aligned data fields.

## Generated Cpp Pattern

- `CreateDescriptorSetLayout_<StructName>()`
- `GetMembers_<StructName>()`
- `GetShaderParametersMetadata<T>()` specialization
- Static registration to `FShaderParameterRegistry`

## `FShaderParametersMetadata2::Members`

`GetMembers_*` fills `std::vector<FShaderParametersMetadata2::FMember>` with:

- Descriptor entries: `AutomaticallyIntroducedUniformBuffer` and each resource/sampler/buffer field (`BindingIndex` is the Vulkan binding in the set).
- **Uniform block layout**: one row per scalar/vector/matrix field inside the Std140/Std430 payload, and nested struct fields (recursive, qualified names like `Outer.Inner`). For these rows, `BindingIndex` is **0**; use `BaseType` (numeric vs resource) and `Name` to distinguish from real binding 0 when needed. `Offset` is the byte offset in the uniform block from Slang `ParameterCategory::Uniform`.
- Uniform **arrays** are not emitted yet (tool logs and skips).

## Type Mapping (Typical)

- `Sampler2D<T>` / `Texture2D<T>` -> `RDGTextureView*`
- `SamplerState` -> `RHISamplerState*`
- combined texture+sampler -> `RDGCombinedTextureSampler`
- `StructuredBuffer<T>` / `RWStructuredBuffer<T>` -> `RDGBuffer*`
- `ConstantBuffer<T>` -> `RDGBuffer*`

## Incremental Build Behavior

- Cache file: `CachedShaderModifiedTime.txt`
- Unchanged source timestamps skip generation.
- `-ForceRegenerate` bypasses cache.
- Content compare write avoids touching files with identical output.

## Xmake Integration

- Normal editor/game builds run shader binding generation through `NilouCodegen`.
- `NilouCodegen` depends on `NilouHeaderTool` and `NilouShaderTool` inside the same xmake build graph, then runs the generators.
- The `build.fence` policy on `NilouCodegen` prevents generated shader binding sources from racing with C++ compilation.
- `xmake codegen` is the manual entry point for refreshing generated files without building the editor.
- `xmake codegen --force` passes `-ForceRegenerate` to `NilouShaderTool`.
- `ShaderBindings/Generated` is owned by `NilouShaderTool`; the `ShaderBindings` module uses `skip_header_tool = true` so `NilouHeaderTool` stale-file cleanup does not remove shader-generated `.gen.cpp` files.

## Notes

- `implementing` files are skipped and pulled by Slang compiler implicitly.
- Nested `ParameterBlock<ParameterBlock<T>>` is not recursively parsed.
- Layout mismatch across usage contexts causes errors.
- `PushConstant` parameter blocks do not generate `FShaderParametersMetadata2`.
- Obsolete generated files without source correspondences are cleaned up.
