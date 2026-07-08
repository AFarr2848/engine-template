
#include <iostream>
#include <stdexcept>
#include "engine/Engine.hpp"

int main() {
  try {
    fe_Engine engine{};
    engine.startEngine();
    engine.run();
  } catch (std::runtime_error e) {
    std::cerr << e.what() << std::endl;
  }
}
