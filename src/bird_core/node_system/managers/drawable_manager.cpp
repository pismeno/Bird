#include "drawable_manager.hpp"

#include <cstdint>

#include <nlohmann/json.hpp>

#include "utils/result.hpp"

namespace bird::node_system::managers {

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

  return bird::ok();
}

Result<void> DrawableManager::on_deserialize_scene(const nlohmann::json &json) {
  if (!json.contains("nodes") || !json["nodes"].is_array()) {
    return bird::ok();
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

  return bird::ok();
}

} // bird::node_system::managers