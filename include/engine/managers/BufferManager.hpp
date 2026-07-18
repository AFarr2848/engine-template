#pragma once

#include <cstdint>
#include <glm/ext/matrix_float4x4.hpp>
class fe_TimingData;
class fe_VulkanContext;
class fe_Vertex;

class fe_BufferManager {
 public:
  fe_BufferManager(fe_VulkanContext& ctx) : ctx(ctx) {}

  void createMeshBuffer(std::vector<fe_Vertex> vertices,
                        std::vector<uint32_t> indices);

  void createTransformBuffer(size_t size);
  void updateTransformBuffer(std::vector<glm::mat4>& transforms);

  uint64_t meshBufferAddress;
  vk::raii::Buffer meshBuffer = nullptr;

  uint64_t transformBufferAddress;
  vk::raii::Buffer transformBuffer = nullptr;

  vk::DeviceSize verticesSize;
  vk::DeviceSize indicesSize;

 private:
  fe_VulkanContext& ctx;

  void createBuffer(vk::DeviceSize size,
                    vk::BufferUsageFlags usage,
                    vk::MemoryPropertyFlags properties,
                    vk::MemoryAllocateFlagsInfo allocFlagsInfo,
                    vk::raii::Buffer& buffer,
                    vk::raii::DeviceMemory& bufferMemory);

  void copyBuffer(vk::raii::Buffer& srcBuffer,
                  vk::raii::Buffer& dstBuffer,
                  vk::DeviceSize size);

  vk::raii::DeviceMemory meshBufferMemory = nullptr;
  vk::raii::DeviceMemory transformBufferMemory = nullptr;
  size_t transformSize;
};
