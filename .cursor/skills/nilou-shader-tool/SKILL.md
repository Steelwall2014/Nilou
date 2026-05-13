---
name: nilou-shader-tool
description: Handle Nilou Slang shader reflection and generated binding workflows. Use when editing Nilou shader files, ParameterBlock definitions, ShaderReflection generation logic, or when users mention NilouShaderTool, generated.h, gen.cpp, descriptor set layout, Std140/Std430, opaque shader parameters, shader search paths, Slang import rules, or shader file naming (e.g. *_Mat.slang).
---

# Nilou Shader Tool Workflow

## Quick Start

Use this skill for tasks related to Nilou shader reflection code generation.

1. Confirm whether the target `.slang` file is a module file (`module ...;`).
2. Check whether struct types are actually referenced by `ParameterBlock<T>`.
3. Apply or review generation rules for `Opaque` and `Std140`/`Std430` layouts.
4. Validate generated outputs (`.generated.h` and `.gen.cpp`) and metadata registration.
5. **Paths & imports**: shared modules live under `Engine/Shaders/Public`; shaders use **only** Slang `import`. **Codegen and in-engine shader compilation use the same Slang search path order** (`Engine/Shaders/Public`, then `Engine` root). See [reference.md](reference.md).
6. **Naming**: material sources typically `Something_Mat.slang`; vertex factories `*VertexFactory.slang`; keep `module` name equal to the file stem (required by `loadModuleFromSourceString`).

## Execution Checklist

- [ ] `ParameterBlock<T>` usage is present for all expected generated structs.
- [ ] Resource fields and scalar/vector/matrix fields are split to the correct data layout.
- [ ] `AutomaticallyIntroducedUniformBuffer` exists when opaque and scalar fields coexist.
- [ ] Descriptor bindings and member offsets are consistent with generated metadata.
- [ ] Shared `import` targets live under `Engine/Shaders/Public` where possible; `Private` pulls use quoted paths from `Engine/` when needed (see reference).
- [ ] File stem matches `module` name; material shaders follow `*_Mat.slang` convention where applicable.
- [ ] Incremental behavior is preserved (timestamp cache + content compare emit).

## Build Verification

The main build target runs shader binding generation through `NilouCodegen` automatically:

```bash
xmake build NilouEditor
```

Use `xmake codegen` only when you want to manually refresh generated files without building the editor. Use `xmake codegen --force` to bypass codegen caches and regenerate outputs.

## Additional Resources

- Detailed design and mapping: [reference.md](reference.md)
