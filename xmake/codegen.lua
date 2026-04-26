target("NilouCodegen")
    set_kind("phony")
    set_policy("build.fence", true)
    add_deps("NilouHeaderTool", "NilouShaderTool", {order = true})

    on_build(function (target)
        import("modules.nilou_codegen").run_generators()
    end)

task("codegen")
    set_menu {
        usage = "xmake codegen [options]",
        description = "Build codegen tools and generate Nilou reflected/shader files.",
        options = {
            {"f", "force", "k", nil, "Force regeneration by bypassing codegen caches."}
        }
    }

    on_run(function ()
        local option = import("core.base.option")
        import("modules.nilou_codegen").run({force = option.get("force")})
    end)
