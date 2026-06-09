#pragma once

#include <string>
#include <vector>
#include <memory>

namespace bird {

class ShaderCompiler {
public:
    ShaderCompiler();
    ~ShaderCompiler();

    std::vector<uint32_t> compile_glsl_to_spirv(
        const std::string& glsl_source,
        const std::string& stage
    );

    std::vector<uint32_t> compile_glsl_file_to_spirv(
        const std::string& filepath
    );

    std::string translate_spirv_to_msl(
        const std::vector<uint32_t>& spirv_code
    );

    std::string compile_glsl_to_msl(
        const std::string& glsl_source,
        const std::string& stage
    );

    std::string compile_glsl_file_to_msl(
        const std::string& filepath
    );

    std::string get_last_error() const { return last_error_; }

private:
    std::string last_error_;

    std::string detect_shader_stage(const std::string& filepath) const;
};

} // namespace bird
