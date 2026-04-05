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

## Notes

- `implementing` files are skipped and pulled by Slang compiler implicitly.
- Nested `ParameterBlock<ParameterBlock<T>>` is not recursively parsed.
- Layout mismatch across usage contexts causes errors.
- `PushConstant` parameter blocks do not generate `FShaderParametersMetadata2`.
- Obsolete generated files without source correspondences are cleaned up.
