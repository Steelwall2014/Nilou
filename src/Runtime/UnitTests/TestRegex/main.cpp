#include <regex>
#include <sstream>
#include <string>
#include <memory>
#include <set>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
int main()
{
    std::regex re(R"(^(layout\s+\((.*)\)\s+uniform|uniform)\s+([a-zA-Z_]+\w*)\s*(\{([\s\S]*?)\}|[a-zA-Z_]+\w*)\s*;$)");
    std::string code = R"(
uniform sampler2D TransmittanceLUT;
layout (binding=0) uniform sampler3D ScatteringDensityLUT;
layout (rgba32f, binding=0) uniform image3D DeltaScatteringLUT;
layout (rgba32f, binding=1) uniform image3D MultiScatteringLUT;
layout (std140) uniform ScatteringOrderBlock {
    int scattering_order;
};)";
    std::smatch matches;
    while (std::regex_search(code, matches, re))
    {
        code = matches.suffix();
    }
    // std::regex_search(code, matches, re);
    // std::regex_search(code, matches, re);
    // std::regex_search(code, matches, re);
    // std::unique_ptr<int> a = std::make_unique<int>(1);
    // int *ptr = a.get();

    // std::set<std::unique_ptr<int>> s;
    // s.emplace(ptr);
    // std::unique_ptr<int> b(ptr);
    // s.erase(b);

    // std::regex re("^[ ]*layout[ ]*\\(.*binding[ ]*=[ ]*([0-9].*)\\)[ ]*uniform[ ]+([a-zA-Z_]+\\w*)[ \\n]*\\{([ \\w\\n;]+)\\}.*;[ \\n]*");
    // // std::regex re("^[ ]*layout[ ]*\\(.*binding[ ]*=[ ]*([0-9].*)\\)[ ]*uniform[ ]+([a-zA-Z_]+\\w*)");
    // // std::regex re("^[ ]*#[ ]*include[ ]+[\"<](.*)[\">].*");
    // std::regex re_binding("binding[ ]*=[ ]*[0-9]+");
    // std::regex re_uniformbuffer("^[ ]*(layout[ ]*\\(.*\\)[ ]*uniform[ ]+)([a-zA-Z_]+\\w*)[ \\n]*\\{([ \\w\\n;]+)\\}(.*;)[ \\n]*");
    // std::regex re_uniform1("^[ ]*layout[ ]*\\(.*\\)[ ]*uniform[ ]+([a-zA-Z_]+\\w*)[ ]+([a-zA-Z_]+\\w*)[ ]*;[ ]*\\n");
    // std::regex re_uniform2("^[ ]*uniform[ ]+([a-zA-Z_]+\\w*)[ ]+([a-zA-Z_]+\\w*)[ ]*;[ ]*\\n");
    // std::regex re_binding1(",[ ]*binding[ ]*=[ ]*[0-9]+[ ]*");               // like layout(std140, binding = 0)
    // std::regex re_binding2("[ ]*binding[ ]*=[ ]*[0-9]+,[ ]*");               // like layout(binding = 0, std140)
    // std::regex re_binding3("\\([ ]*binding[ ]*=[ ]*[0-9]+[ ]*\\)");   // like layout(binding = 0)
    // std::string s = 
    // "#version 450\n"
    // "layout(binding = 2) uniform UniformBufferObject2 {\n"
    // "    mat4 model;\n"
    // "} ubo2;\n"
    // "layout(binding = 1) uniform sampler2D texSampler;\n"
    // "layout(binding = 3) uniform UniformBufferObject3 {\n"
    // "    mat4 model;\n"
    // "} ubo3;\n"
    // "void main() {\n"
    // "    outColor = texture(texSampler, fragTexCoord)*inColor;\n"
    // "}\n";
    // // std::smatch matches;
    // // while (std::regex_search(s, matches, re)) {
    // //     s = matches.suffix();
    // // }
    // // std::stringstream stream(s);
    // // s = "";
    // // s = stream.str();
    // std::smatch matches;
    // // std::regex_search(s, matches, re_uniform);
    // s = std::regex_replace(s, re_uniformbuffer, "");
    // s = std::regex_replace(s, re_uniform1, "");
    // s = std::regex_replace(s, re_uniform2, "");
    // s = std::regex_replace(s, re_binding3, "(std140)");
    return 0;
}