# NilouHeaderTool Reference

## Purpose

`NilouHeaderTool` parses reflected C++ headers with libclang and generates `.gen.cpp` registration code for runtime reflection.

## CLI

```text
NilouHeaderTool -InputDirectory=<scan dir> -OutputDirectory=<output dir> [-ForceRegenerate] [clang args...]
```

- `-InputDirectory`: recursively scan `.h`/`.hpp`.
- `-OutputDirectory`: write generated `.gen.cpp`.
- `-ForceRegenerate`: bypass `CachedHeaderModifiedTime.txt` and regenerate reflected outputs.
- Other flags pass through to libclang (for include paths and defines).

## Incremental Behavior

- Cache file: `CachedHeaderModifiedTime.txt`
- Reparse only modified headers.

## Xmake Integration

- Normal editor/game builds do not invoke `NilouHeaderTool` manually from module `on_prepare`.
- `NilouEditor` / `NilouGame` depend on the `NilouCodegen` phony target.
- `NilouCodegen` depends on `NilouHeaderTool` and `NilouShaderTool` inside the same xmake build graph, then runs the generators.
- The `build.fence` policy on `NilouCodegen` prevents generated sources from racing with C++ compilation.
- `xmake codegen` is the manual entry point for refreshing generated files without building the editor.
- `xmake codegen --force` passes `-ForceRegenerate` to `NilouHeaderTool`.

## Reflection Macros

- `NCLASS` (`reflect-class`): reflected class derived from `NObject`.
- `NSTRUCT` (`reflect-struct`): reflected struct.
- `NENUM` (`reflect-enum`): reflected enum.
- `NPROPERTY()` (`reflect-property`): reflected field.
- `NFUNCTION()` (`reflect-method`): reflected method name metadata.
- `GENERATED_BODY()`: inject static reflection helpers.

Macro declarations live in:
- `Engine/Source/Runtime/CoreUObject/Public/NObject/ObjectMacros.h`

## Naming Conventions

- `NCLASS`: type starts with `A`/`N`/`U`.
- `NSTRUCT`: type starts with `F`.
- `NENUM`: type starts with `E`.

## Supported Property Types

- Built-in numeric/boolean/string types.
- GLM math types and project-specific supported math/value types.
- Reflected structs (`NSTRUCT`).
- Reflected class pointers / supported smart pointers to reflected classes.
- Common containers (`TArray`, `TMap`, `TSet`, `std::vector`, `std::map`, etc.) with supported element types.

Unsupported types may still compile syntactically but can produce invalid generated code.

## Generated Code Shapes

- One `.gen.cpp` per reflected type.
- Class registry macros:
  - `BEGIN_CLASS_REGISTRY(...)`
  - `CLASS_PROPERTY(...)`
  - `END_CLASS_REGISTRY(...)`
- Struct registry macros:
  - `BEGIN_STRUCT_REGISTRY(...)`
  - `STRUCT_PROPERTY(...)`
  - `END_STRUCT_REGISTRY(...)`
- Enum registry macros:
  - `BEGIN_ENUM_REGISTRY(...)`
  - `ENUM_VALUE(...)`
  - `END_ENUM_REGISTRY(...)`

If no superclass exists for struct registration, use `NullSuperClass`.

## Runtime Registration Notes

Static registry objects initialize at startup and:
1. Construct `NClass` metadata.
2. Add reflected properties with offsets and property-type metadata.
3. Assign `Z_StaticClass` for reflected runtime lookup.

## Notes

- `GENERATED_BODY()` must be inside class/struct body.
- Built-in root reflection types (`NObject`, `NPackage`, `NClass`) are pre-registered.
- Orphan generated files are removed when source types disappear.
- Modules whose `Generated` directory is owned by another generator must skip HeaderTool; for example, `ShaderBindings/Generated` is owned by `NilouShaderTool`.
- Parsing uses parallel execution with synchronization for shared registries.
