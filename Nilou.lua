import("ExecuteHeaderTool")

function main(target)
    os.cp("./NilouHeaderTool/src/include/*", "./External/include")
    ExecuteHeaderTool(target)
end