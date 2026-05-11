#include "inputs.hpp"
#include "glfw_inputs.hpp"

namespace bird::inputs {

void Inputs::endFrame() {
  GlfwInputs::endFrameKeys();
  GlfwInputs::endFrameMouse();
}

} // bird::inputs