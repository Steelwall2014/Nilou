# NilouShaderTool Reference

## Purpose

`NilouShaderTool` scans `.slang` shader files before compilation and generates C++ shader parameter bindings from reflected `ParameterBlock<T>` types.

## Shader search paths（Slang `import` 解析）

Slang 会话使用 `FShaderCompilerEnvironment::SearchPaths`（`ShaderCompiler.cpp` 里传入 `sessionDesc.searchPaths`）。

**约定：`NilouShaderTool`（`xmake codegen`）与运行时着色器编译使用相同的 Slang `import` 搜索路径**——顺序均为 `FPaths::EngineShadersPublicDir()`（`Engine/Shaders/Public`），再 `FPaths::EngineDir()`（`Engine` 根目录）。codegen 侧由 `xmake/modules/nilou_codegen.lua` 传入的 `-SearchDirectories` 与上述路径一致。

工程约定（与 `.cursor/rules` 一致）：**可复用、应对外「全局可见」的着色器模块放在 `Engine/Shaders/Public`**；`Private` 下为各功能域内部实现。在 Public → Engine 的 search path 下，可用短模块名引用 `Public`，或用 **带引号的相对 Engine 根的路径** 拉取 `Private` 树中的文件（见下节）。

## Import 规则（Slang `import`）

**约定**：着色器源中只使用 Slang `import` 组织依赖，不使用 C 预处理 `#include`。

- **模块入口**：运行时用 `loadModuleFromSourceString`，**逻辑模块名取自主文件名（不含扩展名）**，须与文件内第一行 `module <Name>;` 一致（`ShaderCompiler.cpp` 中 `moduleFilePath.stem()`）。
- **短名导入** `import Foo;`：在会话的 `searchPaths` 下按 Slang 规则解析（通常对应各搜索路径目录中的 `Foo.slang` / 模块声明）。适合引用放在 **`Engine/Shaders/Public`** 的共享模块（如 `RenderCore`）。
- **字符串路径导入** `import "Shaders/Private/...";`：按路径相对 **某个搜索路径根** 解析。在已加入 `FPaths::EngineDir()` 的管线中，可使用以 `Shaders/Private/...` 开头的路径引用 `Private` 树内文件，例如工程内已有：  
  `import "Shaders/Private/SkyAtmosphere/atmosphere_definitions";`
- **`implementing` 文件**：不参与 `NilouShaderTool` 扫描，由 Slang 在链接/特化阶段按需拉取；仍需遵守 `module`/`implementing` 与路径约定。

**硬约束（与构建规则一致）**：每个 `.slang` 文件开头必须是 `module ...;` 或 `implementing ...;`；着色器侧常量与资源参数须落在 `ParameterBlock<T>` 中以便反射生成绑定；模块依赖仅通过 Slang `import`，不使用 `#include`。

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
- Uniform **arrays** are emitted with `NumElements` and `ArrayStride`; struct arrays also emit expanded child rows such as `Foo[0].Bar` with precise offsets.

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
- Stale `.generated.h` / `.gen.cpp` outputs are replaced with empty placeholders instead of being deleted. This prevents xmake from compiling a source path that was collected before codegen removed it, while still removing stale shader metadata registration code.

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
- Obsolete generated files without reflectable `ParameterBlock<T>` bindings are emitted as empty placeholders.
