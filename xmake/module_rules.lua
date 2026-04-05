function module_rules(module_name, options)
    target(module_name)
        set_kind("shared")
        set_languages("clatest")
        set_languages("cxx20")
        add_files("./Private/**.cpp")
        add_includedirs("./Public", {public = true})
        add_includedirs("./Generated", {public = true})
        add_cxflags("/utf-8")
        add_defines("FMT_USE_NONTYPE_TEMPLATE_ARGS=0")

        on_load(function (target)
            target.is_nilou_module = true
            function target:on_prepare_impl()
                local api_export = "DLLEXPORT"
                local api_import = "DLLIMPORT"
                if self:get("kind") == "static" then
                    api_export = ""
                    api_import = ""
                end
                local new_api_defines = "#pragma once\n"
                new_api_defines = new_api_defines .. "#define " .. self:name():upper() .. "_API " .. api_export .. "\n"
                local dep_names = {}
                for dep_name, dep in pairs(self:deps()) do
                    if dep.is_nilou_module then
                        table.insert(dep_names, dep_name)
                    end
                end
                table.sort(dep_names)
                for _, dep_name in ipairs(dep_names) do
                    new_api_defines = new_api_defines .. "#define " .. dep_name:upper() .. "_API " .. api_import .. "\n"
                end
    
                local definitions_file = path.absolute(self:scriptdir()) .. "/Generated/Definitions." .. self:name() .. ".h"
                local old_api_defines = ""
                if os.exists(definitions_file) then
                    old_api_defines = io.readfile(definitions_file)
                end
                if old_api_defines ~= new_api_defines then
                    io.writefile(definitions_file, new_api_defines)
                end
                self:add("cxflags", "/FI " .. definitions_file, { force = true })
                
                local includedirs = self:get("includedirs")
                if type(includedirs) ~= "table" then
                    includedirs = { includedirs }
                end
                for dep_name, dep in pairs(self:deps()) do
                    local dep_includedirs = dep:get("includedirs")
                    for j, dep_includedir in ipairs(dep_includedirs) do
                        table.insert(includedirs, dep_includedir)
                    end
                end
                local src_dir = path.absolute(self:scriptdir())
                local generated_dir = path.absolute(self:scriptdir() .. "/Generated")
                local include_dir = ""
                for i, v in ipairs(includedirs) do
                    include_dir = include_dir .. string.format(" -I \"%s\"", path.translate(path.absolute(v)))
                end
                local force_include_file = "-include \"" .. definitions_file .. "\""
                local headertool_path = "$(builddir)/$(os)/$(arch)/$(mode)/NilouHeaderTool.exe"
                if options == nil or not options.skip_header_tool then
                    if os.exists(headertool_path) then
                        local exec = string.format("%s -InputDirectory=\"%s\" -OutputDirectory=\"%s\" -x c++ -std=c++20 %s %s", headertool_path, src_dir, generated_dir, include_dir, force_include_file)
                        print(exec)
                        os.exec(exec)
                    else
                        print("NilouHeaderTool not found in " .. headertool_path .. ". Please build NilouHeaderTool first (xmake build NilouHeaderTool).")
                    end
                else
                    print("NilouHeaderTool is skipped for module " .. module_name)
                end
                self:add("files", generated_dir .. "/*.gen.cpp")
            end
        end)

        on_prepare(function (target)
            target:on_prepare_impl()
        end)
end
