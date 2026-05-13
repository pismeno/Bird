#pragma once

#include "node.hpp"

namespace bird::nodes {

class Node2D : public Node {
  public:
    Node2D(std::string name) : Node(std::move(name)) {}

    void on_enter_scene(Scene& scene) override;
};

} // bird::nodes