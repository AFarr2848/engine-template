#include <cstdint>
class fe_VulkanContext;
class fe_Swapchain;
class fe_VulkanEngine;

class fe_TimingData {
 public:
  fe_TimingData(fe_VulkanContext& ctx, fe_Swapchain& swp)
      : ctx(ctx), swp(swp) {};

  /**
   * @brief Init
   * @detail Should go last (?)
   */
  void init() {
    createCommandPool();
    createCommandBuffers();
    createSyncObjects();
  }

  int64_t currentFrame = 0;
  int32_t currentFrameInFlight = 0;

  // Bc the number of semaphores are based on the swapchain image count, which
  // isn't necessarily MAX_FRAMES_IN_FLIGHT
  uint32_t semaphoreIndex = 0;

  vk::raii::CommandPool commandPool = nullptr;

  vk::raii::Semaphore timelineSemaphore = nullptr;

  vk::raii::CommandBuffer& getCurrentCmdBuffer();
  vk::raii::Semaphore& getCurrentPresentCompleteSemaphore();
  vk::raii::Semaphore& getCurrentRenderFinishedSemaphore(uint32_t imageIndex);

  /**
   * @brief Increments currentFrame, currentFrameInFlight, and semaphoreIndex.
   * To be called once per frame.
   */
  void incrementTiming();

  std::unique_ptr<vk::raii::CommandBuffer> beginSingleTimeCommands();
  void endSingleTimeCommands(vk::raii::CommandBuffer& commandBuffer);

 private:
  fe_VulkanContext& ctx;
  fe_Swapchain& swp;

  std::vector<vk::raii::CommandBuffer> commandBuffers;
  std::vector<vk::raii::Semaphore> presentCompleteSemaphore;
  std::vector<vk::raii::Semaphore> renderFinishedSemaphore;

  void createCommandPool();
  void createCommandBuffers();
  void createSyncObjects();
};
