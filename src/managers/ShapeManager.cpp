
#include "engine/managers/ShapeManager.hpp"
#include <cstdint>
uint32_t fe_ShapeManager::addShape(fe_Shape shape) {
  shapes.push_back(shape);
  return shapes.size() - 1;
};

fe_Shape& fe_ShapeManager::getShape(uint32_t index) {
  return shapes.at(index);
}
