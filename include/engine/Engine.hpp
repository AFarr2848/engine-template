#pragma once

#include <memory>

class fe_Window;
class fe_VulkanContext;
class fe_TimingData;
class fe_Swapchain;
class fe_AssetManager;

class fe_Engine {
 public:
  fe_Engine();
  ~fe_Engine();

  // vroom
  void startEngine();
  void run();

 private:
  /**
   * @brief Handles timing, increments frame stuff, inits and submits the
   * command buffer, and calls buffer updates and command records
   */
  void drawFrame();

  void recordCommandBuffer(uint32_t imageIndex);

  // TODO: Move me
  void transitionImageLayout(vk::raii::CommandBuffer& cmd,
                             vk::Image image,
                             vk::ImageLayout old_layout,
                             vk::ImageLayout new_layout,
                             vk::AccessFlags2 src_access_mask,
                             vk::AccessFlags2 dst_access_mask,
                             vk::PipelineStageFlags2 src_stage_mask,
                             vk::PipelineStageFlags2 dst_stage_mask,
                             vk::ImageAspectFlags image_aspect_flags);

  std::unique_ptr<fe_Window> win;
  std::unique_ptr<fe_VulkanContext> ctx;
  std::unique_ptr<fe_Swapchain> swp;
  std::unique_ptr<fe_TimingData> tim;
  std::unique_ptr<fe_AssetManager> man;
};
