#include "node_2d.hpp"

#include "../scene.hpp"

namespace bird::node_system {

void Node2D::on_enter_scene(Scene& scene) {
  scene.get_transform_system().add_transform(this->getId());
}

} // bird::node_system