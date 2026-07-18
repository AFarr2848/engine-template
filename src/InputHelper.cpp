
#include "engine/InputHelper.hpp"
#include <GLFW/glfw3.h>
#include <iostream>

bool fe_InputHelper::isKeyDown(int key) const {
  if (key >= 0 && key < 512) {
    return keyStates[key];
  }
  return false;
}

// TODO: write me
bool fe_InputHelper::isMouseButtonDown(int button) const {

};

void fe_InputHelper::mouseMoved(GLFWwindow* window, glm::vec2 mousePos) {
  if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
    firstMouse = true;
    return;
  }

  // Makes sure last positions are filled in if it's the first time moving the
  // mouse
  if (firstMouse) {
    lastMousePos = mousePos;
    firstMouse = false;
  }

  currentMousePos = mousePos;
};

void fe_InputHelper::updateInputs() {
  mouseOffset = currentMousePos - lastMousePos;
  lastMousePos = currentMousePos;
}

glm::vec2 fe_InputHelper::getMouseOffsets() const {
  return mouseOffset;
}

void fe_InputHelper::setKeyState(int key, bool isDown) {
  if (key >= 0 && key < 512) {
    keyStates[key] = isDown;
  }
}
