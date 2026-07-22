#include "engine/VulkanContext.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vulkan/vulkan_raii.hpp>
#include "Config.hpp"
#include "engine/Structs.hpp"
#include "engine/Window.hpp"
#include "vulkan/vulkan.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

void fe_VulkanContext::createPipelineLayout(
    vk::raii::DescriptorSetLayout& texLayout) {
  vk::PushConstantRange pcRange = {

      .stageFlags =
          vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
      .offset = 0,
      .size = sizeof(fe_PushConstants)

  };

  vk::DescriptorSetLayout layout = texLayout;

  vk::PipelineLayoutCreateInfo layoutInfo = {

      .setLayoutCount = 1,
      .pSetLayouts = &layout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &pcRange

  };
  pipelineLayout = device.createPipelineLayout(layoutInfo);
}

void fe_VulkanContext::createInstance() {
  // Stuff for extension function loading
  vk::detail::DynamicLoader dl;
  PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
      dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
  VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

  constexpr vk::ApplicationInfo appInfo{
      .pApplicationName = "TEMPLATE",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "No Engine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = vk::ApiVersion14};

  // Get the required layers
  std::vector<char const*> requiredLayers;
  if (enableValidationLayers) {
    requiredLayers.assign(validationLayers.begin(), validationLayers.end());
  }

  // Check if the required layers are supported by the Vulkan implementation.
  auto layerProperties = context.enumerateInstanceLayerProperties();
  for (auto const& requiredLayer : requiredLayers) {
    if (std::ranges::none_of(
            layerProperties, [requiredLayer](auto const& layerProperty) {
              return strcmp(layerProperty.layerName, requiredLayer) == 0;
            })) {
      throw std::runtime_error("Required layer not supported: " +
                               std::string(requiredLayer));
    }
  }

  auto requiredExtensions = getRequiredExtensions();

  // Check if the required extensions are supported by the Vulkan
  // implementation.
  auto extensionProperties = context.enumerateInstanceExtensionProperties();
  for (auto const& requiredExtension : requiredExtensions) {
    if (std::ranges::none_of(
            extensionProperties,
            [requiredExtension](auto const& extensionProperty) {
              return strcmp(extensionProperty.extensionName,
                            requiredExtension) == 0;
            })) {
      throw std::runtime_error("Required extension not supported: " +
                               std::string(requiredExtension));
    }
  }

  vk::InstanceCreateInfo createInfo{
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
      .ppEnabledLayerNames = requiredLayers.data(),
      .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
      .ppEnabledExtensionNames = requiredExtensions.data()};

  instance = vk::raii::Instance(context, createInfo);

  VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);
}

std::vector<const char*> fe_VulkanContext::getRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
  if (enableValidationLayers) {
    extensions.push_back(vk::EXTDebugUtilsExtensionName);
  }

  return extensions;
}

void fe_VulkanContext::setupDebugMessenger() {
  if (!enableValidationLayers)
    return;

  vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
  vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
      vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
      vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
  vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
      .pNext = nullptr,
      .messageSeverity = severityFlags,
      .messageType = messageTypeFlags,
      .pfnUserCallback = &debugCallback

  };
  debugMessenger =
      instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
}

