target("slang")
    set_kind("phony")
    add_includedirs("./include", {public = true})
    add_links("$(projectdir)/Engine/Source/ThirdParty/slang/lib/slang", {public = true})
    on_load(function (target)
        local scriptdir = path.absolute(target:scriptdir())
        local zipfile = path.join(scriptdir, "slang-2026.4.2-windows-x86_64.zip")
        local slanglib = path.join(scriptdir, "lib/slang.lib")
        if os.isfile(zipfile) then
            if not os.isfile(slanglib) then
                import("utils.archive")
                print("Extracting Slang to " .. scriptdir)
                archive.extract(zipfile, scriptdir)
                print("Slang extracted to " .. scriptdir)
            else
                print("Slang already extracted to " .. scriptdir)
            end
        else
            print("Slang zip file not found at " .. zipfile)
        end
    end)
    after_build(function (target)
        os.cp("$(scriptdir)/bin/slang.dll", "$(builddir)/$(os)/$(arch)/$(mode)")
        os.cp("$(scriptdir)/bin/slang-compiler.dll", "$(builddir)/$(os)/$(arch)/$(mode)")
        os.cp("$(scriptdir)/bin/slang-glslang.dll", "$(builddir)/$(os)/$(arch)/$(mode)")
        os.cp("$(scriptdir)/bin/*.pdb", "$(builddir)/$(os)/$(arch)/$(mode)")
    end)