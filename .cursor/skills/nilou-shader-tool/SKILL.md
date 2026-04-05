---
name: nilou-shader-tool
description: Handle Nilou Slang shader reflection and generated binding workflows. Use when editing Nilou shader files, ParameterBlock definitions, ShaderReflection generation logic, or when users mention NilouShaderTool, generated.h, gen.cpp, descriptor set layout, Std140/Std430, or opaque shader parameters.
---

# Nilou Shader Tool Workflow

## Quick Start

Use this skill for tasks related to Nilou shader reflection code generation.

1. Confirm whether the target `.slang` file is a module file (`module ...;`).
2. Check whether struct types are actually referenced by `ParameterBlock<T>`.
3. Apply or review generation rules for `Opaque` and `Std140`/`Std430` layouts.
4. Validate generated outputs (`.generated.h` and `.gen.cpp`) and metadata registration.

## Execution Checklist

- [ ] `ParameterBlock<T>` usage is present for all expected generated structs.
- [ ] Resource fields and scalar/vector/matrix fields are split to the correct data layout.
- [ ] `AutomaticallyIntroducedUniformBuffer` exists when opaque and scalar fields coexist.
- [ ] Descriptor bindings and member offsets are consistent with generated metadata.
- [ ] Incremental behavior is preserved (timestamp cache + content compare emit).

## Build Verification

Run in this order:

```bash
xmake build NilouHeaderTool
xmake build NilouShaderTool
xmake build NilouEditor
```

## Additional Resources

- Detailed design and mapping: [reference.md](reference.md)
