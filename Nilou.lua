import("configs")

function main(target)
    Configs = configs()
    target:add("links", Configs.VULKAN_LIBRARY)
    target:add("includedirs", Configs.INCLUDE_PATHS)
    os.cp("./NilouHeaderTool/src/include/*", "./External/include")
end