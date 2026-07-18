#include "engine/Engine.hpp"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vulkan/vulkan_hpp_macros.hpp>
#include <vulkan/vulkan_raii.hpp>
#include "engine/Structs.hpp"
#include "engine/Swapchain.hpp"
#include "engine/Timing.hpp"
#include "engine/VulkanContext.hpp"
#include "engine/Window.hpp"
#include "engine/World.hpp"
#include "engine/managers/BufferManager.hpp"
#include "engine/managers/ShaderManager.hpp"
#include "vulkan/vulkan.hpp"

fe_Engine::~fe_Engine() = default;
fe_Engine::fe_Engine() = default;

void fe_Engine::startEngine() {
  win = std::make_unique<fe_Window>();
  ctx = std::make_unique<fe_VulkanContext>(*win);
  swp = std::make_unique<fe_Swapchain>(*win, *ctx);
  tim = std::make_unique<fe_TimingData>(*ctx, *swp);
  shaderMan = std::make_unique<fe_ShaderManager>(*ctx);
  bufferMan = std::make_unique<fe_BufferManager>(*ctx);
  world = std::make_unique<fe_World>();

  win->init();
  ctx->init();
  swp->init();
  tim->init();
  world->init();

  shaderMan->loadShaderModule("triangle_vert",
                              "build/shaders/triangle_vert.spv",
                              vk::ShaderStageFlagBits::eVertex);
  shaderMan->loadShaderModule("triangle_frag",
                              "build/shaders/triangle_frag.spv",
                              vk::ShaderStageFlagBits::eFragment);

  std::vector<fe_Vertex> vertices = {};
  std::vector<uint32_t> indices = {};

  world->prepareDraw(vertices, indices, drawInfos);

  bufferMan->createMeshBuffer(vertices, indices);
  bufferMan->createTransformBuffer(sizeof(glm::mat4) *
                                   world->transforms.size());
}

void fe_Engine::run() {
  while (!glfwWindowShouldClose(win->window)) {
    glfwPollEvents();
    world->transformShapes();
    bufferMan->updateTransformBuffer(world->transforms);
    drawFrame();
  }
  ctx->device.waitIdle();
}

void fe_Engine::recordCommandBuffer(uint32_t imageIndex) {
  vk::CommandBuffer cmd = tim->getCurrentCmdBuffer();
  vk::CommandBufferBeginInfo beginInfo = {};
  cmd.begin(beginInfo);
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

  cmd.beginRendering(renderingInfo);
  for (auto& info : drawInfos) {
    // bind shaders
    cmd.bindShadersEXT(vk::ShaderStageFlagBits::eVertex,
                       shaderMan->getShader(info.material.shader + "_vert"));
    cmd.bindShadersEXT(vk::ShaderStageFlagBits::eFragment,
                       shaderMan->getShader(info.material.shader + "_frag"));

    // misc. config
    configCommandBuffer();

    // push constants
    fe_PushConstants pcData = {
        .vertBufAddress = bufferMan->meshBufferAddress,
        .transformBufAddress = bufferMan->transformBufferAddress,
        .transformIndex = info.transformIndex};
    cmd.pushConstants(
        ctx->pipelineLayout,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0, sizeof(fe_PushConstants), &pcData);

    cmd.bindIndexBuffer(bufferMan->meshBuffer, bufferMan->verticesSize,
                        vk::IndexType::eUint32);

    cmd.drawIndexed(info.indexCount, 1, info.indexOffset, 0, 0);
  }

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

  uint32_t imageIndex = 0;
  try {
    auto [result, index] =
        swp->swapChain.acquireNextImage(UINT64_MAX, presentFinSem, nullptr);
    imageIndex = index;

    if (result == vk::Result::eSuboptimalKHR) {
      ctx->device.waitIdle();
      swp->recreateSwapChain();
      return;
    }
    // resize swapchain if the window changes
  } catch (const vk::OutOfDateKHRError& e) {
    ctx->device.waitIdle();
    swp->recreateSwapChain();
    return;
  }

  vk::raii::Semaphore& raiiRenderFinished =
      tim->getCurrentRenderFinishedSemaphore(imageIndex);
  vk::Semaphore renderFinSem = *raiiRenderFinished;

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
          .value = static_cast<uint64_t>(tim->currentFrame),
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

    if (result == vk::Result::eSuboptimalKHR) {
      ctx->device.waitIdle();
      swp->recreateSwapChain();
    }
    // resize swapchain if the window changes
  } catch (const vk::OutOfDateKHRError& e) {
    ctx->device.waitIdle();
    swp->recreateSwapChain();
  } catch (const vk::SystemError& e) {
    // Catch any other Vulkan errors that might occur
    std::cerr << "Vulkan Error during present: " << e.what() << std::endl;
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

void fe_Engine::configCommandBuffer() {
  vk::CommandBuffer cmd = tim->getCurrentCmdBuffer();
  // 1. Viewport & Scissor (Note the "WithCount" variants)
  vk::Viewport viewport{0.0f,
                        0.0f,
                        static_cast<float>(swp->swapChainExtent.width),
                        static_cast<float>(swp->swapChainExtent.height),
                        0.0f,
                        1.0f};
  cmd.setViewportWithCountEXT(viewport);

  vk::Rect2D scissor{{0, 0},
                     {static_cast<uint32_t>(swp->swapChainExtent.width),
                      static_cast<uint32_t>(swp->swapChainExtent.height)}};
  cmd.setScissorWithCountEXT(scissor);

  // 2. Input Assembly (How vertices form shapes)
  cmd.setPrimitiveTopologyEXT(vk::PrimitiveTopology::eTriangleList);
  cmd.setPrimitiveRestartEnableEXT(VK_FALSE);

  // 3. Rasterization State (How shapes become pixels)
  cmd.setRasterizerDiscardEnableEXT(VK_FALSE);
  cmd.setPolygonModeEXT(vk::PolygonMode::eFill);
  cmd.setCullModeEXT(vk::CullModeFlagBits::eNone);
  cmd.setFrontFaceEXT(vk::FrontFace::eClockwise);
  cmd.setDepthBiasEnableEXT(VK_FALSE);

  // 4. Depth & Stencil Testing (Turned completely off for a basic triangle)
  cmd.setDepthTestEnableEXT(VK_FALSE);
  cmd.setDepthWriteEnableEXT(VK_FALSE);
  cmd.setDepthBoundsTestEnableEXT(VK_FALSE);
  cmd.setStencilTestEnableEXT(VK_FALSE);

  // 5. Multisampling (Anti-aliasing, set to 1 sample / off)
  cmd.setRasterizationSamplesEXT(vk::SampleCountFlagBits::e1);
  vk::SampleMask sampleMask = 0xFFFFFFFF;
  cmd.setSampleMaskEXT(vk::SampleCountFlagBits::e1, &sampleMask);
  cmd.setAlphaToCoverageEnableEXT(VK_FALSE);

  // 6. Color Blending & Writing (Writing solid colors to your swapchain
  // attachment)
  vk::Bool32 colorBlendEnable = VK_FALSE;
  cmd.setColorBlendEnableEXT(0, 1, &colorBlendEnable);

  // The color write mask defines which RGBA channels we are allowed to write to
  vk::ColorComponentFlags colorWriteMask =
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
  cmd.setColorWriteMaskEXT(0, 1, &colorWriteMask);

  cmd.setVertexInputEXT(0, nullptr, 0, nullptr);
}
