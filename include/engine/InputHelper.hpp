#pragma once
#include <glm/ext/vector_float2.hpp>
class GLFWwindow;

enum Key {
  KEY_LEFT_SHIFT = 340,
  KEY_LEFT_CTRL = 341,
  KEY_SPACE = 32,
  KEY_ESCAPE = 256
};

class fe_InputHelper {
 public:
  bool isKeyDown(int key) const;
  bool isMouseButtonDown(int button) const;

  void mouseMoved(GLFWwindow* window, glm::vec2 mousePos);

  glm::vec2 getMouseOffsets() const;

  void setKeyState(int key, bool isDown);

  /**
   * @brief Call me once per frame
   */
  void updateInputs();

 private:
  bool keyStates[512] = {false};
  glm::vec2 currentMousePos = glm::vec2(0);
  glm::vec2 lastMousePos = glm::vec2(0);
  glm::vec2 mouseOffset = glm::vec2(0);
  bool firstMouse = false;
};
