// This has to handle:
//  - Uploading vertices and indices
//  - Choosing textures
//  - Input
//  - Configuring drawing settings
//  - Updating transforms
//

#include "engine/World.hpp"
#include "engine/Camera.hpp"
#include "engine/InputHelper.hpp"
#include "engine/Shapes.hpp"
#include "engine/Structs.hpp"
uint32_t fe_World::addShape(fe_Shape shape, fe_Material material) {
  shapes.push_back({{shape}, {material}});
  transforms.push_back(glm::mat4(1.0f));
  return shapes.size() - 1;
}

void fe_World::transformShapes() {}

void fe_World::processInput(fe_FrameContext frameContext) {
  if (inputHelper.isKeyDown('A')) {
    camera.processKeyboard(LEFT, frameContext.deltaTime);
  }
  if (inputHelper.isKeyDown('W'))
    camera.processKeyboard(FORWARD, frameContext.deltaTime);
  if (inputHelper.isKeyDown('D'))
    camera.processKeyboard(RIGHT, frameContext.deltaTime);
  if (inputHelper.isKeyDown('S'))
    camera.processKeyboard(BACKWARD, frameContext.deltaTime);

  if (inputHelper.isKeyDown(KEY_SPACE))
    camera.processKeyboard(UP, frameContext.deltaTime);
  if (inputHelper.isKeyDown(KEY_LEFT_CTRL))
    camera.processKeyboard(DOWN, frameContext.deltaTime);

  camera.ProcessMouseMovement(inputHelper.getMouseOffsets().x,
                              inputHelper.getMouseOffsets().y);
}

void fe_World::prepareDraw(std::vector<fe_Vertex>& vertices,
                           std::vector<uint32_t>& indices,
                           std::vector<fe_DrawInfo>& drawInfos) {
  for (auto& pair : shapes) {
    drawInfos.push_back(
        {.indexCount = static_cast<uint32_t>(pair.first.indices.size()),
         .indexOffset = static_cast<uint32_t>(indices.size()),
         .transformIndex = static_cast<uint32_t>(drawInfos.size()),
         .material = pair.second});
    vertices.insert(vertices.end(), pair.first.vertices.begin(),
                    pair.first.vertices.end());
    indices.insert(indices.end(), pair.first.indices.begin(),
                   pair.first.indices.end());
  }
}

fe_WorldData fe_World::getWorldData(fe_FrameContext frameContext) {
  fe_WorldData worldData = {
      .view = camera.GetViewMatrix(),
      .model = glm::mat4(1.0f),
      .proj =
          glm::perspective(glm::radians(45.0f),
                           static_cast<float>(frameContext.screenWidth) /
                               static_cast<float>(frameContext.screenHeight),
                           0.1f, 100.0f),
      .cameraPos = camera.Position

  };
  worldData.proj[1][1] *= -1;

  return worldData;
}

void fe_World::createShapes() {
  addShape({fe_Cube()}, {.shader = "triangle", .texture = ""});
}
