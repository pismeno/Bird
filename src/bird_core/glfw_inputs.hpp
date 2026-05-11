#include <GLFW/glfw3.h>

namespace bird {

class GlfwInputs {
 public:
  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};

}