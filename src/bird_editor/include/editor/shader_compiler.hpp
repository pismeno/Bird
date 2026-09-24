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
  ShaderCompiler(EditorContext &context)
      : asset_manager(context.resources_context->asset_manager),
        file_system(context.os_context->file_system) {}

  Result<void> compile_glsl_to_spirv(const std::string& glsl_filepath, const std::string& spv_output_filepath);

  Result<std::vector<uint32_t>> compile_source_to_spirv(const std::string& glsl_source, const std::string& stage);

  Result<std::string> translate_spirv_to_msl(const std::vector<uint32_t>& spirv_code);
  Result<std::string> compile_glsl_to_msl(const std::string& glsl_source, const std::string& stage);

 private:
  AssetManager* asset_manager;
  IFileSystem* file_system;
};

} // bird