void fe_VulkanContext::createLogicalDevice() {
  const auto queueFamilies = physicalDevice.getQueueFamilyProperties();
  std::optional<uint32_t> unifiedQueueIndex;

  for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
    const bool supportsGraphics = static_cast<bool>(
        queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics);
    const bool supportsPresent =
        physicalDevice.getSurfaceSupportKHR(i, *surface);

    if (supportsGraphics && supportsPresent) {
      unifiedQueueIndex = i;
      break;
    }
  }

  if (!unifiedQueueIndex.has_value()) {
    throw std::runtime_error(
        "Fatal: Could not find a GPU queue that supports both graphics and "
        "presentation!");
  }

  queueIndex = unifiedQueueIndex.value();

  // Declare driver extensiosn
  const std::vector<const char*> enabledExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_SHADER_OBJECT_EXTENSION_NAME};

  // Feature chain from the bottom up
  //
  vk::PhysicalDeviceVulkan11Features vulkan11Features{
      .pNext = nullptr,
      .shaderDrawParameters = VK_TRUE,
  };
  vk::PhysicalDeviceVulkan12Features vulkan12Features{
      .pNext = &vulkan11Features,
      .shaderSampledImageArrayNonUniformIndexing = vk::True,
      .descriptorBindingSampledImageUpdateAfterBind = vk::True,
      .descriptorBindingPartiallyBound = vk::True,
      .descriptorBindingVariableDescriptorCount = vk::True,
      .runtimeDescriptorArray = vk::True,
      .timelineSemaphore = vk::True,
      .bufferDeviceAddress = vk::True

  };

  vk::PhysicalDeviceVulkan13Features vulkan13Features{
      .pNext = &vulkan12Features,
      .synchronization2 = VK_TRUE,
      .dynamicRendering = VK_TRUE};

  vk::PhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures{
      .pNext = &vulkan13Features, .shaderObject = VK_TRUE};

  // Master wrapper points to the top of feature chain
  vk::PhysicalDeviceFeatures2 masterFeatures{.pNext = &shaderObjectFeatures};

  float queuePriority = 1.0f;
  vk::DeviceQueueCreateInfo queueCreateInfo{.queueFamilyIndex = queueIndex,
                                            .queueCount = 1,
                                            .pQueuePriorities = &queuePriority};

  vk::DeviceCreateInfo deviceCreateInfo{
      .pNext = &masterFeatures,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queueCreateInfo,
      .enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
      .ppEnabledExtensionNames = enabledExtensions.data(),
      .pEnabledFeatures = nullptr};

  device = physicalDevice.createDevice(deviceCreateInfo);
  VULKAN_HPP_DEFAULT_DISPATCHER.init(*device);

  graphicsQueue = device.getQueue(queueIndex, 0);
}

void fe_VulkanContext::pickPhysicalDevice() {
  std::vector<vk::raii::PhysicalDevice> devices =
      instance.enumeratePhysicalDevices();
  const auto devIter = std::ranges::find_if(devices, [&](auto const& device) {
    // Check if the device supports the Vulkan 1.3 API version
    bool supportsVulkan1_3 =
        device.getProperties().apiVersion >= VK_API_VERSION_1_3;

    // Check if any of the queue families support graphics operations
    auto queueFamilies = device.getQueueFamilyProperties();
    bool supportsGraphics =
        std::ranges::any_of(queueFamilies, [](auto const& qfp) {
          return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics);
        });

    // Check if all required device extensions are available
    auto availableDeviceExtensions =
        device.enumerateDeviceExtensionProperties();
    bool supportsAllRequiredExtensions = std::ranges::all_of(
        requiredDeviceExtension,
        [&availableDeviceExtensions](auto const& requiredDeviceExtension) {
          return std::ranges::any_of(
              availableDeviceExtensions,
              [requiredDeviceExtension](auto const& availableDeviceExtension) {
                return strcmp(availableDeviceExtension.extensionName,
                              requiredDeviceExtension) == 0;
              });
        });

    // TODO: I think all this is wrong  idk

    auto features =
        device.template getFeatures2<vk::PhysicalDeviceFeatures2,
                                     vk::PhysicalDeviceVulkan11Features,
                                     vk::PhysicalDeviceVulkan12Features,
                                     vk::PhysicalDeviceVulkan13Features>();
    bool supportsRequiredFeatures =
        features.template get<vk::PhysicalDeviceVulkan11Features>()
            .shaderDrawParameters &&
        features.template get<vk::PhysicalDeviceVulkan12Features>()
            .timelineSemaphore &&
        features.template get<vk::PhysicalDeviceVulkan13Features>()
            .synchronization2 &&
        features.template get<vk::PhysicalDeviceVulkan13Features>()
            .dynamicRendering;

    return supportsVulkan1_3 && supportsGraphics &&
           supportsAllRequiredExtensions && supportsRequiredFeatures;
  });
  if (devIter != devices.end()) {
    physicalDevice = *devIter;
  } else {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL fe_VulkanContext::debugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT type,
    const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void*) {
  if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError ||
      severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
    std::cerr << "validation layer: type " << to_string(type)
              << " msg: " << pCallbackData->pMessage << std::endl;
  }

  return vk::False;
}

