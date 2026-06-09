#include "shader_compiler.hpp"

#include <glslang/Public/ShaderLang.h>
#include <glslang/Include/glslang_c_interface.h>
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Public/ResourceLimits.h>
#include <spirv_msl.hpp>

#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

namespace bird {

namespace {
    EShLanguage detect_glslang_stage(const std::string& stage) {
        if (stage == "vert" || stage == "vertex") return EShLangVertex;
        if (stage == "frag" || stage == "fragment") return EShLangFragment;
        if (stage == "comp" || stage == "compute") return EShLangCompute;
        if (stage == "geom" || stage == "geometry") return EShLangGeometry;
        if (stage == "tesc" || stage == "tesscontrol") return EShLangTessControl;
        if (stage == "tese" || stage == "tessevaluation") return EShLangTessEvaluation;
        
        throw std::runtime_error("Unknown shader stage: " + stage);
    }
}

ShaderCompiler::ShaderCompiler() {
    glslang::InitializeProcess();
}

ShaderCompiler::~ShaderCompiler() {
    glslang::FinalizeProcess();
}

std::string ShaderCompiler::detect_shader_stage(const std::string& filepath) const {
    std::filesystem::path path(filepath);
    std::string stem = path.stem().string();

    size_t last_dot = stem.rfind('.');
    if (last_dot != std::string::npos) {
        return stem.substr(last_dot + 1);
    }
    
    std::string ext = path.extension().string();
    if (ext == ".glsl") {
        std::string name_without_glsl = stem;
        size_t dot_pos = name_without_glsl.rfind('.');
        if (dot_pos != std::string::npos) {
            return name_without_glsl.substr(dot_pos + 1);
        }
    }
    
    throw std::runtime_error("Cannot detect shader stage from filename: " + filepath);
}

std::vector<uint32_t> ShaderCompiler::compile_glsl_to_spirv(
    const std::string& glsl_source,
    const std::string& stage
) {
    try {
        EShLanguage glslang_stage = detect_glslang_stage(stage);
        
        glslang::TShader shader(glslang_stage);
        const char* source = glsl_source.c_str();
        shader.setStrings(&source, 1);
      
        shader.setEnvInput(glslang::EShSourceGlsl, glslang_stage, glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
        shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
        shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_5);
        
        const TBuiltInResource* resources = GetDefaultResources();
        if (!shader.parse(resources, 450, false, EShMsgDefault)) {
            last_error_ = shader.getInfoLog();
            throw std::runtime_error("GLSL compilation failed: " + last_error_);
        }
        
        glslang::TProgram program;
        program.addShader(&shader);
        
        if (!program.link(EShMsgDefault)) {
            last_error_ = program.getInfoLog();
            throw std::runtime_error("GLSL linking failed: " + last_error_);
        }
        
        std::vector<uint32_t> spirv;
        spv::SpvBuildLogger logger;
        glslang::GlslangToSpv(*program.getIntermediate(glslang_stage), spirv, &logger);
        
        return spirv;
    } catch (const std::exception& e) {
        last_error_ = std::string("GLSL to SPIR-V compilation error: ") + e.what();
        throw;
    }
}

std::vector<uint32_t> ShaderCompiler::compile_glsl_file_to_spirv(
    const std::string& filepath
) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            last_error_ = "Cannot open shader file: " + filepath;
            throw std::runtime_error(last_error_);
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();
        
        std::string stage = detect_shader_stage(filepath);
        return compile_glsl_to_spirv(source, stage);
    } catch (const std::exception& e) {
        last_error_ = std::string("Failed to compile shader file ") + filepath + ": " + e.what();
        throw;
    }
}

std::string ShaderCompiler::translate_spirv_to_msl(
    const std::vector<uint32_t>& spirv_code
) {
    try {
        spirv_cross::CompilerMSL compiler(spirv_code);
        
        spirv_cross::CompilerMSL::Options options;
        options.platform = spirv_cross::CompilerMSL::Options::macOS;
        compiler.set_msl_options(options);
        
        return compiler.compile();
    } catch (const std::exception& e) {
        last_error_ = std::string("SPIR-V to MSL translation error: ") + e.what();
        throw;
    }
}

std::string ShaderCompiler::compile_glsl_to_msl(
    const std::string& glsl_source,
    const std::string& stage
) {
    try {
        auto spirv = compile_glsl_to_spirv(glsl_source, stage);
        return translate_spirv_to_msl(spirv);
    } catch (const std::exception& e) {
        last_error_ = std::string("GLSL to MSL compilation error: ") + e.what();
        throw;
    }
}

std::string ShaderCompiler::compile_glsl_file_to_msl(
    const std::string& filepath
) {
    try {
        auto spirv = compile_glsl_file_to_spirv(filepath);
        return translate_spirv_to_msl(spirv);
    } catch (const std::exception& e) {
        last_error_ = std::string("Failed to compile shader file ") + filepath + " to MSL: " + e.what();
        throw;
    }
}

} // namespace bird
