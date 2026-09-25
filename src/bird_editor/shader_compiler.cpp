#include <editor/shader_compiler.hpp>

#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Public/ResourceLimits.h>
#include <spirv_msl.hpp>

#include <resources/asset_manager.hpp>
#include <resources/asset_types.hpp>
#include <resources/asset_handle.hpp>
#include <utils/result.hpp>

namespace bird {

namespace {
Result<EShLanguage> detect_glslang_stage(const std::string& stage) {
    if (stage == "vert" || stage == "vertex") return EShLangVertex;
    if (stage == "frag" || stage == "fragment") return EShLangFragment;
    if (stage == "comp" || stage == "compute") return EShLangCompute;
    if (stage == "geom" || stage == "geometry") return EShLangGeometry;
    if (stage == "tesc" || stage == "tesscontrol") return EShLangTessControl;
    if (stage == "tese" || stage == "tessevaluation") return EShLangTessEvaluation;

    return bird::fail("Unknown shader stage: " + stage);
}

std::string extract_stage(std::string_view filepath) {
    size_t last_dot = filepath.find_last_of('.');
    if (last_dot == std::string_view::npos || last_dot == 0) return {};

    size_t second_last_dot = filepath.find_last_of('.', last_dot - 1);

    size_t last_slash = filepath.find_last_of("/\\");
    if (second_last_dot == std::string_view::npos ||
        (last_slash != std::string_view::npos && second_last_dot < last_slash)) {
        return {};
    }

    return std::string(filepath.substr(second_last_dot + 1, last_dot - second_last_dot - 1));
}
}

Result<std::unique_ptr<ShaderCompiler>> ShaderCompiler::create(EditorContext& editor_context) {
    auto compiler = std::unique_ptr<ShaderCompiler>(new ShaderCompiler());
    auto r_init = compiler->init(editor_context);
    if (!r_init) return bird::fail(r_init.error());
    return std::move(compiler);
}

Result<void> ShaderCompiler::init(EditorContext& editor_context) {
    file_system = editor_context.os_context->file_system;
    asset_manager = editor_context.resources_context->asset_manager;
    return bird::ok();
}

Result<std::vector<uint32_t>> ShaderCompiler::compile_source_to_spirv(const std::string& glsl_source, const std::string& stage) {
    auto r_glslang_stage = detect_glslang_stage(stage);
    if (!r_glslang_stage) return bird::fail(r_glslang_stage.error());

    EShLanguage glslang_stage = r_glslang_stage.value();
    glslang::TShader shader(glslang_stage);

    const char* source_c_str = glsl_source.c_str();
    shader.setStrings(&source_c_str, 1);

    shader.setEnvInput(glslang::EShSourceGlsl, glslang_stage, glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_5);

    const TBuiltInResource* resources = GetDefaultResources();
    if (!shader.parse(resources, 450, false, EShMsgDefault)) {
        return bird::fail(std::string("GLSL compilation failed: ") + shader.getInfoLog());
    }

    glslang::TProgram program;
    program.addShader(&shader);

    if (!program.link(EShMsgDefault)) {
        return bird::fail(std::string("GLSL linking failed: ") + program.getInfoLog());
    }

    std::vector<uint32_t> spirv;
    spv::SpvBuildLogger logger;
    glslang::GlslangToSpv(*program.getIntermediate(glslang_stage), spirv, &logger);

    return spirv;
}

Result<std::vector<uint32_t>> ShaderCompiler::compile_glsl_to_spirv(const std::string& glsl_filepath, const std::string& spv_output_filepath) {
  // Read
  auto r_acquire_glsl = asset_manager->acquire<TextAsset>(glsl_filepath);
  if (!r_acquire_glsl) return bird::fail(r_acquire_glsl.error());
  std::string source_str = r_acquire_glsl.value()->as_string();
  std::string stage = extract_stage(glsl_filepath);

  // Compile
  auto r_spirv = compile_source_to_spirv(source_str, stage);
  if (!r_spirv) return bird::fail(r_spirv.error());

  // Write
  auto& spirv = r_spirv.value(); // Get mutable reference to move from later
  const auto* byte_ptr = reinterpret_cast<const uint8_t*>(spirv.data());
  std::size_t byte_size = spirv.size() * sizeof(uint32_t);

  std::vector<uint8_t> binary(byte_ptr, byte_ptr + byte_size);

  auto r_write_spv = file_system->write_bytes(spv_output_filepath, std::move(binary));
  if (!r_write_spv) return bird::fail(r_write_spv.error());

  // Return the SPIR-V data directly to the caller using std::move
  return std::move(spirv);
}

Result<std::string> ShaderCompiler::translate_spirv_to_msl(const std::vector<uint32_t>& spirv_code) {
    try {
        spirv_cross::CompilerMSL compiler(spirv_code);

        spirv_cross::CompilerMSL::Options options;
        options.platform = spirv_cross::CompilerMSL::Options::macOS;
        compiler.set_msl_options(options);

        return compiler.compile();
    } catch (const std::exception& e) {
        return bird::fail(std::string("SPIR-V to MSL translation error: ") + e.what());
    }
}

Result<std::string> ShaderCompiler::compile_glsl_to_msl(const std::string& glsl_source, const std::string& stage) {
    auto r_spirv = compile_source_to_spirv(glsl_source, stage);
    if (!r_spirv) return bird::fail(r_spirv.error());

    auto r_msl = translate_spirv_to_msl(r_spirv.value());
    if (!r_msl) return bird::fail(r_msl.error());

    return r_msl.value();
}

} // namespace bird