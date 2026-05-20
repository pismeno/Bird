#include "scene.hpp"

#include <iostream>

#include <nlohmann/json.hpp>

namespace bird::node_system {

Result Scene::serialize(nlohmann::json &json) const {

  json["nodes"] = nlohmann::json::array(); // creates a blank JSON list on key "nodes"

  for (const auto& node : nodes) {
    if (node.getId() != INVALID_NODE_ID) {
      nlohmann::json node_json;
      node_json["type"] = node_types[node.getId().index()];
      node_json["id"] = node.getId().index();
      node_json["parent_id"] = node.getParentId().index();
      node_json["children_ids"] = nlohmann::json::array();
      for (const auto& child_id : node.getChildrenIds()) {
        node_json["children_ids"].push_back(child_id.index());
      }
      json["nodes"].push_back(node_json);
    }
  }

  return Result::ok();
}

Result Scene::deserialize(const nlohmann::json &json) {
  return Result::ok();
}

} // bird::node_system