#pragma once
class fe_Window;

class fe_VulkanContext {
 public:
  fe_VulkanContext(fe_Window& win) : win(win) {}

  void cleanup();

  /**
   * @brief Init
   * @details Requires the window to be setup
   */
  void init() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
  };

  // TODO: move this
  uint32_t findMemoryType(uint32_t typeFilter,
                          vk::MemoryPropertyFlags properties);

  vk::raii::Instance instance = nullptr;
  vk::raii::PhysicalDevice physicalDevice = nullptr;
  vk::raii::Device device = nullptr;
  vk::raii::SurfaceKHR surface = nullptr;
  vk::raii::Queue graphicsQueue = nullptr;
  uint32_t queueIndex = ~0;

  vk::raii::PipelineLayout pipelineLayout = nullptr;

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

  /**
   * @brief Creates the pipeline layout. Needs to be called seperately and after
   * the texture set layout has been created.
   */
  void createPipelineLayout(vk::raii::DescriptorSetLayout& texLayout);
  //
  // some util stuff
  //

  vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates,
                                 vk::ImageTiling tiling,
                                 vk::FormatFeatureFlags features);

  void transitionImageLayout(vk::raii::CommandBuffer& cmd,
                             vk::Image image,
                             vk::ImageLayout old_layout,
                             vk::ImageLayout new_layout,
                             vk::AccessFlags2 src_access_mask,
                             vk::AccessFlags2 dst_access_mask,
                             vk::PipelineStageFlags2 src_stage_mask,
                             vk::PipelineStageFlags2 dst_stage_mask,
                             vk::ImageAspectFlags image_aspect_flags);

  void createBuffer(vk::DeviceSize size,
                    vk::BufferUsageFlags usage,
                    vk::MemoryPropertyFlags properties,
                    vk::MemoryAllocateFlagsInfo allocFlagsInfo,
                    vk::raii::Buffer& buffer,
                    vk::raii::DeviceMemory& bufferMemory);

  void copyBuffer(vk::raii::Buffer& srcBuffer,
                  vk::raii::Buffer& dstBuffer,
                  vk::DeviceSize size);

  void createImage(uint32_t width,
                   uint32_t height,
                   vk::Format format,
                   vk::ImageTiling tiling,
                   vk::ImageUsageFlags usage,
                   vk::MemoryPropertyFlags properties,
                   vk::raii::Image& image,
                   vk::raii::DeviceMemory& imageMemory,
                   uint32_t layerCount);

 private:
  fe_Window& win;

  vk::raii::Context context;
  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

  std::vector<const char*> getRequiredExtensions();
  static VKAPI_ATTR vk::Bool32 VKAPI_CALL
  debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                vk::DebugUtilsMessageTypeFlagsEXT type,
                const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void*);
};
