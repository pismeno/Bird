#include <doctest/doctest.h>
#include <glm/glm.hpp>

#include "node_system/managers/transform_manager.hpp"
#include "node_system/node_types.hpp"

using namespace bird;

inline void CHECK_TRANSLATION(const glm::mat3x3& mat, float expected_x, float expected_y) {
  CHECK(mat[2][0] == doctest::Approx(expected_x));
  CHECK(mat[2][1] == doctest::Approx(expected_y));
}

inline void CHECK_SCALE(const glm::mat3x3& mat, float expected_x, float expected_y) {
  CHECK(mat[0][0] == doctest::Approx(expected_x));
  CHECK(mat[1][1] == doctest::Approx(expected_y));
}

TEST_CASE("TransformManager - Initialization and Lifecycle") {
  TransformManager tm;
  NodeID node = NodeID::from(1, 0);

  SUBCASE("Nodes start with no transform") {
    CHECK_FALSE(tm.has_transform(node));
  }

  SUBCASE("Requiring manager initializes an identity transform") {
    tm.on_node_require_manager(node);
    CHECK(tm.has_transform(node));

    const auto& mat = tm.get_global_matrix(node);
      CHECK_TRANSLATION(mat, 0.0f, 0.0f);
      CHECK_SCALE(mat, 1.0f, 1.0f);
  }

  SUBCASE("Destroying a node cleans up its transform") {
    tm.on_node_require_manager(node);
    tm.on_node_destroyed(node);
    CHECK_FALSE(tm.has_transform(node));
  }
}

TEST_CASE("TransformManager - Local Transformations") {
  TransformManager tm;
  NodeID node = NodeID::from(2, 1);

  tm.on_node_require_manager(node);

  SUBCASE("Position") {
    REQUIRE(tm.set_node_position(node, glm::vec2(15.5f, -10.0f)));
    tm.on_update(); // Flush/calculate matrices

    CHECK_TRANSLATION(tm.get_global_matrix(node), 15.5f, -10.0f);
  }

  SUBCASE("Scale") {
    REQUIRE(tm.set_node_scale(node, glm::vec2(2.0f, 4.0f)));
    tm.on_update();

    CHECK_SCALE(tm.get_global_matrix(node), 2.0f, 4.0f);
  }
}

TEST_CASE("TransformManager - Spatial Hierarchy") {
  TransformManager tm;

  NodeID parent = NodeID::from(1, 0);
  NodeID child = NodeID::from(2, 0);
  NodeID grandchild = NodeID::from(3, 0);

  tm.on_node_require_manager(parent);
  tm.on_node_require_manager(child);
  tm.on_node_require_manager(grandchild);

  SUBCASE("Translation is inherited additively") {
  REQUIRE(tm.set_node_position(parent, glm::vec2(10.0f, 10.0f)));
  REQUIRE(tm.set_node_position(child, glm::vec2(5.0f, -5.0f)));

  // Link them spatially
  tm.set_parent(child, parent);
  tm.on_update();

  // Parent should be exactly where we put it
  CHECK_TRANSLATION(tm.get_global_matrix(parent), 10.0f, 10.0f);
  // Child's global should be Parent + Child (10+5, 10-5)
  CHECK_TRANSLATION(tm.get_global_matrix(child), 15.0f, 5.0f);
}

SUBCASE("Scale cascades down and scales child translation") {
  REQUIRE(tm.set_node_scale(parent, glm::vec2(2.0f, 2.0f)));
  REQUIRE(tm.set_node_position(child, glm::vec2(10.0f, 0.0f)));

  tm.set_parent(child, parent);
  tm.on_update();

  // The child is 10 units to the right in LOCAL space.
  // Because the parent is scaled by 2, global translation should be 20.
  CHECK_TRANSLATION(tm.get_global_matrix(child), 20.0f, 0.0f);
  CHECK_SCALE(tm.get_global_matrix(child), 2.0f, 2.0f);
  }

  SUBCASE("Deep Hierarchy (Grandchild)") {
  REQUIRE(tm.set_node_position(parent, glm::vec2(100.0f, 0.0f)));
  REQUIRE(tm.set_node_position(child, glm::vec2(50.0f, 0.0f)));
  REQUIRE(tm.set_node_position(grandchild, glm::vec2(25.0f, 0.0f)));

  tm.set_parent(child, parent);
  tm.set_parent(grandchild, child);
  tm.on_update();

  CHECK_TRANSLATION(tm.get_global_matrix(grandchild), 175.0f, 0.0f);
  }
}

TEST_CASE("TransformManager - Reparenting and Detaching") {
  TransformManager tm;
  NodeID old_parent = NodeID::from(1, 0);
  NodeID new_parent = NodeID::from(2, 0);
  NodeID child = NodeID::from(3, 0);

  tm.on_node_require_manager(old_parent);
  tm.on_node_require_manager(new_parent);
  tm.on_node_require_manager(child);

  REQUIRE(tm.set_node_position(old_parent, glm::vec2(10.0f, 10.0f)));
  REQUIRE(tm.set_node_position(new_parent, glm::vec2(50.0f, 50.0f)));
  REQUIRE(tm.set_node_position(child, glm::vec2(0.0f, 0.0f)));

  SUBCASE("Removing a parent resets child to local transform") {
    tm.set_parent(child, old_parent);
    tm.on_update();
    CHECK_TRANSLATION(tm.get_global_matrix(child), 10.0f, 10.0f);

    tm.remove_from_parent(child);
    tm.on_update();
    CHECK_TRANSLATION(tm.get_global_matrix(child), 0.0f, 0.0f); // Back to local 0,0
  }

  SUBCASE("Switching parents updates global transform") {
    tm.set_parent(child, old_parent);
    tm.on_update();

    tm.set_parent(child, new_parent);
    tm.on_update();
    CHECK_TRANSLATION(tm.get_global_matrix(child), 50.0f, 50.0f);
    }
}

TEST_CASE("TransformManager - Scene Clear") {
  TransformManager tm;
  NodeID node = NodeID::from(1, 0);

  tm.on_node_require_manager(node);
  CHECK(tm.has_transform(node));

  tm.on_scene_clear();

  CHECK_FALSE(tm.has_transform(node));
}