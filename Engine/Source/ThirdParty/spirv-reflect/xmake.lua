target("spirv-reflect")
    set_kind("phony")
    add_includedirs("./include", {public = true})
    add_links("$(projectdir)/Engine/Source/ThirdParty/spirv-reflect/lib/spirv-reflect-static", {public = true})