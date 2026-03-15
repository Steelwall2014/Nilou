target("slang")
    set_kind("phony")
    add_includedirs("./include", {public = true})
    add_links("$(projectdir)/Engine/Source/ThirdParty/slang/lib/slang", {public = true})
    on_load(function (target)
        local zipfile = path.join("$(scriptdir)", "slang-2026.4.2-windows-x86_64.zip")
        local outdir = "$(scriptdir)"
        local slanglib = path.join("$(scriptdir)", "lib/slang.lib")
        if os.isfile(zipfile) and not os.isfile(slanglib) then
            import("utils.archive")
            archive.extract(zipfile, outdir)
        end
    end)
    after_build(function (target)
        os.cp("$(scriptdir)/bin/slang.dll", "$(builddir)/$(os)/$(arch)/$(mode)")
        os.cp("$(scriptdir)/bin/slang-compiler.dll", "$(builddir)/$(os)/$(arch)/$(mode)")
        os.cp("$(scriptdir)/bin/slang-glslang.dll", "$(builddir)/$(os)/$(arch)/$(mode)")
        os.cp("$(scriptdir)/bin/*.pdb", "$(builddir)/$(os)/$(arch)/$(mode)")
    end)