#pragma once
#include <GLFW/glfw3.h>
#include "engine/Engine.hpp"

class fe_Engine;

class fe_Window {
 public:
  fe_Window(fe_InputHelper& inputHelper) : inputHelper(inputHelper) {};

  void init();

  GLFWwindow* window;

 private:
  void initWindow();
  void pollEvents();
  bool shouldClose() const;
  void mouseMoved(float, float);
  void resetMouse();

  static void toggleFullscreen(GLFWwindow* window);

  fe_InputHelper& inputHelper;

  static void GLFWMouseCallback(GLFWwindow*, double, double);
  static void GLFWKeyCallback(GLFWwindow* win,
                              int key,
                              int scancode,
                              int action,
                              int mods);
};
