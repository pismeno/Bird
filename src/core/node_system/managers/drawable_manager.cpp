#include <node_system/managers/drawable_manager.hpp>

#include <cstdint>

#include <nlohmann/json.hpp>

#include <utils/result.hpp>
#include "glm/vec4.hpp"

namespace mole {

void DrawableManager::on_node_destroyed(NodeID node_id) noexcept {
  drawables[node_id.index()].is_visible = false;
}

void DrawableManager::on_node_require_manager(NodeID node_id) noexcept {
  drawables[node_id.index()].is_visible = true;
}

void DrawableManager::on_scene_clear() noexcept {
  for (auto &drawable: drawables) {
    drawable.is_visible = false;
  }
}

Result<void> DrawableManager::on_serialize_scene(nlohmann::json &json) const {
  json["nodes"] = nlohmann::json::array();

  uint32_t node_idx = 0;

  for (node_idx = 0; node_idx < drawables.size(); ++node_idx) {
    if (!drawables[node_idx].is_visible) {
      continue;
    }

    nlohmann::json drawable_json;

    drawable_json["id"] = node_idx;
    drawable_json["z_index"] = drawables[node_idx].z_index;

    json["nodes"].push_back(drawable_json);
  }

  return mole::ok();
}

Result<void> DrawableManager::on_deserialize_scene(const nlohmann::json &json) {
  if (!json.contains("nodes") || !json["nodes"].is_array()) {
    return mole::ok();
  }

  for (const auto& drawable_json : json["nodes"]) {
    if (!drawable_json.contains("id") || !drawable_json["id"].is_number_integer()) {
      continue;
    }

    uint32_t node_idx = drawable_json["id"].get<uint32_t>();

    if (node_idx >= drawables.size()) {
      continue;
    }


    if (!drawable_json.contains("z_index")) {
      continue;
    }

    drawables[node_idx].z_index = drawable_json["z_index"].get<uint32_t>();
    drawables[node_idx].is_visible = true;
  }

  return mole::ok();
}

Result<void> DrawableManager::set_node_visible(NodeID node_id, bool visible) {
  if (!has_drawable(node_id)) return mole::fail("Node has no drawable");
  drawables[node_id.index()].is_visible = visible;
  return mole::ok();
}

Result<void> DrawableManager::set_node_size(NodeID node_id, glm::vec2 size) {
  if (!has_drawable(node_id)) return mole::fail("Node has no drawable");
  drawables[node_id.index()].size = size;
  return mole::ok();
}

Result<void> DrawableManager::set_node_material(NodeID node_id, MaterialID material_id) {
  if (!has_drawable(node_id)) return mole::fail("Node has no drawable");
  drawables[node_id.index()].material_id = material_id;
  return mole::ok();
}

Result<void> DrawableManager::set_node_z_index(NodeID node_id, uint32_t z_index) {
  if (!has_drawable(node_id)) return mole::fail("Node has no drawable");
  drawables[node_id.index()].z_index = z_index;
  return mole::ok();
}

} // mole