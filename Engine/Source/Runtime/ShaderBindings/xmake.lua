module_rules("ShaderBindings")
    add_deps("RHI")
    add_deps("RenderCore")
    before_prepare(function (target)
        local shadertool_path = "$(builddir)/$(os)/$(arch)/$(mode)/NilouShaderTool.exe"
        local shader_src_dir = "$(projectdir)/Engine/Shaders"
        local generated_dir = "$(projectdir)/Engine/Source/Runtime/ShaderBindings"
        local search_dirs = "$(projectdir)/Engine/Shaders/Public"
        if (os.exists(shadertool_path)) then
            local exec = string.format("%s -InputDirectory=\"%s\" -OutputDirectory=\"%s\" -SearchDirectories=%s", shadertool_path, shader_src_dir, generated_dir, search_dirs)
            print(exec)
            os.exec(exec)
            target:add("files", generated_dir .. "/Private/*.gen.cpp")
        end
    end)