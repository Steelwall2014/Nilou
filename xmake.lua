add_rules("mode.release", "mode.debug")
add_rules("plugin.vsxmake.autoupdate")
--add_requires("vcpkg::gdal", {alias = "gdal"})
-- target("crossguid")
--     set_runtimes("MDd")
--     set_languages("clatest")
--     set_languages("cxx17")
--     set_kind("static")
--     add_includedirs("./External/include")
--     add_files("External/include/crossguid/*.cpp")
-- target("dds")
--     set_runtimes("MDd")
--     set_languages("clatest")
--     set_languages("cxx17")
--     set_kind("static")
--     add_includedirs("./External/include")
--     add_files("External/include/dds/*.cpp")
-- target("glad")
--     set_runtimes("MDd")
--     set_languages("clatest")
--     set_languages("cxx17")
--     set_kind("static")
--     add_includedirs("./External/include")
--     add_files("External/include/glad/*.c")
-- target("imgui")
--     set_runtimes("MDd")
--     set_languages("clatest")
--     set_languages("cxx17")
--     set_kind("static")
--     add_includedirs("./External/include")
--     add_files("External/include/imgui/*.cpp")
-- target("tinygltf")
--     set_languages("clatest")
--     set_languages("cxx17")
--     set_kind("static")
--     add_includedirs("./External/include")
--     add_files("External/include/tinygltf/*.cpp")


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

function BuildExternalProject(config) 
    target(config.projectName)
    Execute(config.macros, add_defines)
    Execute(config.link, add_links)
    if is_mode("release") then
        set_runtimes("MD")
    else 
        set_runtimes("MDd")
    end 
    set_languages("clatest")
    set_languages("cxx17")
    set_kind("static")
    add_includedirs("./External/include")
    add_files("External/include/" .. config.projectName .. "/*.cpp")
    add_files("External/include/" .. config.projectName .. "/*.c")
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
    set_languages("cxx17")
    projectType = config.projectType
    if projectType ~= nil then
        set_kind(projectType)
    end
    Execute(config.macros, add_defines)
    Execute(config.files, add_files)
    Execute(config.includePaths, add_includedirs)
    Execute(config.depends, add_deps)
    Execute(config.link, add_links)

    if is_mode("release") then
        add_defines("_RELEASE")
        Execute(config.releaseLink, add_links)
        set_optimize("fastest")
        if is_plat("windows") then
            set_runtimes("MD")
            add_cxflags(
                --"/NODEFAULTLIB:libc.lib", "/NODEFAULTLIB:libcmt.lib", "/NODEFAULTLIB:libcd.lib", "/NODEFAULTLIB:libcmtd.lib", "/NODEFAULTLIB:msvcrtd.lib", 
                "/Zi", "/W0", "/MP", "/Ob2", "/Oi", "/Ot", "/Oy", "/GT", "/GF", "/GS-", "/Gy", "/arch:AVX2",
                "/fp:precise", "/Gr", "/TP", {
                    force = true
                })
            SetException(config)
        end
    else
        add_defines("_DEBUG")
        Execute(config.debugLink, add_links)
        set_optimize("none")
        if is_plat("windows") then
            set_runtimes("MDd")
            add_cxflags(
                --"/NODEFAULTLIB:libc.lib", "/NODEFAULTLIB:libcmt.lib", "/NODEFAULTLIB:libcd.lib", "/NODEFAULTLIB:libcmtd.lib", "/NODEFAULTLIB:msvcrt.lib", 
                "/Zi", "/W0", "/MP", "/Ob0", "/Oy-", "/GF", "/GS", "/arch:AVX2", "/fp:precise", "/Gr", "/TP", {
                force = true
            })
            SetException(config)
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

function ExecuteHeaderTool(target)
    if is_mode("release") then
        build_path = "$(buildir)/windows/x64/release/"
    else
        build_path = "$(buildir)/windows/x64/debug/"
    end
    os.exec(build_path .. "HeaderTool ./src ./src/Runtime/Generated")
end

function copyFunc(target)
    if is_mode("release") then
        build_path = "$(buildir)/windows/x64/release/"
    else
        build_path = "$(buildir)/windows/x64/debug/"
    end
    os.cp("./External/shared/*.*", build_path)
end

--[[
function copyFunc(target)
    if is_plat("windows") then
        build_path = ""
        src_path = ""
        if is_mode("release") then
            src_path = "third_party/binary/release/"
            build_path = "$(buildir)/windows/x64/release/"
        else
            src_path = "third_party/binary/debug/"
            build_path = "$(buildir)/windows/x64/debug/"
        end
        os.cp(src_path .. "*.*", build_path)
        os.cp("glfw/dll/glfw3.dll", build_path .. "glfw3.dll")
    end
end
]]--
BuildExternalProject({projectName = "crossguid", macros = {"GUID_WINDOWS"}, link = {"Ole32"}})
BuildExternalProject({projectName = "dds"})
BuildExternalProject({projectName = "glad"})
BuildExternalProject({projectName = "imgui"})

