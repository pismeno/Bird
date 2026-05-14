#include "node_2d.hpp"

#include "node_system/scene.hpp"
#include "node_system/managers/transform/transform_manager.hpp"

namespace bird::node_system {

using namespace managers;

void Node2D::on_enter_scene(Scene& scene) {
  scene.get_manager<TransformManager>()->add_transform(this->getId());
}

} // bird::node_system