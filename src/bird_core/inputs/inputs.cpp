#include "inputs.hpp"
#include "glfw_inputs.hpp"

namespace bird {

void Inputs::endFrame() {
  GlfwInputs::endFrameKeys();
  GlfwInputs::endFrameMouse();
}

} // bird