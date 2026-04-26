function module_rules(module_name, options)
    target(module_name)
        set_kind("shared")
        set_languages("clatest")
        set_languages("cxx20")
        add_files("./Private/**.cpp")
        add_includedirs("./Public", {public = true})
        add_includedirs("./Generated", {public = true})
        add_cxflags("/utf-8")
        add_defines("FMT_USE_NONTYPE_TEMPLATE_ARGS=0")
        add_deps("NilouCodegen", {order = true})

        on_load(function (target)
            target.is_nilou_module = true
            target.skip_header_tool = options ~= nil and options.skip_header_tool
            function target:on_prepare_impl()
                import("modules.nilou_codegen").prepare_module(self)
            end
        end)

        on_prepare(function (target)
            target:on_prepare_impl()
        end)
end
