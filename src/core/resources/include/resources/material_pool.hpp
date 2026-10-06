#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>

#include <resources/material.hpp>
#include "resources/material_types.hpp"
#include "utils/string_hash.hpp"

namespace bird {

class MaterialPool {
 public:
  const Material& get(MaterialID material_id) const;
  MaterialID get_id(std::string_view string_material_id) const;
  uint32_t get_version(MaterialID material_id) const;
  uint32_t get_version(std::string_view string_material_id) const;
  [[nodiscard]] bool is_valid(MaterialID material_id) const;
  [[nodiscard]] bool is_valid(std::string_view string_material_id) const;
  MaterialID add(std::string_view string_id, Material material);
  void destroy(std::string_view);
  void destroy(MaterialID);

  void set_property(MaterialID material_id, std::string_view property, MaterialProperty value);
  void set_property(std::string_view string_material_id, std::string_view property, MaterialProperty value);
 private:
  std::unordered_map<std::string, MaterialID, StringHash, std::equal_to<>> string_id_lookup;
  std::vector<Material> materials;
  std::vector<uint32_t> generations;
  std::vector<uint32_t> material_versions;

  uint32_t next_unused_index{0};
  std::vector<uint32_t> recycled_indices;
};

} // bird