-- ShaderBindings/Generated is produced by NilouShaderTool; skip NilouHeaderTool
-- so its stale-file cleanup does not remove shader-generated .gen.cpp files.
module_rules("ShaderBindings", { skip_header_tool = true })
    add_deps("RHI")
    add_deps("RenderCore")