#pragma once
class fe_Window;

class fe_VulkanContext {
 public:
  void init(fe_Window& win) { this->win = &win; }

  void cleanup();

  // TODO: move this
  uint32_t findMemoryType(uint32_t typeFilter,
                          vk::MemoryPropertyFlags properties);

  vk::raii::Instance instance = nullptr;
  vk::raii::PhysicalDevice physicalDevice = nullptr;
  vk::raii::Device device = nullptr;
  vk::raii::SurfaceKHR surface = nullptr;
  vk::raii::Queue graphicsQueue = nullptr;
  uint32_t queueIndex = ~0;

  /**
   * @brief Creates the vk::raii::instance
   */
  void createInstance();

  /**
   * @brief Creates the vk::raii::DebugUtilsMessengerEXT
   */
  void setupDebugMessenger();

  /**
   * @brief Creates the vk::raii::PhysicalDevice
   * @details Picks the last returned device that supports vk1.3, graphics
   * operations, and all of the needed extensions and features.
   */
  void pickPhysicalDevice();
  /**
   * @brief Creates device and graphicsQueue,
   */
  void createLogicalDevice();
  /**
   * @brief Creates the surface. Incredible.
   */
  void createSurface();

  // TODO: idk why this is even in here
  vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates,
                                 vk::ImageTiling tiling,
                                 vk::FormatFeatureFlags features);

 private:
  fe_Window* win = nullptr;

  vk::raii::Context context;
  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

  std::vector<const char*> getRequiredExtensions();
  static VKAPI_ATTR vk::Bool32 VKAPI_CALL
  debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                vk::DebugUtilsMessageTypeFlagsEXT type,
                const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void*);
};
