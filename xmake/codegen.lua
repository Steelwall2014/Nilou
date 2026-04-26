target("NilouCodegen")
    set_kind("phony")
    set_policy("build.fence", true)
    add_deps("NilouHeaderTool", "NilouShaderTool", {order = true})

    on_build(function (target)
        import("modules.nilou_codegen").run_generators()
    end)

task("codegen")
    set_menu {
        usage = "xmake codegen",
        description = "Build codegen tools and generate Nilou reflected/shader files."
    }

    on_run(function ()
        import("modules.nilou_codegen").run()
    end)
