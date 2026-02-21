target("NilouShaderTool")
    set_kind("binary")
    add_files("*.cpp")
    set_policy("build.fence", true)
    set_languages("clatest")
    set_languages("cxx20")
    add_deps("slang")
    if is_mode("debug") then 
        add_defines("NILOU_DEBUG")
    end
    add_includedirs("../../Runtime/RHI/Public")
    add_includedirs("../../Runtime/Core/Public")
    add_includedirs("../../Runtime/CoreUObject/Public")
    add_includedirs("../../ThirdParty/headeronlys")
