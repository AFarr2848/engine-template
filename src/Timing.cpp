#include "engine/Timing.hpp"
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <vulkan/vulkan_raii.hpp>
#include "Config.hpp"
#include "engine/Swapchain.hpp"
#include "engine/VulkanContext.hpp"
#include "vulkan/vulkan.hpp"

void fe_TimingData::createCommandPool() {
  vk::CommandPoolCreateInfo poolInfo{
      .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = ctx.queueIndex};
  commandPool = vk::raii::CommandPool(ctx.device, poolInfo);
}

void fe_TimingData::createCommandBuffers() {
  commandBuffers.clear();
  vk::CommandBufferAllocateInfo allocInfo{
      .commandPool = commandPool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = MAX_FRAMES_IN_FLIGHT};
  commandBuffers = vk::raii::CommandBuffers(ctx.device, allocInfo);
}

void fe_TimingData::createSyncObjects() {
  presentCompleteSemaphore.clear();
  renderFinishedSemaphore.clear();

  vk::SemaphoreTypeCreateInfo timelineCreateInfo{
      .semaphoreType = vk::SemaphoreType::eTimeline, .initialValue = 0};

  vk::SemaphoreCreateInfo createInfo{.pNext = &timelineCreateInfo};

  try {
    timelineSemaphore = vk::raii::Semaphore(ctx.device, createInfo);

  } catch (const vk::SystemError& err) {
    throw std::runtime_error(
        std::string("Failed to create timeline semaphore: ") + err.what());
  }

  for (size_t i = 0; i < swp.swapChainImages.size(); i++) {
    presentCompleteSemaphore.emplace_back(ctx.device,
                                          vk::SemaphoreCreateInfo());
    renderFinishedSemaphore.emplace_back(ctx.device, vk::SemaphoreCreateInfo());
  }
}

std::unique_ptr<vk::raii::CommandBuffer>
fe_TimingData::beginSingleTimeCommands() {
  vk::CommandBufferAllocateInfo allocInfo{
      .commandPool = commandPool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = 1};
  std::unique_ptr<vk::raii::CommandBuffer> commandBuffer =
      std::make_unique<vk::raii::CommandBuffer>(
          std::move(vk::raii::CommandBuffers(ctx.device, allocInfo).front()));

  vk::CommandBufferBeginInfo beginInfo{
      .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
  commandBuffer->begin(beginInfo);

  return commandBuffer;
}

void fe_TimingData::endSingleTimeCommands(
    vk::raii::CommandBuffer& commandBuffer) {
  commandBuffer.end();

  vk::SubmitInfo submitInfo{.commandBufferCount = 1,
                            .pCommandBuffers = &*commandBuffer};
  ctx.graphicsQueue.submit(submitInfo, nullptr);
  ctx.graphicsQueue.waitIdle();
}

vk::raii::CommandBuffer& fe_TimingData::getCurrentCmdBuffer() {
  return commandBuffers.at(currentFrameInFlight);
}

vk::raii::Semaphore& fe_TimingData::getCurrentPresentCompleteSemaphore() {
  return presentCompleteSemaphore.at(semaphoreIndex);
}
vk::raii::Semaphore& fe_TimingData::getCurrentRenderFinishedSemaphore(
    uint32_t imageIndex) {
  return renderFinishedSemaphore.at(imageIndex);
}

void fe_TimingData::incrementTiming() {
  semaphoreIndex = (semaphoreIndex + 1) % presentCompleteSemaphore.size();
  currentFrame += 1;
  currentFrameInFlight = (currentFrame) % MAX_FRAMES_IN_FLIGHT;
}
