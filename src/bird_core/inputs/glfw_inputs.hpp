#include "GLFW/glfw3.h"

namespace bird {

/**
 * @brief Class for handling GLFW inputs
 */
class GlfwInputs {
 public:
  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
  static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
  static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

  static void endFrameKeys();
  static void endFrameMouse();
};

}