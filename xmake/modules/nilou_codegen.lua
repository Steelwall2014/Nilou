local codegen_ran = false

function tool_path(tool_name)
    local config = import("core.project.config")

    local builddir = config.builddir() or "build"
    local plat = config.plat() or os.host()
    local arch = config.arch() or os.arch()
    local mode = config.mode() or "debug"
    return path.join(builddir, plat, arch, mode, tool_name .. ".exe")
end

function build_tools()
    os.execv("xmake", {"build", "NilouHeaderTool"})
    os.execv("xmake", {"build", "NilouShaderTool"})
end

function each_real_dep(target, callback, visited)
    visited = visited or {}

    for _, dep_name in ipairs(target:get("deps") or {}) do
        if dep_name ~= "NilouCodegen" and not visited[dep_name] then
            visited[dep_name] = true

            local dep = target:dep(dep_name)
            if dep then
                callback(dep_name, dep)
                each_real_dep(dep, callback, visited)
            end
        end
    end
end

function prepare_module(target)
    local api_export = "DLLEXPORT"
    local api_import = "DLLIMPORT"
    if target:get("kind") == "static" then
        api_export = ""
        api_import = ""
    end

    local new_api_defines = "#pragma once\n"
    new_api_defines = new_api_defines .. "#define " .. target:name():upper() .. "_API " .. api_export .. "\n"

    local dep_names = {}
    each_real_dep(target, function (dep_name, dep)
        if dep.is_nilou_module then
            table.insert(dep_names, dep_name)
        end
    end)
    table.sort(dep_names)

    for _, dep_name in ipairs(dep_names) do
        new_api_defines = new_api_defines .. "#define " .. dep_name:upper() .. "_API " .. api_import .. "\n"
    end

    local generated_dir = path.absolute(path.join(target:scriptdir(), "Generated"))
    local definitions_file = path.join(generated_dir, "Definitions." .. target:name() .. ".h")
    os.mkdir(generated_dir)

    local old_api_defines = ""
    if os.exists(definitions_file) then
        old_api_defines = io.readfile(definitions_file)
    end
    if old_api_defines ~= new_api_defines then
        io.writefile(definitions_file, new_api_defines)
    end

    target:add("cxflags", "/FI " .. definitions_file, {force = true})
    target:add("files", path.join(generated_dir, "*.gen.cpp"))

    return definitions_file, generated_dir
end

function module_include_flags(target)
    local includedirs = target:get("includedirs")
    if type(includedirs) ~= "table" then
        includedirs = {includedirs}
    end

    each_real_dep(target, function (_, dep)
        local dep_includedirs = dep:get("includedirs") or {}
        for _, dep_includedir in ipairs(dep_includedirs) do
            table.insert(includedirs, dep_includedir)
        end
    end)

    local include_flags = ""
    for _, includedir in ipairs(includedirs) do
        if includedir then
            include_flags = include_flags .. string.format(" -I \"%s\"", path.translate(path.absolute(includedir)))
        end
    end
    return include_flags
end

function run_header_tool_for_target(target, opt)
    if not target.is_nilou_module or target.skip_header_tool then
        return
    end

    local definitions_file, generated_dir = prepare_module(target)
    local src_dir = path.absolute(target:scriptdir())
    local include_flags = module_include_flags(target)
    local force_include_file = "-include \"" .. definitions_file .. "\""
    local force_regenerate = ""
    if opt and opt.force then
        force_regenerate = " -ForceRegenerate"
    end

    local exec = string.format(
        "%s -InputDirectory=\"%s\" -OutputDirectory=\"%s\"%s -x c++ -std=c++20 %s %s",
        tool_path("NilouHeaderTool"),
        src_dir,
        generated_dir,
        force_regenerate,
        include_flags,
        force_include_file)

    print(exec)
    os.exec(exec)
end

function run_shader_tool(opt)
    local shader_src_dir = path.join("$(projectdir)", "Engine/Shaders")
    local output_dir = path.join("$(projectdir)", "Engine/Source/Runtime/ShaderBindings/Generated")
    local search_dirs = path.join("$(projectdir)", "Engine/Shaders/Public")
    search_dirs = search_dirs .. ";" .. path.join("$(projectdir)", "Engine")
    os.mkdir(output_dir)

    local force_regenerate = ""
    if opt and opt.force then
        force_regenerate = " -ForceRegenerate"
    end

    local exec = string.format(
        "%s -InputDirectory=\"%s\" -OutputHeaderDirectory=\"%s\" -OutputCppDirectory=\"%s\" -SearchDirectories=%s%s",
        tool_path("NilouShaderTool"),
        shader_src_dir,
        output_dir,
        output_dir,
        search_dirs,
        force_regenerate)

    print(exec)
    os.exec(exec)
end

function run_generators(opt)
    opt = opt or {}

    if codegen_ran and not opt.force then
        return
    end
    codegen_ran = true

    local project = import("core.project.project")

    local target_names = {}
    for target_name, _ in pairs(project.targets()) do
        table.insert(target_names, target_name)
    end
    table.sort(target_names)

    for _, target_name in ipairs(target_names) do
        run_header_tool_for_target(project.target(target_name), opt)
    end

    run_shader_tool(opt)
end

function run(opt)
    build_tools()
    run_generators(opt)
end
