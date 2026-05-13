#pragma once

#include "transforms.hpp"

#include <vector>

namespace bird::nodes {

class TransformSystem {
 public:
  void ensure_capacity(size_t capacity);
  void add_transform(NodeID node_id);
  bool has_transform(NodeID node_id) const;
  Transform2D* get_transform(NodeID node_id);
  void update_dirty_transforms();

 private:
  // The fast math array (Data)
  std::vector<Transform2D> transforms;

  // The Lookup Table: index is NodeID, value is transform_index
  // Initialize with -1 (meaning no transform)
  std::vector<int> node_to_transform;
};

} // bird::nodes