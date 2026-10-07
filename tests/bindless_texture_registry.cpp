#include <doctest/doctest.h>
#include <unordered_set>

#include "rendering/bindless_texture_registry.hpp"

using namespace bird;

TEST_CASE("BindlessTextureRegistry index assignment") {
  BindlessTextureRegistry registry;

  SUBCASE("Requesting the same texture ID multiple times returns the exact same index") {
    uint64_t texture_a = 1042;
    uint64_t texture_b = 8888;

    uint32_t index_a1 = registry.get_index(texture_a);
    uint32_t index_a2 = registry.get_index(texture_a);
    uint32_t index_a3 = registry.get_index(texture_a);

    uint32_t index_b1 = registry.get_index(texture_b);
    uint32_t index_b2 = registry.get_index(texture_b);

    // Same IDs must return the same index
    CHECK(index_a1 == index_a2);
    CHECK(index_a2 == index_a3);
    CHECK(index_b1 == index_b2);

    // Sanity check: different IDs shouldn't match
    CHECK(index_a1 != index_b1);
  }

  SUBCASE("Different texture IDs are assigned strictly unique indices") {
    std::unordered_set<uint32_t> assigned_indices;
    const int num_unique_textures = 1000;

    for (uint64_t i = 1; i <= num_unique_textures; ++i) {
      // Generate some arbitrary unique texture IDs
      uint64_t texture_id = i * 1337;

      uint32_t index = registry.get_index(texture_id);

      // Ensure this index hasn't been handed out before
      CHECK(assigned_indices.find(index) == assigned_indices.end());

      assigned_indices.insert(index);
    }

    // Final verification that we generated exactly the expected number of unique indices
    CHECK(assigned_indices.size() == num_unique_textures);
  }
}