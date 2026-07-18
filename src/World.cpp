// This has to handle:
//  - Uploading vertices and indices
//  - Choosing textures
//  - Input
//  - Configuring drawing settings
//  - Updating transforms
//

#include "engine/World.hpp"
#include "engine/Shapes.hpp"
#include "engine/Structs.hpp"
uint32_t fe_World::addShape(fe_Shape shape, fe_Material material) {
  shapes.push_back({{shape}, {material}});
  transforms.push_back(glm::mat4(1.0f));
  return shapes.size() - 1;
}

void fe_World::transformShapes() {}

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

void fe_World::createShapes() {
  addShape({fe_Cube()}, {.shader = "triangle", .texture = ""});
}
