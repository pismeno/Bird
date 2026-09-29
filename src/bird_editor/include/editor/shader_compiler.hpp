#pragma once

#include <string>
#include <vector>
#include <memory>

#include <utils/result.hpp>
#include <resources/asset_manager.hpp>
#include <resources/resources_context.hpp>
#include <editor/editor_context.hpp>

namespace bird {

class ShaderCompiler {
 public:
  /**
   * Compiles a GLSL shader file to SPIR-V file.
   * @param glsl_filepath Path to the GLSL source file.
   * @param spv_output_filepath Path where the compiled SPIR-V file should be saved.
   */
  Result<std::vector<uint32_t>> compile_glsl_to_spirv(const std::string& glsl_filepath, const std::string& spv_output_filepath);

  /**
   * Compiles GLSL source code to SPIR-V code
   * @param glsl_source Glsl source string
   * @param stage The shader stage
   */
  Result<std::vector<uint32_t>> compile_source_to_spirv(const std::string& glsl_source, const std::string& stage);

  Result<std::string> translate_spirv_to_msl(const std::vector<uint32_t>& spirv_code);
  Result<std::string> compile_glsl_to_msl(const std::string& glsl_source, const std::string& stage);

  /**
   * Factory method to create a ShaderCompiler instance.
   */
  static Result<std::unique_ptr<ShaderCompiler>> create(EditorContext& editor_context);
 private:
  ShaderCompiler() = default;

  Result<void> init(EditorContext& editor_context);

  AssetManager* asset_manager;
  IFileSystem* file_system;
};

} // bird