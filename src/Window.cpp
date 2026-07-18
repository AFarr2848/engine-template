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

void fe_Window::resetMouse() {
  firstMouse = true;
}

void fe_Window::GLFWKeyCallback(GLFWwindow* win,
                                int key,
                                int scancode,
                                int action,
                                int mods) {
  fe_InputHelper* inputHelper =
      static_cast<fe_InputHelper*>(glfwGetWindowUserPointer(win));

  if (action == GLFW_PRESS)
    inputHelper->setKeyState(key, true);
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
