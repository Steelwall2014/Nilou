import("configs")

function main(target)
    Configs = configs()
    local src_dir = "\"" .. path.absolute("./src") .. "\""
    local generated_dir = "\"" .. path.absolute("./src/Runtime/Generated") .. "\""
    local include_dir = ""
    for i, v in ipairs(Configs.INCLUDE_PATHS) do
        include_dir = include_dir .. " \"" .. path.absolute(v) .. "\""
    end
    local exec = "./NilouHeaderTool/build/windows/x64/release/NilouHeaderTool.exe " .. src_dir .. " " ..  generated_dir .. include_dir
    print(exec)
    os.exec(exec)
end