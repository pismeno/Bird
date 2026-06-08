#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <chrono> // Added for profiling

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "windowing/iwindow.hpp"
#include "node_system/scene.hpp"
#include "node_system/node.hpp"
#include "node_system/managers/transform_manager.hpp"
#include "node_system/managers/drawable_manager.hpp"
#include "node_system/render_gatherer.hpp"
#include "rendering/render_command.hpp"
#include "node_system/scene_factory.hpp"

void debug_print_commands(const std::vector<RenderCommand>& commands) {
  std::cout << "--- Frame Render Commands (" << commands.size() << ") ---\n";

  size_t count = std::min<size_t>(commands.size(), 10);

  for (size_t i = 0; i < count; ++i) {
    const auto& cmd = commands[i];

    std::cout << "[" << i << "] "
              << "TexID: " << cmd.texture_id
              << " | Z: " << cmd.z_index
              << " | mat: (" << glm::to_string(cmd.transform) << ")\n";
  }
  std::cout << "------------------------------------------" << std::endl;
}

int main() {
  using namespace bird;
  using namespace bird::inputs;
  using namespace bird::node_system;
  using namespace bird::node_system::managers;

  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  std::unique_ptr<IWindow> window = IWindow::create(WindowOptions{800, 600, "Bird Editor - CRASH TEST"});
  std::unique_ptr<IWindow> window2 = IWindow::create(WindowOptions{800, 600, "Bird Editor 2"});
  (void)window->init();
  (void)window2->init();

  std::cout << "Hello, Bird Editor Crash Tests!" << std::endl;
  std::cout << "Controls:\n"
            << " [1] Extreme Math (Zero Scale, High Shear)\n"
            << " [2] Rapid Reparenting Ping-Pong\n"
            << " [3] Mass Allocation (10,000 Nodes)\n"
            << " [4] Mass Deletion (Triggers Compaction)\n"
            << " [5] Delete Parent (Orphan Crash Test)\n"
            << " [T] Print Idle Frame Timings\n"
            << " [Space] Default Transform Test\n"
            << " [L-Click] Print Render Commands\n"
            << "------------------------------------------\n";

  SceneFactory scene_factory;

  scene_factory.register_manager<TransformManager>();
  scene_factory.register_manager<DrawableManager>();
  auto load_node_result = scene_factory.load_node_definitions_from("node_system/node_definitions.json");
  if (!load_node_result) {
    std::cerr << load_node_result.error() << std::endl;
    return -1;
  }

  auto load_scenes_result = scene_factory.load_scene_definitions_from("node_system/scene_definitions.json");
  if (!load_scenes_result) {
    std::cerr << load_scenes_result.error() << std::endl;
    return -1;
  }

  std::cout << "Creating scene..." << std::endl;

  auto scene = scene_factory.create_scene("scene_2d");

  std::cout << "Creating nodes..." << std::endl;
  NodeID parID = scene->create_node(INVALID_NODE_ID, "node");
  NodeID childID = scene->create_node(parID, "sprite_2d");
  NodeID childID2 = scene->create_node(childID, "sprite_2d");
  NodeID famID = scene->create_node(INVALID_NODE_ID, "sprite_2d");
  NodeID famChildID = scene->create_node(famID, "sprite_2d");



  std::cout << "Getting managers..." << std::endl;
  auto transform_manager = scene->get_manager<TransformManager>();
  auto drawable_manager = scene->get_manager<DrawableManager>();

  std::cout << "Setting transforms..." << std::endl;
  (void)transform_manager->set_node_position(childID, glm::vec2(100, 100));
  (void)transform_manager->set_node_position(childID2, glm::vec2(100, 100));
  (void)transform_manager->set_node_position(famChildID, glm::vec2(400, 400));
  (void)transform_manager->set_node_position(famID, glm::vec2(400, 400));

  std::cout << "Entering loop..." << std::endl;

  RenderGatherer gatherer;
  std::vector<NodeID> stress_nodes; // Keeps track of dynamically spawned nodes

  while (!window->shouldClose()) {
    window->update();
    window2->update();

    auto& inputs1 = window->getInputs();
    auto& inputs2 = window2->getInputs();

    bool print_frame_timings = false; // Flag to avoid console spam

    // ==========================================
    // CRASH TESTS
    // ==========================================

    if (inputs1.isKeyPressed(KeyCode::T)) {
      std::cout << "\n[Manual Timing Request] Measuring base frame updates..." << std::endl;
      print_frame_timings = true;
    }

    // TEST 1: Extreme Transform Math
    if (inputs1.isKeyPressed(KeyCode::KP1)) {
      std::cout << "\n[Crash Test 1] Applying extreme scales, shears, and rotations..." << std::endl;

      (void)transform_manager->set_node_scale(childID, glm::vec2(0.0f, 0.0f));
      (void)transform_manager->set_node_shear(childID2, glm::vec2(99999.0f, -99999.0f));
      (void)transform_manager->set_node_rotation(famID, 1e10f);

      std::cout << "  -> [LOG] Operations dispatched safely.\n";
      print_frame_timings = true;
    }

    // TEST 2: Rapid Reparenting
    if (inputs1.isKeyPressed(KeyCode::KP2)) {
      std::cout << "\n[Crash Test 2] Rapidly swapping parents..." << std::endl;

      auto start = std::chrono::high_resolution_clock::now();
      (void)transform_manager->set_parent(childID2, famChildID);
      (void)transform_manager->set_parent(childID2, parID);
      (void)transform_manager->set_parent(childID2, famID);
      (void)transform_manager->set_parent(childID2, childID); // Return to original
      auto end = std::chrono::high_resolution_clock::now();

      std::chrono::duration<double, std::milli> ms = end - start;

      std::cout << "  -> [TIME] 4 Reparenting ops took: " << ms.count() << " ms\n";
      print_frame_timings = true;
    }

    // TEST 3: Mass Allocation Stress
    if (inputs1.isKeyPressed(KeyCode::KP3)) {
      std::cout << "\n[Crash Test 3] Spawning 10,000 drawables..." << std::endl;
      size_t starting_size = stress_nodes.size();

      auto start = std::chrono::high_resolution_clock::now();
      for (int i = 0; i < 10000; ++i) {
        NodeID new_node = scene->create_node(INVALID_NODE_ID, "sprite_2d");
        stress_nodes.push_back(new_node);

        (void)transform_manager->set_node_position(new_node, glm::vec2(i * 0.1f, i * 0.1f));
        (void)transform_manager->set_node_shear(new_node, glm::vec2(1.5f, -0.5f));
      }
      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> ms = end - start;

      std::cout << "  -> [TIME] Spawning/Positioning took: " << ms.count() << " ms\n";
      std::cout << "  -> [LOG] Total tracked stress nodes: " << stress_nodes.size() << "\n";
      print_frame_timings = true;
    }

    // TEST 4: Mass Deletion & Compaction
    if (inputs1.isKeyPressed(KeyCode::KP4)) {
      std::cout << "\n[Crash Test 4] Destroying mass nodes..." << std::endl;

      size_t successful_destructions = 0;

      auto start = std::chrono::high_resolution_clock::now();
      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> ms = end - start;

      std::cout << "  -> [TIME] Dispatching " << successful_destructions << " destroys took: " << ms.count() << " ms\n";
      stress_nodes.clear();
      print_frame_timings = true;
    }

    // TEST 5: Orphan/Ghost Reference Testing
    if (inputs1.isKeyPressed(KeyCode::KP5)) {
      std::cout << "\n[Crash Test 5] Destroying famID. Checking how famChildID handles orphan status..." << std::endl;
      scene->destroy_node(famID);
      print_frame_timings = true;
    }

    // ==========================================
    // STANDARD INPUTS
    // ==========================================

    if (inputs1.isKeyPressed(KeyCode::Space)) {
      (void)transform_manager->set_node_position(famID, glm::vec2(50, 50));
      (void)transform_manager->set_node_shear(famID, glm::vec2(1.0f, 0.0f));
    }

    if (inputs2.isKeyPressed(KeyCode::Space)) {
      (void)transform_manager->set_node_position(childID, glm::vec2(200, 100));
    }

    if (inputs1.isMouseButtonReleased(MouseCode::Left)) {
      auto start = std::chrono::high_resolution_clock::now();
      auto commands = gatherer.gather_render_commands(*scene);
      auto end = std::chrono::high_resolution_clock::now();

      std::chrono::duration<double, std::milli> ms = end - start;
      std::cout << "\n[Render Gatherer] Took " << ms.count() << " ms to gather " << commands.size() << " commands.\n";

      debug_print_commands(commands);
    }

    if (inputs1.isKeyPressed(KeyCode::F11)) {
      (void)window->setFullscreen(!window->isFullscreen());
    }

    // ==========================================
    // SYSTEM UPDATES & TIMING
    // ==========================================

    // Time the update (Matrix calculations happen here)
    auto update_start = std::chrono::high_resolution_clock::now();
    scene->update();
    auto update_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> update_ms = update_end - update_start;

    inputs1.endFrame();
    inputs2.endFrame();

    // Time the end_frame (Compaction happens here)
    auto end_frame_start = std::chrono::high_resolution_clock::now();
    scene->end_frame();
    auto end_frame_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> end_frame_ms = end_frame_end - end_frame_start;

    // Output the frame timings if a test was triggered
    if (print_frame_timings) {
      std::cout << "  -> [TIME] scene->update() (Transforms recalculated): " << update_ms.count() << " ms\n";
      std::cout << "  -> [TIME] scene->end_frame() (Array compaction): " << end_frame_ms.count() << " ms\n";
      std::cout << "------------------------------------------\n";
    }
  }

  (void)window->close();
  (void)scene_factory.save_scene_to(*scene, "scene_2d3.json");

  glfwTerminate();
  return 0;
}