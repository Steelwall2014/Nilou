function module_rules(module_name, kind)
    if kind == nil then
        kind = "static"
    end
    target(module_name)
        set_kind(kind)
        set_languages("clatest")
        set_languages("cxx20")
        add_files("./Private/**.cpp")
        add_includedirs("./Public", {public = true})
        add_includedirs("$(projectdir)/External/include")
        add_cxflags("/utf-8")
        add_defines("FMT_USE_NONTYPE_TEMPLATE_ARGS=0")

        on_prepare(function (target)
            function get_includedirs_recursive(target)
                local deps = target:get("deps")
                local includedirs = target:get("includedirs")
                for i, dep in ipairs(deps) do
                    local dep_includedirs = get_includedirs_recursive(target:dep(dep))
                    for j, dep_includedir in ipairs(dep_includedirs) do
                        table.insert(includedirs, dep_includedir)
                    end
                end
                return includedirs
            end
            local includedirs = get_includedirs_recursive(target)
            local src_dir = path.absolute(target:scriptdir())
            local generated_dir = path.absolute(target:scriptdir() .. "/Generated")
            local include_dir = ""
            for i, v in ipairs(includedirs) do
                include_dir = include_dir .. string.format(" -I \"%s\"", path.translate(path.absolute(v)))
            end
            local headertool_path = "$(builddir)/$(os)/$(arch)/$(mode)/NilouHeaderTool.exe"
            if (os.exists(headertool_path)) then
                local exec = string.format("%s -InputDirectory=\"%s\" -OutputDirectory=\"%s\" -x c++ -std=c++20 %s", headertool_path, src_dir, generated_dir, include_dir)
                print(exec)
                os.exec(exec)
                target:add("files", generated_dir .. "/*.gen.cpp")
            else
                print("NilouHeaderTool not found in " .. headertool_path .. ". Please build NilouHeaderTool first (xmake build NilouHeaderTool).")
            end
        end)
end
