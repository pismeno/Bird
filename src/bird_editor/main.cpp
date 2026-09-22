#include <iostream>

#define GLFW_INCLUDE_NONE
#include "glslang/Public/ShaderLang.h"

#include <editor/editor.hpp>

int main() {
  using namespace bird;

  Editor editor = Editor();

  auto r_init = editor.init();
  if (!r_init) {
    std::cerr << r_init.error() << std::endl;
    return 1;
  }

  editor.make_context_current();

  auto r_run = editor.run();
  if (!r_run) {
    std::cerr << r_run.error() << std::endl;
    return 1;
  }

  auto r_shutdown = editor.shutdown();
  if (!r_shutdown) {
    std::cerr << r_shutdown.error() << std::endl;
  }

  return 0;
}