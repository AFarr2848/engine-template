#include "engine/managers/BufferManager.hpp"
#include <cstdint>
#include <glm/ext/matrix_float4x4.hpp>
#include <vulkan/vulkan.hpp>
#include "engine/Structs.hpp"
#include "engine/Timing.hpp"
#include "engine/VulkanContext.hpp"
#include "vulkan/vulkan.hpp"

void fe_BufferManager::createWorldBuffer() {
  ctx.createBuffer(sizeof(fe_WorldData),
                   vk::BufferUsageFlagBits::eStorageBuffer |
                       vk::BufferUsageFlagBits::eShaderDeviceAddress,
                   vk::MemoryPropertyFlagBits::eDeviceLocal |
                       vk::MemoryPropertyFlagBits::eHostVisible |
                       vk::MemoryPropertyFlagBits::eHostCoherent,
                   {.flags = vk::MemoryAllocateFlagBits::eDeviceAddress},
                   worldBuffer, worldBufferMemory);
  worldBufferAddress = ctx.device.getBufferAddress({.buffer = worldBuffer});
}

void fe_BufferManager::updateWorldBuffer(fe_WorldData worldData) {
  // Check if memcpy works properly
  static_assert(std::is_trivially_copyable<fe_WorldData>::value,
                "fe_WorldData is not safely copyable with memcpy!");

  void* data = ctx.device.mapMemory2({
      .memory = worldBufferMemory,
      .offset = 0,
      .size = sizeof(fe_WorldData),
  });

  // literally just copy it in because it's eHostVisible and eDeviceLocal
  memcpy(data, &worldData, sizeof(fe_WorldData));

  ctx.device.unmapMemory2({.memory = worldBufferMemory});
}

void fe_BufferManager::createTransformBuffer(size_t size) {
  ctx.createBuffer(size,
                   vk::BufferUsageFlagBits::eStorageBuffer |
                       vk::BufferUsageFlagBits::eShaderDeviceAddress,
                   vk::MemoryPropertyFlagBits::eDeviceLocal |
                       vk::MemoryPropertyFlagBits::eHostVisible |
                       vk::MemoryPropertyFlagBits::eHostCoherent,
                   {.flags = vk::MemoryAllocateFlagBits::eDeviceAddress},
                   transformBuffer, transformBufferMemory);
  transformBufferAddress =
      ctx.device.getBufferAddress({.buffer = transformBuffer});
  transformSize = size;
}

void fe_BufferManager::updateTransformBuffer(
    std::vector<glm::mat4>& transforms) {
  assert(sizeof(glm::mat4) * transforms.size() == transformSize);

  void* data = ctx.device.mapMemory2({
      .memory = transformBufferMemory,
      .offset = 0,
      .size = transformSize,
  });

  // literally just copy it in because it's eHostVisible and eDeviceLocal
  memcpy(data, transforms.data(), transformSize);

  ctx.device.unmapMemory2({.memory = transformBufferMemory});
}

void fe_BufferManager::createMeshBuffer(std::vector<fe_Vertex> vertices,
                                        std::vector<uint32_t> indices) {
  // find sizes of buffers
  verticesSize = sizeof(vertices[0]) * vertices.size();
  indicesSize = sizeof(indices[0]) * indices.size();

  // makea da buffer
  ctx.createBuffer(verticesSize + indicesSize,
                   vk::BufferUsageFlagBits::eShaderDeviceAddress |
                       vk::BufferUsageFlagBits::eIndexBuffer |
                       vk::BufferUsageFlagBits::eStorageBuffer,
                   vk::MemoryPropertyFlagBits::eHostVisible |
                       vk::MemoryPropertyFlagBits::eDeviceLocal |
                       vk::MemoryPropertyFlagBits::eHostCoherent,
                   {.flags = vk::MemoryAllocateFlagBits::eDeviceAddress},
                   meshBuffer, meshBufferMemory);

  // map memory so CPU can see
  void* data = ctx.device.mapMemory2({
      .memory = meshBufferMemory,
      .offset = 0,
      .size = verticesSize + indicesSize,
  });

  // literally just copy it in because it's eHostVisible and eDeviceLocal
  memcpy(data, vertices.data(), verticesSize);
  void* indicesDest = static_cast<char*>(data) + verticesSize;
  memcpy(indicesDest, indices.data(), indicesSize);

  ctx.device.unmapMemory2({.memory = meshBufferMemory});
  meshBufferAddress = ctx.device.getBufferAddress({.buffer = meshBuffer});
}
