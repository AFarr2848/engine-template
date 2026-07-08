#pragma once
class fe_VulkanContext;
class fe_Window;
class fe_Images;

class fe_Swapchain {
 public:
  fe_Swapchain(fe_Window& win, fe_VulkanContext& ctx)
      : win(win),
        ctx(ctx)

  {}

  /**
   * @brief Init
   * @details Requires ctx.init() and win.init() to be called
   */
  void init() {
    createSwapChain();
    createImageViews();
  }

  void recreate();
  void cleanup();

  vk::Extent2D extent() const;

  std::vector<vk::Image> swapChainImages;
  std::vector<vk::raii::ImageView> swapChainImageViews;
  vk::Extent2D swapChainExtent;
  vk::raii::SwapchainKHR swapChain = nullptr;
  vk::SurfaceFormatKHR swapChainSurfaceFormat;

  vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<vk::SurfaceFormatKHR>& availableFormats);

  uint32_t chooseSwapMinImageCount(
      vk::SurfaceCapabilitiesKHR const& surfaceCapabilities);

  vk::PresentModeKHR chooseSwapPresentMode(
      const std::vector<vk::PresentModeKHR>& availablePresentModes);

  void recreateSwapChain();

  void cleanupSwapChain();

  vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

 private:
  fe_VulkanContext& ctx;
  fe_Window& win;

  void createSwapChain();
  void createImageViews();
};
