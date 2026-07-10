#include "engine/Window.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include "Config.hpp"

void fe_Window::init() {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
  glfwSetCursorPosCallback(window, GLFWMouseCallback);

  if (!glfwVulkanSupported()) {
    std::cerr << "Vulkan not supported!\n";
  }
}

void fe_Window::resetMouse() {
  firstMouse = true;
}

void fe_Window::framebufferResizeCallback(GLFWwindow* window,
                                          int width,
                                          int height) {
  fe_Window* thisWindow =
      reinterpret_cast<fe_Window*>(glfwGetWindowUserPointer(window));
  thisWindow->framebufferResized = true;
}

void fe_Window::GLFWMouseCallback(GLFWwindow* window,
                                  double xposIn,
                                  double yposIn) {
  fe_Window* thisWindow =
      reinterpret_cast<fe_Window*>(glfwGetWindowUserPointer(window));
  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);

  if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
    thisWindow->lastX = xpos;
    thisWindow->lastY = ypos;
    thisWindow->firstMouse = true;
    return;
  }

  // Makes sure last positions are filled in if it's the first time moving the
  // mouse
  if (thisWindow->firstMouse) {
    thisWindow->lastX = xpos;
    thisWindow->lastY = ypos;
    thisWindow->firstMouse = false;
  }

  float xoffset = xpos - thisWindow->lastX;
  float yoffset = ypos - thisWindow->lastY;

  thisWindow->lastX = xpos;
  thisWindow->lastY = ypos;

  // TODO:: fix this jawn
  // thisWindow->engine->mouseMoved(xoffset, yoffset);
}
