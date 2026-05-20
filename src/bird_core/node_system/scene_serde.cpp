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
      json["nodes"].push_back(node_json);
    }
  }

  json["managers"] = nlohmann::json::object();

  for (auto& manager : managers_flat_array) {
    auto manager_name = manager->get_name();
    json["managers"][manager_name] = nlohmann::json::object();
    Result result = manager->on_serialize_scene(json["managers"][manager_name]);

    if (!result.success) {
      return result;
    }
  }

  return Result::ok();
}

Result Scene::deserialize(const nlohmann::json &json) {

  clear();

  if (!json.contains("nodes") || !json["nodes"].is_array()) {
    return Result::fail("Invalid scene JSON: missing 'nodes' array");
  }

  for (const auto& node_json : json["nodes"]) {
    if (!node_json.contains("type") || !node_json["type"].is_string()) {
      return Result::fail("Invalid scene JSON: node missing 'type' string");
    }

    const auto& node_type = node_json["type"].get<std::string>();

    if (!node_json.contains("id") || !node_json["id"].is_number_integer()) {
      return Result::fail("Invalid scene JSON: node missing 'id' integer");
    }

    NodeID id = NodeID::from(node_json["id"].get<uint32_t>(), 0);

    Node& out_node = nodes[id.index()];
    out_node.id = id;
    out_node.parentId = INVALID_NODE_ID;

    node_types[id.index()] = node_type;

    uint32_t current_id = next_unused_id.load();
    if (id.index() >= current_id) {
      next_unused_id.store(id.index() + 1);
    }

    for (auto& manager : managers_flat_array) {
      manager->on_node_created(id);
    }

    auto type_it = node_type_definitions.find(node_type);
    if (type_it != node_type_definitions.end()) {
      for (const auto& manager : type_it->second.managers) {
        manager->on_node_require_manager(id);
      }
    }

    if (!node_json.contains("parent_id") || !node_json["parent_id"].is_number_integer()) {
      return Result::fail("Invalid scene JSON: node missing 'parent_id' integer");
    }

    uint32_t raw_parent = node_json["parent_id"].get<uint32_t>();
    NodeID parent_id = (raw_parent == INVALID_NODE_ID.index()) ? INVALID_NODE_ID : NodeID::from(raw_parent, 0);

    if (parent_id != INVALID_NODE_ID && parent_id != id) {
      reparent_node(id, parent_id);
    }
  }

  if (json.contains("managers") && json["managers"].is_object()) {
    for (auto& manager : managers_flat_array) {
      auto manager_name = manager->get_name();
      if (json["managers"].contains(manager_name)) {
        Result result = manager->on_deserialize_scene(json["managers"][manager_name]);

        if (!result.success) {
          return result;
        }
      }
    }
  }

  return Result::ok();
}

} // bird::node_system