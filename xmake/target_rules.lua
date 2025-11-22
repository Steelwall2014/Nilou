function target_rules(target_name, target_type)
    target(target_name)
        set_kind("binary")
        set_languages("clatest")
        set_languages("cxx20")
        add_cxflags("/utf-8")

        before_prepare(function (target)
            if target_type == "Editor" then
                for dep_name, dep in pairs(target:deps()) do
                    if dep.is_module then
                        dep:set("kind", "shared")
                    end
                end
            elseif target_type == "Game" then
                for dep_name, dep in pairs(target:deps()) do
                    if dep.is_module then
                        dep:set("kind", "static")
                    end
                end
            end
        end)

        on_prepare(function (target)
            local dep_names = {}
            for dep_name, dep in pairs(target:deps()) do
                if dep.is_module then
                    table.insert(dep_names, dep_name)
                end
            end
            table.sort(dep_names)
            for _, dep_name in ipairs(dep_names) do
                if target_type == "Editor" then
                    target:add("defines", dep_name:upper() .. "_API=DLLIMPORT")
                elseif target_type == "Game" then
                    target:add("defines", dep_name:upper() .. "_API=")
                end
            end
        end)
end