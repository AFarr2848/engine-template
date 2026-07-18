#pragma once
#include <cstdint>
#include <glm/glm.hpp>

struct fe_Vertex {
  alignas(16) glm::vec3 pos;
  alignas(16) glm::vec3 normal;
  alignas(16) glm::vec2 uv;
};

struct fe_PushConstants {
  uint64_t vertBufAddress;
  uint64_t transformBufAddress;
  uint32_t transformIndex;
};

struct fe_Material {
  std::string shader;
  std::string texture;
};

struct fe_DrawInfo {
  uint32_t indexCount;
  uint32_t indexOffset;
  uint32_t transformIndex;
  fe_Material material;
};

struct fe_Texture {
  vk::Image image;
  vk::DeviceMemory memory;
  vk::ImageView view;
};
