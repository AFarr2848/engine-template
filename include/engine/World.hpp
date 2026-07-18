#pragma once
#include "engine/Camera.hpp"
#include "engine/Shapes.hpp"
#include "engine/Structs.hpp"
#include "vulkan/vulkan.hpp"
class fe_Camera;

class fe_World {
 public:
  fe_World() {}

  std::vector<glm::mat4> transforms;

  void init() { createShapes(); }

  uint32_t addShape(fe_Shape shape, fe_Material material);

  void transformShapes();

  void prepareDraw(std::vector<fe_Vertex>& vertices,
                   std::vector<uint32_t>& indices,
                   std::vector<fe_DrawInfo>& drawInfos);

 private:
  std::vector<std::pair<fe_Shape, fe_Material>> shapes;

  void createShapes();
};
