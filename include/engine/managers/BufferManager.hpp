#pragma once

#include <cstdint>
#include <glm/ext/matrix_float4x4.hpp>
class fe_TimingData;
class fe_VulkanContext;
class fe_Vertex;
class fe_WorldData;

class fe_BufferManager {
 public:
  fe_BufferManager(fe_VulkanContext& ctx) : ctx(ctx) {}

  void createMeshBuffer(std::vector<fe_Vertex> vertices,
                        std::vector<uint32_t> indices);

  void createTransformBuffer(size_t size);
  void updateTransformBuffer(std::vector<glm::mat4>& transforms);

  void createWorldBuffer();
  void updateWorldBuffer(fe_WorldData data);

  // Contains vertices and indices
  uint64_t meshBufferAddress;
  vk::raii::Buffer meshBuffer = nullptr;

  // Contains transforms for shapes
  uint64_t transformBufferAddress;
  vk::raii::Buffer transformBuffer = nullptr;

  // Contains static world info per frame like camera
  uint64_t worldBufferAddress;
  vk::raii::Buffer worldBuffer = nullptr;

  vk::DeviceSize verticesSize;
  vk::DeviceSize indicesSize;

 private:
  fe_VulkanContext& ctx;

  vk::raii::DeviceMemory meshBufferMemory = nullptr;
  vk::raii::DeviceMemory transformBufferMemory = nullptr;
  vk::raii::DeviceMemory worldBufferMemory = nullptr;
  size_t transformSize;
};
