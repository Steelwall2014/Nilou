set_xmakever("2.9.1")
set_project("Nilou")

engine_version = "0.1.0"
add_rules("plugin.compile_commands.autoupdate", { outputdir = ".vscode" })

includes("configs.lua")
local Configs = GetConfigs()
add_rules("mode.release", "mode.debug")
set_runtimes("MD")
if (is_os("windows")) then 
    add_defines("_CRT_SECURE_NO_WARNINGS")
end
-- TODO use xmake package manager
-- add_requires("vcpkg::gdal", {configs = {shared = true}})
-- add_requires("vcpkg::glfw3")
add_requires("glfw")
-- add_requires("imgui[glfw-binding,opengl3-binding,vulkan-binding]", { alias = "imgui" })
add_requires("imgui", {configs = {glfw = true, opengl3 = true, vulkan = true}})
-- add_requires("draco")
add_requires("fmt")
add_requires("spirv-reflect")
-- add_requires("vcpkg::llvm")
add_requireconfs("*", {external = false})

target("Nilou")
    set_languages("clatest")
    set_languages("cxx20")
    set_kind("binary")

    before_build(function (target)
        os.cp("$(projectdir)/External/bin/*", "$(buildir)/$(os)/$(arch)/$(mode)")
        local includedirs = target:get("includedirs")
        local src_dir = path.absolute("src")
        local generated_dir = path.absolute("src/Runtime/Generated")
        local include_dir = ""
        for i, v in ipairs(includedirs) do
            include_dir = include_dir .. string.format(" \"%s\"", path.absolute(v))
        end
        local exec = string.format("$(buildir)/$(os)/$(arch)/$(mode)/NilouHeaderTool.exe \"%s\" \"%s\" %s", src_dir, generated_dir, include_dir)
        print(exec)
        os.exec(exec)
    end)

    add_packages(
        "glfw",
        "imgui",
        -- "draco",
        "fmt",
        "spirv-reflect"
    )

    add_deps(
        "crossguid",
        "glad",
        "base64",
        "VulkanSDK",
        "NilouHeaderTool"
    )

    add_defines([[PROJECT_DIR=R"($(projectdir))"]])
    add_defines("FMT_USE_NONTYPE_TEMPLATE_ARGS=0")
    add_includedirs(
        "External/include", 
        "src/Runtime/Framework", 
        "src/Runtime/Applications", 
        "src/Runtime/RHI", 
        "src/Runtime/HAL", 
        "src/Runtime/Rendering", 
        "src/Runtime/GameStatics",
        "src/Runtime/Generated",
        "src/Runtime/Serialization",
        "src/Runtime/RenderPass",
        "src/Runtime/Geospatial",
        "src/Runtime/Cesium3DTiles",
        "src/Runtime/RenderGraph"
    )
    add_files("src/Runtime/**.cpp")
    add_cxflags("/bigobj")

    add_links(
        "kernel32", 
        "User32", 
        "Gdi32", 
        "Shell32", 
        "Opengl32", 
        "External/lib/*"
    )

function Execute(map, func)
    if map ~= nil then
        for i, v in ipairs(map) do
            func(v)
        end
    end
end
function SetException(config)
    enableException = config.enableException
    if (enableException ~= nil) and (enableException) then
        add_cxflags("/EHsc", {
            force = true
        })
    end
end

