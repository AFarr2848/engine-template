#include "engine/Engine.hpp"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vulkan/vulkan_raii.hpp>
#include "engine/Swapchain.hpp"
#include "engine/Timing.hpp"
#include "engine/VulkanContext.hpp"
#include "engine/Window.hpp"
#include "vulkan/vulkan.hpp"

fe_Engine::~fe_Engine() = default;
fe_Engine::fe_Engine() = default;

void fe_Engine::startEngine() {
  win = std::make_unique<fe_Window>();
  ctx = std::make_unique<fe_VulkanContext>(*win);
  swp = std::make_unique<fe_Swapchain>(*win, *ctx);
  tim = std::make_unique<fe_TimingData>(*ctx, *swp);

  win->init();
  ctx->init();
  swp->init();
  tim->init();
}

void fe_Engine::run() {
  while (!glfwWindowShouldClose(win->window)) {
    glfwPollEvents();
    drawFrame();
  }
  ctx->device.waitIdle();
}

void fe_Engine::recordCommandBuffer(uint32_t imageIndex) {
  // Transition the current image to be written to
  tim->getCurrentCmdBuffer().begin({});
  transitionImageLayout(
      tim->getCurrentCmdBuffer(), swp->swapChainImages[imageIndex],
      vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, {},
      vk::AccessFlagBits2::eColorAttachmentWrite,
      vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      vk::ImageAspectFlagBits::eColor);

  vk::ClearValue clearColor{};
  clearColor.color =
      vk::ClearColorValue{std::array<float, 4>{0.1f, 0.1f, 0.15f, 1.0f}};

  vk::RenderingAttachmentInfo colorAttachmentInfo{
      .imageView = swp->swapChainImageViews[imageIndex],
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .resolveMode = vk::ResolveModeFlagBits::eNone,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue = clearColor};

  // 4. Set up the overall rendering region and parameters
  vk::RenderingInfo renderingInfo = {
      .renderArea = {.offset = {0, 0}, .extent = swp->swapChainExtent},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachmentInfo,
      //.pDepthAttachment = &depthAttachmentInfo

  };

  tim->getCurrentCmdBuffer().beginRendering(renderingInfo);

  tim->getCurrentCmdBuffer().endRendering();

  // transition to present
  transitionImageLayout(
      tim->getCurrentCmdBuffer(), swp->swapChainImages[imageIndex],
      vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
      vk::AccessFlagBits2::eColorAttachmentWrite,          // srcAccessMask
      {},                                                  // dstAccessMask
      vk::PipelineStageFlagBits2::eColorAttachmentOutput,  // srcStage
      vk::PipelineStageFlagBits2::eBottomOfPipe,           // dstStage
      vk::ImageAspectFlagBits::eColor);

  tim->getCurrentCmdBuffer().end();
}

void fe_Engine::drawFrame() {
  // Wait for previous frame to finish
  if (tim->currentFrame != 0) {
    uint64_t waitValue = tim->currentFrame - 1;
    vk::Semaphore semaphore = *(tim->timelineSemaphore);
    while (vk::Result::eTimeout ==
           ctx->device.waitSemaphores({.semaphoreCount = 1,
                                       .pSemaphores = &semaphore,
                                       .pValues = &waitValue},
                                      UINT64_MAX))
      ;
  }

  // grab the next image to display, telling the GPU to signal
  // presentCompleteSemaphore when it's done presenting

  vk::raii::Semaphore& raiiPresentFinished =
      tim->getCurrentPresentCompleteSemaphore();
  vk::Semaphore presentFinSem = *raiiPresentFinished;

  auto [result, imageIndex] =
      swp->swapChain.acquireNextImage(UINT64_MAX, presentFinSem, nullptr);

  vk::raii::Semaphore& raiiRenderFinished =
      tim->getCurrentRenderFinishedSemaphore(imageIndex);
  vk::Semaphore renderFinSem = *raiiRenderFinished;

  if (result == vk::Result::eErrorOutOfDateKHR) {
    // TODO:
    //  changeResolution();
    return;
  }
  if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  // clear the command buffer that's about to be used
  tim->getCurrentCmdBuffer().reset();

  // update all my stuff
  // TODO:

  // submit my draw commands
  recordCommandBuffer(imageIndex);

  // Tell the GPU to wait on the present image to be released
  std::vector<vk::SemaphoreSubmitInfo> waitInfos = {vk::SemaphoreSubmitInfo{
      .semaphore = presentFinSem,
      .stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput}};

  // Tell the GPU to signal the swapchain present and the CPU timeline
  // semaphore when it's done
  vk::Semaphore timelineSemaphore = *(tim->timelineSemaphore);
  std::vector<vk::SemaphoreSubmitInfo> signalInfos = {
      vk::SemaphoreSubmitInfo{
          .semaphore = renderFinSem,
          .stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput},
      vk::SemaphoreSubmitInfo{
          .semaphore = timelineSemaphore,
          .value = static_cast<uint64_t>(tim->currentFrame + 1),
          .stageMask = vk::PipelineStageFlagBits2::eAllCommands}};

  vk::CommandBufferSubmitInfo cmdBufferInfo = {.commandBuffer =
                                                   tim->getCurrentCmdBuffer()};

  vk::SubmitInfo2 submitInfo{
      .waitSemaphoreInfoCount = static_cast<uint32_t>(waitInfos.size()),
      .pWaitSemaphoreInfos = waitInfos.data(),
      .commandBufferInfoCount = 1,
      .pCommandBufferInfos = &cmdBufferInfo,
      .signalSemaphoreInfoCount = static_cast<uint32_t>(signalInfos.size()),
      .pSignalSemaphoreInfos = signalInfos.data()};

  ctx->graphicsQueue.submit2(submitInfo);

  try {
    vk::SwapchainKHR rawSwapchain = *swp->swapChain;
    const vk::PresentInfoKHR presentInfo{.waitSemaphoreCount = 1,
                                         .pWaitSemaphores = &renderFinSem,
                                         .swapchainCount = 1,
                                         .pSwapchains = &rawSwapchain,
                                         .pImageIndices = &imageIndex};
    auto result = ctx->graphicsQueue.presentKHR(presentInfo);
  } catch (const vk::SystemError& e) {
    std::cerr << e.what() << std::endl;
  }

  tim->incrementTiming();
}

void fe_Engine::transitionImageLayout(vk::raii::CommandBuffer& cmd,
                                      vk::Image image,
                                      vk::ImageLayout old_layout,
                                      vk::ImageLayout new_layout,
                                      vk::AccessFlags2 src_access_mask,
                                      vk::AccessFlags2 dst_access_mask,
                                      vk::PipelineStageFlags2 src_stage_mask,
                                      vk::PipelineStageFlags2 dst_stage_mask,
                                      vk::ImageAspectFlags image_aspect_flags) {
  vk::ImageMemoryBarrier2 barrier = {
      .srcStageMask = src_stage_mask,
      .srcAccessMask = src_access_mask,
      .dstStageMask = dst_stage_mask,
      .dstAccessMask = dst_access_mask,
      .oldLayout = old_layout,
      .newLayout = new_layout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange = {.aspectMask = image_aspect_flags,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};
  vk::DependencyInfo dependency_info = {.dependencyFlags = {},
                                        .imageMemoryBarrierCount = 1,
                                        .pImageMemoryBarriers = &barrier};
  cmd.pipelineBarrier2(dependency_info);
}
