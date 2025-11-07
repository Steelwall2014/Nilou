set_xmakever("3.0.0")
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

    on_prepare(function (target)
        local includedirs = target:get("includedirs")
        local src_dir = path.absolute("src")
        local generated_dir = path.absolute("src/Runtime/Generated")
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

    after_build(function (target)
        os.cp("$(projectdir)/External/bin/*", "$(builddir)/$(os)/$(arch)/$(mode)")
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
        "microprofile"
    )

    add_defines([[PROJECT_DIR=R"($(projectdir))"]])
    add_defines("FMT_USE_NONTYPE_TEMPLATE_ARGS=0")
    if (is_mode("debug")) then
        add_defines("NILOU_DEBUG")
    end
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
    add_files("src/Runtime/**.cpp|**.gen.cpp")
    add_cxflags("/bigobj","/EHsc")

    add_links(
        "kernel32", 
        "User32", 
        "Gdi32", 
        "Shell32", 
        "Opengl32", 
        "External/lib/*"
    )

includes("xmake/module_rules.lua")
includes("src/misc/xmake.lua")
includes("Engine/Source/ThirdParty/clang-c/xmake.lua")
includes("Engine/Source/Programs/NilouHeaderTool/xmake.lua")
includes("Engine/Source/ThirdParty/base64/xmake.lua")
includes("Engine/Source/ThirdParty/crossguid/xmake.lua")
includes("Engine/Source/ThirdParty/glad/xmake.lua")
includes("Engine/Source/ThirdParty/vulkan/xmake.lua")
includes("Engine/Source/ThirdParty/microprofile/xmake.lua")
includes("Engine/Source/ThirdParty/stb_image/xmake.lua")
includes("Engine/Source/ThirdParty/tinygltf/xmake.lua")
includes("Engine/Source/ThirdParty/json/xmake.lua")
includes("Engine/Source/ThirdParty/tiny3dtiles/xmake.lua")
includes("Engine/Source/Runtime/Core/xmake.lua")
includes("Engine/Source/Runtime/CoreUObject/xmake.lua")
includes("Engine/Source/Runtime/RHI/xmake.lua")
includes("Engine/Source/Runtime/RenderCore/xmake.lua")
includes("Engine/Source/Runtime/Engine/xmake.lua")
includes("Engine/Source/Runtime/VulkanRHI/xmake.lua")
includes("Engine/Source/Runtime/Launch/xmake.lua")