#include "engine/Window.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include "Config.hpp"
#include "engine/InputHelper.hpp"

void fe_Window::init() {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
  glfwSetWindowUserPointer(window, &inputHelper);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, GLFWMouseCallback);
  glfwSetKeyCallback(window, GLFWKeyCallback);

  if (!glfwVulkanSupported()) {
    std::cerr << "Vulkan not supported!\n";
  }
}

void fe_Window::GLFWKeyCallback(GLFWwindow* win,
                                int key,
                                int scancode,
                                int action,
                                int mods) {
  fe_InputHelper* inputHelper =
      static_cast<fe_InputHelper*>(glfwGetWindowUserPointer(win));

  if (action == GLFW_PRESS) {
    if (key == GLFW_KEY_ESCAPE) {
      if (glfwGetInputMode(win, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      else
        glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    if (key == GLFW_KEY_F11)
      toggleFullscreen(win);
    inputHelper->setKeyState(key, true);
  }
  if (action == GLFW_RELEASE)
    inputHelper->setKeyState(key, false);
}

void fe_Window::GLFWMouseCallback(GLFWwindow* window,
                                  double xposIn,
                                  double yposIn) {
  fe_InputHelper* inputHelper =
      static_cast<fe_InputHelper*>(glfwGetWindowUserPointer(window));

  inputHelper->mouseMoved(window, glm::vec2(xposIn, yposIn));
}

void fe_Window::toggleFullscreen(GLFWwindow* window) {
  static int windowedPosx;
  static int windowedResx;
  static int windowedPosy;
  static int windowedResy;

  if (glfwGetWindowMonitor(window) == NULL) {
    glfwGetWindowPos(window, &windowedPosx, &windowedPosy);
    glfwGetWindowSize(window, &windowedResx, &windowedResy);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();

    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height,
                         mode->refreshRate);
  } else {
    glfwSetWindowMonitor(window, NULL, windowedPosx, windowedPosy, windowedResx,
                         windowedResy, GLFW_DONT_CARE);
  }
}
