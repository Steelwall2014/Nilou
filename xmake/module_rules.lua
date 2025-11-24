function module_rules(module_name)
    target(module_name)
        set_kind("shared")
        set_languages("clatest")
        set_languages("cxx20")
        add_files("./Private/**.cpp")
        add_includedirs("./Public", {public = true})
        add_includedirs("$(projectdir)/Engine/Source/ThirdParty/include")
        add_cxflags("/utf-8")
        add_defines("FMT_USE_NONTYPE_TEMPLATE_ARGS=0")

        on_load(function (target)
            target.is_module = true
            local scriptdir = path.absolute(target:scriptdir())
            target.definitions_file = scriptdir .. "/Generated/Definitions." .. target:name() .. ".h"
            target:add("cxflags", "/FI " .. target.definitions_file, { force = true })
        end)

        on_prepare(function (target)
            local api_export = "DLLEXPORT"
            local api_import = "DLLIMPORT"
            if target:get("kind") == "static" then
                api_export = ""
                api_import = ""
            end
            local new_api_defines = "#pragma once\n"
            new_api_defines = new_api_defines .. "#define " .. target:name():upper() .. "_API " .. api_export .. "\n"
            local dep_names = {}
            for dep_name, dep in pairs(target:deps()) do
                if dep.is_module then
                    table.insert(dep_names, dep_name)
                end
            end
            table.sort(dep_names)
            for _, dep_name in ipairs(dep_names) do
                new_api_defines = new_api_defines .. "#define " .. dep_name:upper() .. "_API " .. api_import .. "\n"
            end

            local old_api_defines = ""
            if os.exists(target.definitions_file) then
                old_api_defines = io.readfile(target.definitions_file)
            end
            if old_api_defines ~= new_api_defines then
                io.writefile(target.definitions_file, new_api_defines)
            end
            
            local includedirs = target:get("includedirs")
            local deps = target:deps()
            for dep_name, dep in pairs(deps) do
                local dep_includedirs = dep:get("includedirs")
                for j, dep_includedir in ipairs(dep_includedirs) do
                    table.insert(includedirs, dep_includedir)
                end
            end
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
