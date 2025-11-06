#pragma once
#include <map>
#include <vector>
#include <string>

namespace nilou {

namespace shader_preprocess {

std::string PreprocessInclude(const std::string& ShaderCode, const std::string& WorkingDirectory, const std::vector<std::string>& IncludeDirectories);

} // namespace shader_preprocess

} // namespace nilou