include_paths = {
        "./External/include", 
        "./src/Runtime/Framework", 
        "./src/Runtime/Applications", 
        "./src/Runtime/RHI", 
        "./src/Runtime/HAL", 
        "./src/Runtime/Rendering", 
        "./src/Runtime/GameStatics",
        "./src/Runtime/Generated",
        "./src/Runtime/Serialization",
        "./src/Runtime/RenderPass",
        "./Assets/Shaders", 
        "./External/include/GDAL"}

BuildProject({
    projectName = "Nilou",
    projectType = "binary",
    macros = {},
    depends = {"crossguid", "dds", "glad", "imgui"},
    files = {"src/Runtime/**.cpp|UnitTests/**.cpp"},
    includePaths = include_paths,
    debugLink = {"lib/debug/*"},
    releaseLink = {"lib/release/*"},
    link = {"./External/lib/*", "kernel32", "User32", "Gdi32", "Shell32", "Opengl32"},
    -- beforeBuildFunc = ExecuteHeaderTool,
    afterBuildFunc = copyFunc,
    enableException = true,
    --unityBuildBatch = 8
})

BuildProject({
    projectName = "GltfMaterialImporter",
    projectType = "binary",
    files = {"src/GltfMaterialImporter/**.cpp"},
    includePaths = include_paths,
    debugLink = {"lib/debug/*"},
    releaseLink = {"lib/release/*"},
    link = {"./External/lib/*", "kernel32", "User32", "Gdi32", "Shell32", "Opengl32"},
    -- beforeBuildFunc = ExecuteHeaderTool,
    afterBuildFunc = copyFunc,
    enableException = true,
    --unityBuildBatch = 8
})


BuildProject({
    projectName = "TestUniformProject",
    projectType = "binary",
    macros = {"WORLD_UP_Z_FORWARD_X_RIGHT_Y"},
    depends = {"crossguid", "dds", "glad", "imgui"},
    files = {
        "src/Runtime/Framework/Common/UniformBuffer.cpp", 
        "src/Runtime/Rendering/RenderResource.cpp", 
        "src/Runtime/Framework/Common/AssertionMacros.cpp",
        "src/Runtime/UnitTests/TestUniformBuffer/main.cpp"},
    includePaths = include_paths,
    debugLink = {"lib/debug/*"},
    releaseLink = {"lib/release/*"},
    link = {"./External/lib/*", "kernel32", "User32", "Gdi32", "Shell32", "Opengl32"},
    beforeBuildFunc = ExecuteHeaderTool,
    afterBuildFunc = copyFunc,
    enableException = true,
    --unityBuildBatch = 8
})

BuildProject({
    projectName = "HeaderTool",
    projectType = "binary",
    files = {
        "src/HeaderTool/**.cpp",
        "./src/Runtime/GameStatics/**.cpp",},
    includePaths = {
        "./src/HeaderTool/",
        "./src/Runtime/GameStatics/"},
    enableException = true,
    -- beforeBuildFunc = ExecuteHeaderTool,
    --unityBuildBatch = 8
})

BuildProject({
    projectName = "TestTransformProject",
    projectType = "binary",
    files = {
        "src/Runtime/Framework/Common/Transform.cpp", 
        "src/Runtime/UnitTests/TestTransform/main.cpp"},
    includePaths = include_paths,
    beforeBuildFunc = ExecuteHeaderTool,
    enableException = true,
    --unityBuildBatch = 8
})

BuildProject({
    projectName = "TestPermutation",
    projectType = "binary",
    files = {
        "src/Runtime/UnitTests/TestPermutation/main.cpp"},
    enableException = true,
    --unityBuildBatch = 8
})

BuildProject({
    projectName = "TestVirtualPath",
    projectType = "binary",
    files = {
        "src/Runtime/UnitTests/TestVirtualPath/main.cpp"},
    enableException = true,
    --unityBuildBatch = 8
})

BuildProject({
    projectName = "TestShadInclude",
    projectType = "binary",
    files = {
        "src/Runtime/UnitTests/TestShadInclude/main.cpp",
        "./src/Runtime/GameStatics/**.cpp",},
    includePaths = include_paths,
    enableException = true,
    --unityBuildBatch = 8
})

BuildProject({
    projectName = "TestRegex",
    projectType = "binary",
    files = {
        "src/Runtime/UnitTests/TestRegex/main.cpp"},
    enableException = true,
    includePaths = include_paths,
    --unityBuildBatch = 8
})

BuildProject({
    projectName = "TestShaderPreprocess",
    projectType = "binary",
    files = {
        "src/Runtime/UnitTests/TestShaderPreprocess/main.cpp"},
    enableException = true,
    --unityBuildBatch = 8
})
--
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro defination
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--
