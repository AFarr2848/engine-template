#pragma once

#include <cstdint>
class fe_TimingData;
class fe_VulkanContext;
class Vertex;

class fe_BufferManager {
 public:
  fe_BufferManager(fe_VulkanContext& ctx) : ctx(ctx) {}

  void createMeshBuffer(std::vector<Vertex> vertices,
                        std::vector<uint32_t> indices);

  uint64_t meshBufferAddress;
  vk::raii::Buffer meshBuffer = nullptr;
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
};
