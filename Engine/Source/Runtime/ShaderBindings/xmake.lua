module_rules("ShaderBindings", { skip_header_tool = true })
    add_deps("RHI")
    add_deps("RenderCore")
    on_prepare(function (target)
        local shadertool_path = "$(builddir)/$(os)/$(arch)/$(mode)/NilouShaderTool.exe"
        local shader_src_dir = "$(projectdir)/Engine/Shaders"
        local output_header_dir = "$(projectdir)/Engine/Source/Runtime/ShaderBindings/Generated"
        local output_cpp_dir = "$(projectdir)/Engine/Source/Runtime/ShaderBindings/Generated"
        local search_dirs = "$(projectdir)/Engine/Shaders/Public"
        if (os.exists(shadertool_path)) then
            local exec = string.format("%s -InputDirectory=\"%s\" -OutputHeaderDirectory=\"%s\" -OutputCppDirectory=\"%s\" -SearchDirectories=%s", shadertool_path, shader_src_dir, output_header_dir, output_cpp_dir, search_dirs)
            print(exec)
            os.exec(exec)
        end
        target:on_prepare_impl()
    end)