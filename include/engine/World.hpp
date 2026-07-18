#pragma once
#include "engine/Camera.hpp"
#include "engine/InputHelper.hpp"
#include "engine/Shapes.hpp"
#include "engine/Structs.hpp"

class fe_Camera;
class fe_FrameContext;

class fe_World {
 public:
  fe_World(fe_InputHelper& inputHelper) : inputHelper(inputHelper) {}

  std::vector<glm::mat4> transforms;

  void init() { createShapes(); }

  uint32_t addShape(fe_Shape shape, glm::mat4 transform, fe_Material material);

  void transformShapes();

  void prepareDraw(std::vector<fe_Vertex>& vertices,
                   std::vector<uint32_t>& indices,
                   std::vector<fe_DrawInfo>& drawInfos);

  fe_WorldData getWorldData(fe_FrameContext frameContext);

  void processInput(fe_FrameContext frameContext);

 private:
  std::vector<std::pair<fe_Shape, fe_Material>> shapes;
  fe_Camera camera;
  fe_InputHelper& inputHelper;

  void createShapes();
};
