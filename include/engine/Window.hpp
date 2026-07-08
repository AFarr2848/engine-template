#pragma once
#include <GLFW/glfw3.h>

class fe_Engine;

class fe_Window {
 public:
  fe_Window() {};

  void init();

  GLFWwindow* window;
  bool framebufferResized = false;

 private:
  void initWindow();
  void pollEvents();
  bool shouldClose() const;
  void mouseMoved(float, float);
  void resetMouse();

  bool firstMouse = true;
  int lastX;
  int lastY;

  static void framebufferResizeCallback(GLFWwindow*, int, int);
  static void GLFWMouseCallback(GLFWwindow*, double, double);
};