function BuildProject(config)
    target(config.projectName)

    if config.afterBuildFunc ~= nil then
        after_build(config.afterBuildFunc)
    end

    if config.beforeBuildFunc ~= nil then
        before_build(config.beforeBuildFunc)
    end

    set_languages("clatest")
    set_languages("cxx20")
    projectType = config.projectType
    if projectType ~= nil then
        set_kind(projectType)
    end
    Execute(config.macros, add_defines)
    Execute(config.files, add_files)
    Execute(config.includePaths, add_includedirs)
    Execute(config.depends, add_deps)
    Execute(config.link, add_links)
    Execute(config.package, add_packages)
    if config.runargs ~= nil then
        set_runargs(unpack(config.runargs))
    end 
    -- if runargs ~= nil then
    --     for i, v in ipairs(runargs) do
    --         set_runargs(v)
    --     end
    -- end

    if config.enableException then 
        set_exceptions("cxx")
    else 
        set_exceptions("no-cxx")
    end

    -- set_runtimes("MD")
    if is_mode("release") then
        add_defines("NILOU_RELEASE")
        Execute(config.releaseLink, add_links)
        set_optimize("fastest")
        if is_plat("windows") then
            add_cxflags(
                --"/NODEFAULTLIB:libc.lib", "/NODEFAULTLIB:libcmt.lib", "/NODEFAULTLIB:libcd.lib", "/NODEFAULTLIB:libcmtd.lib", "/NODEFAULTLIB:msvcrtd.lib", 
                "/Zi", "/W0", "/MP", "/Ob2", "/Oi", "/Ot", "/Oy", "/GT", "/GF", "/GS-", "/Gy", "/arch:AVX2",
                "/fp:precise", "/Gr", "/TP", {
                    force = true
                })
            -- SetException(config)
        end
    else
        add_defines("NILOU_DEBUG")
        Execute(config.debugLink, add_links)
        set_optimize("none")
        if is_plat("windows") then
            -- set_runtimes("MDd")
            add_cxflags(
                --"/NODEFAULTLIB:libc.lib", "/NODEFAULTLIB:libcmt.lib", "/NODEFAULTLIB:libcd.lib", "/NODEFAULTLIB:libcmtd.lib", "/NODEFAULTLIB:msvcrt.lib", 
                "/Zi", "/W0", "/MP", "/Ob0", "/Oy-", "/GF", "/GS", "/arch:AVX2", "/fp:precise", "/Gr", "/TP", "/bigobj", {
                force = true
            })
            -- SetException(config)
        end
    end
    
    unityBuildBatch = config.unityBuildBatch
    if (unityBuildBatch ~= nil) and (unityBuildBatch > 0) then
        add_rules("c.unity_build", {
            batchsize = unityBuildBatchSize
        })
        add_rules("c++.unity_build", {
            batchsize = unityBuildBatchSize
        })
    end
end

function copyFunc(target)
    if is_mode("release") then
        build_path = "$(buildir)/windows/x64/release/"
    else
        build_path = "$(buildir)/windows/x64/debug/"
    end
    os.cp("./External/shared/*.*", build_path)
end

-- BuildProject({
--     projectName = "Nilou",
--     projectType = "binary",
--     macros = {"FMT_USE_NONTYPE_TEMPLATE_ARGS=0"},
--     depends = {"crossguid", "glad", "base64", "NilouHeaderTool"},
--     files = {"src/Runtime/**.cpp|UnitTests/**.cpp", "E:/VulkanSDK/1.3.246.1/Source/SPIRV-Reflect/spirv_reflect.c"},
--     debugLink = {"lib/debug/*"},
--     releaseLink = {"lib/release/*"},
--     link = {"kernel32", "User32", "Gdi32", "Shell32", "Opengl32", "./External/lib/*", Configs.VULKAN_LIBRARY},
--     package = {"vcpkg::gdal", "glfw", "imgui", "vcpkg::draco", "magic_enum", "vcpkg::glslang", "fmt"},
--     -- beforeBuildFunc = "Nilou",
--     includePaths = Configs.INCLUDE_PATHS,
--     -- afterBuildFunc = copyFunc,
--     enableException = true,
--     --unityBuildBatch = 8
-- })

includes("src/misc/xmake.lua")
includes("src/Programs/NilouHeaderTool/xmake.lua")
includes("External/base64/xmake.lua")
includes("External/crossguid/xmake.lua")
includes("External/glad/xmake.lua")
includes("External/vulkan/xmake.lua")
