---
name: nilou-header-tool
description: Handle Nilou C++ reflection generation workflows based on NCLASS/NSTRUCT/NENUM/NPROPERTY/NFUNCTION macros. Use when editing reflected C++ headers, ObjectMacros, generated .gen.cpp files, NilouHeaderTool parsing logic, or when users mention libclang reflection, StaticClass injection, or class/property registration.
---

# Nilou Header Tool Workflow

## Quick Start

Use this skill for tasks related to reflection macros and generated `.gen.cpp` files.

1. Ensure reflected types/macros are used in valid positions.
2. Verify property field types are in supported type sets.
3. Review generated registry code shape for class/struct/enum.
4. Check incremental generation behavior and stale file cleanup.

## Execution Checklist

- [ ] `GENERATED_BODY()` is present and placed inside reflected class/struct body.
- [ ] `NCLASS`/`NSTRUCT`/`NENUM` naming conventions are respected.
- [ ] Every `NPROPERTY()` field type is supported (including nested container element types).
- [ ] Generated registry macros match expected runtime metadata registration.
- [ ] Output updates are incremental and no orphan `.gen.cpp` remains.

## Build Verification

The main build target runs header reflection through `NilouCodegen` automatically:

```bash
xmake build NilouEditor
```

Use `xmake codegen` only when you want to manually refresh generated files without building the editor. Use `xmake codegen --force` to bypass codegen caches and regenerate outputs.

## Additional Resources

- Detailed macro semantics and runtime mechanism: [reference.md](reference.md)
