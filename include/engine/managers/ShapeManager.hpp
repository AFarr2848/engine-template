#pragma once

#include "engine/Shapes.hpp"
class fe_ShapeManager {
 public:
  fe_ShapeManager() {};

  uint32_t addShape(fe_Shape shape);
  fe_Shape& getShape(uint32_t index);

 private:
  std::vector<fe_Shape> shapes;
};
