#pragma once

#include <cstdint>
#include <vector>
#include <string_view>
#include <string>

namespace bird {

namespace {
/**
 * @brief Base class for all assets.
 */
struct Asset {
  virtual ~Asset() = default;
};
}

struct ImageAsset : Asset {
  static constexpr const char* TYPE_ID = "image_asset";

  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t channels = 0;

  std::vector<uint8_t> pixel_data;

  [[nodiscard]] const inline size_t get_size_bytes() const noexcept { return pixel_data.size(); };
};

/**
 * @brief Base class for all binary assets.
 *
 * All asset structs have to contain TYPE_ID
 */
struct BinaryAsset : Asset {
  static constexpr const char* TYPE_ID = "generic_binary_asset";

  std::vector<uint8_t> data;

  [[nodiscard]] const inline size_t get_size_bytes() const noexcept { return data.size(); };
};

/**
 * @brief Base class for all text assets.
 */
struct TextAsset : public BinaryAsset {
  static constexpr const char* TYPE_ID = "text_asset";

  [[nodiscard]] std::string_view as_string_view() const noexcept;
  [[nodiscard]] std::string as_string() const;
};

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