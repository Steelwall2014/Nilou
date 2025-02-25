add_rules("mode.debug", "mode.release")
set_runtimes("MD")

target("NilouHeaderTool")
    set_kind("binary")
    add_files("*.cpp")
    set_policy("build.fence", true)
    set_languages("clatest")
    set_languages("cxx20")
    if is_mode("debug") then 
        add_defines("NILOU_DEBUG")
    end
    add_includedirs("$(projectdir)/External/include")
    add_linkdirs("$(projectdir)/External/lib")
    add_links("libclang")

-- function ExecuteHeaderTool(target, Configs)
--     local src_dir = "\"" .. path.absolute("./src") .. "\""
--     local generated_dir = "\"" .. path.absolute("./src/Runtime/Generated") .. "\""
--     local include_dir = ""
--     for i, v in ipairs(Configs.INCLUDE_PATHS) do
--         include_dir = include_dir .. " \"" .. v .. "\""
--     end
--     local exec = target:targetfile() .. " " .. src_dir .. " " ..  generated_dir .. " " ..  include_dir
--     print(exec)
--     os.exec(exec)
-- end
