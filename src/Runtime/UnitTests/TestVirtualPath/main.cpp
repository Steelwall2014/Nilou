#include <filesystem>
#include <map>

using namespace std;

map<string, string> GShaderSourceDirectoryMappings;

void AddShaderSourceDirectoryMapping(const string& VirtualShaderDirectory, const string& RealShaderDirectory)
{
    GShaderSourceDirectoryMappings[VirtualShaderDirectory] = RealShaderDirectory;
}

string GetShaderAbsolutePathFromVirtualPath(const string &VirtualFilePath)
{
    bool RealFilePathFound = false;
    filesystem::path RealFilePath;
    filesystem::path ParentVirtualDirectoryPath = filesystem::path(VirtualFilePath).parent_path();
    filesystem::path RelativeVirtualDirectoryPath = filesystem::path(VirtualFilePath).filename();

    while (!ParentVirtualDirectoryPath.empty() && ParentVirtualDirectoryPath.generic_string() != "/" && ParentVirtualDirectoryPath.generic_string() != "\\")
    {
        if (GShaderSourceDirectoryMappings.count(ParentVirtualDirectoryPath.generic_string()) != 0)
        {
            RealFilePath = 
                filesystem::path(GShaderSourceDirectoryMappings[ParentVirtualDirectoryPath.generic_string()]) / RelativeVirtualDirectoryPath;
            RealFilePathFound = true;
            break;
        }

        RelativeVirtualDirectoryPath = ParentVirtualDirectoryPath.filename() / RelativeVirtualDirectoryPath;
        ParentVirtualDirectoryPath = ParentVirtualDirectoryPath.parent_path();
    }
    return RealFilePath.generic_string();
}


int main()
{
    AddShaderSourceDirectoryMapping("/Project/Assets", "D:\\Nilou\\Assets");
    string a = GetShaderAbsolutePathFromVirtualPath("Assets/Shaders/test.glsl");
    return 0;
}