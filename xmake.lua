set_xmakever("2.9.1")
set_project("Nilou")

engine_version = "0.1.0"
add_rules("plugin.compile_commands.autoupdate", { outputdir = ".vscode" })

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

includes("src/misc/xmake.lua")
includes("src/Programs/NilouHeaderTool/xmake.lua")
includes("External/base64/xmake.lua")
includes("External/crossguid/xmake.lua")
includes("External/glad/xmake.lua")
includes("External/vulkan/xmake.lua")
