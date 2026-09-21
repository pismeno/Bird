#pragma once

#include <string>
#include <vector>
#include <memory>

#include <bird_core/utils/result.hpp>

namespace bird {

class ShaderCompiler {
public:
    ShaderCompiler();
    ~ShaderCompiler();

    Result<std::vector<uint32_t>> compile_glsl_to_spirv(const std::string& glsl_source, const std::string& stage);
    Result<std::vector<uint32_t>> compile_glsl_file_to_spirv(const std::string& filepath);
    Result<std::string> translate_spirv_to_msl(const std::vector<uint32_t>& spirv_code);
    Result<std::string> compile_glsl_to_msl(const std::string& glsl_source,const std::string& stage);
    Result<std::string> compile_glsl_file_to_msl(const std::string& filepath);
private:
    Result<std::string> detect_shader_stage(const std::string& filepath) const;
};

} // namespace bird
