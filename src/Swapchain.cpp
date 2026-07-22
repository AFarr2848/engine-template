
#include "engine/Swapchain.hpp"
#include "engine/VulkanContext.hpp"
#include "engine/Window.hpp"
#include "vulkan/vulkan.hpp"
void fe_Swapchain::createSwapChain() {
  auto surfaceCapabilities =
      ctx.physicalDevice.getSurfaceCapabilitiesKHR(*ctx.surface);
  swapChainExtent = chooseSwapExtent(surfaceCapabilities);
  swapChainSurfaceFormat = chooseSwapSurfaceFormat(
      ctx.physicalDevice.getSurfaceFormatsKHR(*ctx.surface));
  vk::SwapchainCreateInfoKHR swapChainCreateInfo{
      .surface = *ctx.surface,
      .minImageCount = chooseSwapMinImageCount(surfaceCapabilities),
      .imageFormat = swapChainSurfaceFormat.format,
      .imageColorSpace = swapChainSurfaceFormat.colorSpace,
      .imageExtent = swapChainExtent,
      .imageArrayLayers = 1,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment |
                    vk::ImageUsageFlagBits::eTransferDst,
      .imageSharingMode = vk::SharingMode::eExclusive,
      .preTransform = surfaceCapabilities.currentTransform,
      .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .presentMode = chooseSwapPresentMode(
          ctx.physicalDevice.getSurfacePresentModesKHR(*ctx.surface)),
      .clipped = true};

  swapChain = vk::raii::SwapchainKHR(ctx.device, swapChainCreateInfo);
  swapChainImages = swapChain.getImages();
}

void fe_Swapchain::createImageViews() {
  assert(swapChainImageViews.empty());

  vk::ImageViewCreateInfo imageViewCreateInfo{
      .viewType = vk::ImageViewType::e2D,
      .format = swapChainSurfaceFormat.format,
      .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
  for (auto& image : swapChainImages) {
    imageViewCreateInfo.image = image;
    swapChainImageViews.emplace_back(ctx.device, imageViewCreateInfo);
  }
}

vk::Extent2D fe_Swapchain::chooseSwapExtent(
    const vk::SurfaceCapabilitiesKHR& capabilities) {
  if (capabilities.currentExtent.width != 0xFFFFFFFF) {
    return capabilities.currentExtent;
  }
  int width, height;
  glfwGetFramebufferSize(win.window, &width, &height);

  return {std::clamp<uint32_t>(width, capabilities.minImageExtent.width,
                               capabilities.maxImageExtent.width),
          std::clamp<uint32_t>(height, capabilities.minImageExtent.height,
                               capabilities.maxImageExtent.height)};
}

vk::SurfaceFormatKHR fe_Swapchain::chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
  assert(!availableFormats.empty());
  const auto formatIt =
      std::ranges::find_if(availableFormats, [](const auto& format) {
        return format.format == vk::Format::eR8G8B8A8Srgb &&
               format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
      });
  return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}

uint32_t fe_Swapchain::chooseSwapMinImageCount(
    vk::SurfaceCapabilitiesKHR const& surfaceCapabilities) {
  auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
  if ((0 < surfaceCapabilities.maxImageCount) &&
      (surfaceCapabilities.maxImageCount < minImageCount)) {
    minImageCount = surfaceCapabilities.maxImageCount;
  }
  return minImageCount;
}

vk::PresentModeKHR fe_Swapchain::chooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR>& availablePresentModes) {
  // TODO: Remove this lol
  return vk::PresentModeKHR::eImmediate;
  assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) {
    return presentMode == vk::PresentModeKHR::eFifo;
  }));
  return std::ranges::any_of(availablePresentModes,
                             [](const vk::PresentModeKHR value) {
                               return vk::PresentModeKHR::eMailbox == value;
                             })
             ? vk::PresentModeKHR::eMailbox
             : vk::PresentModeKHR::eFifo;
}

void fe_Swapchain::createDepthImage() {
  vk::Format depthFormat = vk::Format::eD32Sfloat;

  vk::ImageCreateInfo imageInfo{
      .imageType = vk::ImageType::e2D,
      .format = depthFormat,
      .extent = vk::Extent3D{swapChainExtent.width, swapChainExtent.height, 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = vk::SampleCountFlagBits::e1,
      .tiling = vk::ImageTiling::eOptimal,
      .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
      .sharingMode = vk::SharingMode::eExclusive,
      .initialLayout = vk::ImageLayout::eUndefined};

  depthImage = ctx.device.createImage(imageInfo);

  vk::MemoryRequirements memRequirements = depthImage.getMemoryRequirements();

  vk::MemoryAllocateInfo allocInfo{
      .allocationSize = memRequirements.size,
      .memoryTypeIndex =
          ctx.findMemoryType(memRequirements.memoryTypeBits,
                             vk::MemoryPropertyFlagBits::eDeviceLocal)};

  depthImageMemory = vk::raii::DeviceMemory(ctx.device, allocInfo);
  depthImage.bindMemory(depthImageMemory, 0);

  vk::ImageViewCreateInfo viewInfo{
      .image = depthImage,
      .viewType = vk::ImageViewType::e2D,
      .format = depthFormat,
      .subresourceRange = vk::ImageSubresourceRange{
          .aspectMask = vk::ImageAspectFlagBits::eDepth,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1}};

  depthImageView = ctx.device.createImageView(viewInfo);
}

void fe_Swapchain::recreateSwapChain() {
  int width = 0, height = 0;
  glfwGetFramebufferSize(win.window, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(win.window, &width, &height);
    glfwWaitEvents();
  }
  ctx.device.waitIdle();

  cleanupSwapChain();
  createSwapChain();
  createImageViews();
  createDepthImage();
}

void fe_Swapchain::cleanupSwapChain() {
  swapChainImageViews.clear();
  swapChain = nullptr;
  depthImage = nullptr;
  depthImageView = nullptr;
}