void fe_VulkanContext::createSurface() {
  VkSurfaceKHR _surface;
  if (glfwCreateWindowSurface(*instance, win.window, nullptr, &_surface) != 0) {
    throw std::runtime_error("failed to create window surface!");
  }
  surface = vk::raii::SurfaceKHR(instance, _surface);
}

uint32_t fe_VulkanContext::findMemoryType(uint32_t typeFilter,
                                          vk::MemoryPropertyFlags properties) {
  vk::PhysicalDeviceMemoryProperties memProperties =
      physicalDevice.getMemoryProperties();
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags &
                                   properties) == properties)) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

vk::Format fe_VulkanContext::findSupportedFormat(
    const std::vector<vk::Format>& candidates,
    vk::ImageTiling tiling,
    vk::FormatFeatureFlags features) {
  for (const auto format : candidates) {
    vk::FormatProperties props = physicalDevice.getFormatProperties(format);
    if (tiling == vk::ImageTiling::eLinear &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    }
    if (tiling == vk::ImageTiling::eOptimal &&
        (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  throw std::runtime_error("failed to find supported format");
}

void fe_VulkanContext::transitionImageLayout(
    vk::raii::CommandBuffer& cmd,
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

void fe_VulkanContext::createBuffer(vk::DeviceSize size,
                                    vk::BufferUsageFlags usage,
                                    vk::MemoryPropertyFlags properties,
                                    vk::MemoryAllocateFlagsInfo allocFlagsInfo,
                                    vk::raii::Buffer& buffer,
                                    vk::raii::DeviceMemory& bufferMemory) {
  vk::BufferCreateInfo bufferInfo{
      .size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive};

  buffer = vk::raii::Buffer(device, bufferInfo);
  vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
  vk::MemoryAllocateInfo allocInfo{
      .pNext = allocFlagsInfo,
      .allocationSize = memRequirements.size,
      .memoryTypeIndex =
          findMemoryType(memRequirements.memoryTypeBits, properties)

  };
  bufferMemory = vk::raii::DeviceMemory(device, allocInfo);
  buffer.bindMemory(*bufferMemory, 0);
}

void fe_VulkanContext::createImage(uint32_t width,
                                   uint32_t height,
                                   vk::Format format,
                                   vk::ImageTiling tiling,
                                   vk::ImageUsageFlags usage,
                                   vk::MemoryPropertyFlags properties,
                                   vk::raii::Image& image,
                                   vk::raii::DeviceMemory& imageMemory,
                                   uint32_t layerCount) {
  vk::ImageCreateInfo imageInfo{.imageType = vk::ImageType::e2D,
                                .format = format,
                                .extent = {width, height, 1},
                                .mipLevels = 1,
                                .arrayLayers = layerCount,
                                .samples = vk::SampleCountFlagBits::e1,
                                .tiling = tiling,
                                .usage = usage,
                                .sharingMode = vk::SharingMode::eExclusive

  };

  image = vk::raii::Image(device, imageInfo);

  vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
  vk::MemoryAllocateInfo allocInfo{
      .allocationSize = memRequirements.size,
      .memoryTypeIndex =
          findMemoryType(memRequirements.memoryTypeBits, properties)};
  imageMemory = vk::raii::DeviceMemory(device, allocInfo);
  vk::BindImageMemoryInfo info = {
      .image = image, .memory = imageMemory, .memoryOffset = 0};
  device.bindImageMemory2(info);
}
