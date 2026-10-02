#pragma once

#include <cstdint>
#include <vector>
#include <string_view>
#include <string>

namespace bird {

/**
 * @brief Base class for all binary assets.
 *
 * All asset structs have to contain TYPE_ID
 */
struct BinaryAsset {
  static constexpr const char* TYPE_ID = "generic_binary_asset";

  alignas(8) std::vector<uint8_t> data;
};

/**
 * @brief Base class for all text assets.
 */
struct TextAsset : public BinaryAsset {
  static constexpr const char* TYPE_ID = "text_asset";

  [[nodiscard]] std::string_view as_string_view() const noexcept;
  [[nodiscard]] std::string as_string() const;
};

/*
// Optional: You can create strongly-typed text assets just like you did with shaders
struct JsonAsset : public TextAsset {
  static constexpr const char* TYPE_ID = "json_asset";
  static constexpr const char* EXTENSION = "json";
};
*/

/**
 * @brief Base class for all shader assets. This is not a struct to be used for actual shader assets.
 */
struct ShaderAsset : public BinaryAsset {
  [[nodiscard]] const uint32_t* as_32bit_words() const noexcept;
};

struct VertexShaderAsset : public ShaderAsset {
  static constexpr const char* TYPE_ID = "vertex_shader_asset";
  static constexpr const char* EXTENSION = "vert";
};

struct FragmentShaderAsset : public ShaderAsset {
  static constexpr const char* TYPE_ID = "fragment_shader_asset";
  static constexpr const char* EXTENSION = "frag";
};

struct ComputeShaderAsset : public ShaderAsset {
  static constexpr const char* TYPE_ID = "compute_shader_asset";
  static constexpr const char* EXTENSION = "comp";
};

struct GeometryShaderAsset : public ShaderAsset {
  static constexpr const char* TYPE_ID = "geometry_shader_asset";
  static constexpr const char* EXTENSION = "geom";
};

struct TessControlShaderAsset : public ShaderAsset {
  static constexpr const char* TYPE_ID = "tess_control_shader_asset";
  static constexpr const char* EXTENSION = "tesc";
};

struct TessEvaluationShaderAsset : public ShaderAsset {
  static constexpr const char* TYPE_ID = "tess_evaluation_shader_asset";
  static constexpr const char* EXTENSION = "tese";
};

}