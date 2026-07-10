#include "engine/managers/BufferManager.hpp"
#include <vulkan/vulkan.hpp>
#include "engine/Structs.hpp"
#include "engine/Timing.hpp"
#include "engine/VulkanContext.hpp"
#include "vulkan/vulkan.hpp"

void fe_BufferManager::createMeshBuffer(std::vector<Vertex> vertices,
                                        std::vector<uint32_t> indices) {
  // find sizes of buffers
  verticesSize = sizeof(vertices[0]) * vertices.size();
  indicesSize = sizeof(indices[0]) * indices.size();

  // makea da buffer
  createBuffer(verticesSize + indicesSize,
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
      .size = verticesSize,
  });

  // literally just copy it in because it's eHostVisible and eDeviceLocal
  memcpy(data, vertices.data(), verticesSize);
  void* indicesDest = static_cast<char*>(data) + verticesSize;
  memcpy(indicesDest, indices.data(), indicesSize);

  ctx.device.unmapMemory2({.memory = meshBufferMemory});
  meshBufferAddress = ctx.device.getBufferAddress({.buffer = meshBuffer});
}

void fe_BufferManager::createBuffer(vk::DeviceSize size,
                                    vk::BufferUsageFlags usage,
                                    vk::MemoryPropertyFlags properties,
                                    vk::MemoryAllocateFlagsInfo allocFlagsInfo,
                                    vk::raii::Buffer& buffer,
                                    vk::raii::DeviceMemory& bufferMemory) {
  vk::BufferCreateInfo bufferInfo{
      .size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive};

  buffer = vk::raii::Buffer(ctx.device, bufferInfo);
  vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
  vk::MemoryAllocateInfo allocInfo{
      .pNext = allocFlagsInfo,
      .allocationSize = memRequirements.size,
      .memoryTypeIndex =
          ctx.findMemoryType(memRequirements.memoryTypeBits, properties)

  };
  bufferMemory = vk::raii::DeviceMemory(ctx.device, allocInfo);
  buffer.bindMemory(*bufferMemory, 0);
}
