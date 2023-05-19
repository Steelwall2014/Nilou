Include_paths = {
    "D:/Nilou/External/include", 
    "D:/Nilou/src/Runtime/Framework", 
    "D:/Nilou/src/Runtime/Applications", 
    "D:/Nilou/src/Runtime/RHI", 
    "D:/Nilou/src/Runtime/HAL", 
    "D:/Nilou/src/Runtime/Rendering", 
    "D:/Nilou/src/Runtime/GameStatics",
    "D:/Nilou/src/Runtime/Generated",
    "D:/Nilou/src/Runtime/Serialization",
    "D:/Nilou/src/Runtime/RenderPass",
    "D:/Nilou/src/Runtime/Geospatial",
    "D:/Nilou/src/Runtime/Cesium3DTiles",
    "D:/Nilou/Assets/Shaders"}

function ExecuteHeaderTool()
    local exec_cmd = "D:/Nilou/NilouHeaderTool/build/windows/x64/release/NilouHeaderTool.exe D:/Nilou/src D:/Nilou/src/Runtime/Generated "
    for i, include in ipairs(Include_paths) do
        exec_cmd = exec_cmd .. include .. " "
    end
    os.execute(exec_cmd)
    --D:\Nilou\build\windows\x64\debug\HeaderTool.exe D:\Nilou\src D:\Nilou\src\Runtime\Generated--
end

ExecuteHeaderTool()