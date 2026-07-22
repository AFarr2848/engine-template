
#include "engine/managers/TextureManager.hpp"
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vulkan/vulkan_raii.hpp>
#include "Config.hpp"
#include "engine/Timing.hpp"
#include "engine/VulkanContext.hpp"
#include "engine/managers/ShaderManager.hpp"
#include "vulkan/vulkan.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void fe_TextureManager::loadTextures() {
  for (const auto& entry :
       std::filesystem::directory_iterator("./assets/textures")) {
    fe_Texture tex = {.name = entry.path().filename().string()};
    createTextureSampler(tex);
    createTextureImage(tex, entry.path(), glm::vec3(1.0f, 0, 1.0f));
    createTextureImageView(tex);
    textures.push_back(std::move(tex));
  }
  createTexDscSetLayout();
  createTexDscPool();
  createTexDscSet();
}

void fe_TextureManager::createTextureSampler(fe_Texture& tex) {
  vk::PhysicalDeviceProperties properties = ctx.physicalDevice.getProperties();
  vk::SamplerCreateInfo samplerInfo{
      .magFilter = vk::Filter::eLinear,
      .minFilter = vk::Filter::eLinear,
      .mipmapMode = vk::SamplerMipmapMode::eLinear,
      .addressModeU = vk::SamplerAddressMode::eRepeat,
      .addressModeV = vk::SamplerAddressMode::eRepeat,
      .addressModeW = vk::SamplerAddressMode::eRepeat,
      .mipLodBias = 0.0f,
      .anisotropyEnable = vk::False,
      .compareEnable = vk::False,
      .compareOp = vk::CompareOp::eAlways};
  tex.sampler = vk::raii::Sampler(ctx.device, samplerInfo);
}

void fe_TextureManager::createTextureImageView(fe_Texture& tex) {
  vk::ImageViewCreateInfo createInfo = {
      .image = tex.image,
      .viewType = vk::ImageViewType::e2D,
      .format = vk::Format::eR8G8B8A8Srgb,
      .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}

  };

  tex.view = ctx.device.createImageView(createInfo);
}

void fe_TextureManager::createTextureImage(fe_Texture& tex,
                                           std::string path,
                                           glm::vec3 defaultColor) {
  int texWidth, texHeight, texChannels;
  stbi_uc* pixels;
  stbi_uc fallbackPixels[4] = {static_cast<stbi_uc>(defaultColor.r * 255),
                               static_cast<stbi_uc>(defaultColor.g * 255),
                               static_cast<stbi_uc>(defaultColor.b * 255), 255};

  if (!path.empty()) {
    pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels,
                       STBI_rgb_alpha);
  } else {
    texWidth = 1;
    texHeight = 1;
    pixels = fallbackPixels;  // Point to local array
  }

  if (!pixels) {
    throw std::runtime_error("failed to load texture image!");
  }
  vk::DeviceSize imageSize = texWidth * texHeight * 4;

  vk::raii::Buffer stagingBuffer = nullptr;
  vk::raii::DeviceMemory stagingBufferMemory = nullptr;
  ctx.createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc,
                   vk::MemoryPropertyFlagBits::eHostVisible |
                       vk::MemoryPropertyFlagBits::eHostCoherent,
                   {}, stagingBuffer, stagingBufferMemory);

  void* data = stagingBufferMemory.mapMemory(0, imageSize);
  memcpy(data, pixels, imageSize);
  stagingBufferMemory.unmapMemory();
  if (!path.empty() && pixels) {
    stbi_image_free(pixels);
  }

  ctx.createImage(
      texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
      vk::MemoryPropertyFlagBits::eDeviceLocal, tex.image, tex.memory, 1);

  std::unique_ptr<vk::raii::CommandBuffer> cmd = tim.beginSingleTimeCommands();

  ctx.transitionImageLayout(*cmd, tex.image, vk::ImageLayout::eUndefined,
                            vk::ImageLayout::eTransferDstOptimal, {},
                            vk::AccessFlagBits2::eTransferWrite,
                            vk::PipelineStageFlagBits2::eTopOfPipe,
                            vk::PipelineStageFlagBits2::eTransfer,
                            vk::ImageAspectFlagBits::eColor);
  copyBufferToImage(*cmd, stagingBuffer, tex.image,
                    static_cast<uint32_t>(texWidth),
                    static_cast<uint32_t>(texHeight));
  ctx.transitionImageLayout(
      *cmd, tex.image, vk::ImageLayout::eTransferDstOptimal,
      vk::ImageLayout::eShaderReadOnlyOptimal,
      vk::AccessFlagBits2::eTransferWrite, vk::AccessFlagBits2::eShaderRead,
      vk::PipelineStageFlagBits2::eTransfer,
      vk::PipelineStageFlagBits2::eFragmentShader,
      vk::ImageAspectFlagBits::eColor);
  tim.endSingleTimeCommands(*cmd);

  ctx.device.waitIdle();
}

void fe_TextureManager::createTexDscSetLayout() {
  vk::DescriptorSetLayoutBinding binding{
      .binding = 0,
      .descriptorType = vk::DescriptorType::eCombinedImageSampler,
      .descriptorCount = MAX_TEXTURES,
      .stageFlags = vk::ShaderStageFlagBits::eFragment

  };

  vk::DescriptorBindingFlags bindingFlags = {
      vk::DescriptorBindingFlagBits::ePartiallyBound |
      vk::DescriptorBindingFlagBits::eVariableDescriptorCount |
      vk::DescriptorBindingFlagBits::eUpdateAfterBind};

  vk::DescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{
      .bindingCount = 1, .pBindingFlags = &bindingFlags};

  vk::DescriptorSetLayoutCreateInfo layoutInfo{
      .pNext = &flagsInfo,
      .flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
      .bindingCount = 1,
      .pBindings = &binding};

  texSetLayout = vk::raii::DescriptorSetLayout(ctx.device, layoutInfo);
}

void fe_TextureManager::createTexDscPool() {
  vk::DescriptorPoolSize poolSize{

      .type = vk::DescriptorType::eCombinedImageSampler,
      .descriptorCount = MAX_TEXTURES

  };

  vk::DescriptorPoolCreateInfo poolInfo{
      .flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind |
               vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets = 1,
      .poolSizeCount = 1,
      .pPoolSizes = &poolSize

  };

  texDscPool = vk::raii::DescriptorPool(ctx.device, poolInfo);
}

void fe_TextureManager::addTextureFromColor(glm::vec3 color, std::string name) {
  fe_Texture tex = {.name = name};
  createTextureSampler(tex);
  createTextureImage(tex, "", color);
  createTextureImageView(tex);
  updateTexDscSet(tex);
  textures.push_back(std::move(tex));
}

void fe_TextureManager::updateTexDscSet(fe_Texture& tex) {
  vk::DescriptorImageInfo imageInfo = {
      .sampler = tex.sampler,
      .imageView = tex.view,
      .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};

  vk::WriteDescriptorSet write = {
      .dstSet = texDscSet,
      .dstBinding = 0,
      .dstArrayElement = static_cast<uint32_t>(textures.size()),
      .descriptorCount = 1,
      .descriptorType = vk::DescriptorType::eCombinedImageSampler,
      .pImageInfo = &imageInfo};

  ctx.device.updateDescriptorSets(write, nullptr);
}

uint32_t fe_TextureManager::getTextureIndex(std::string str) {
  for (int i = 0; i < textures.size(); i++) {
    if (textures[i].name == str)
      return i;
  }
  throw std::runtime_error("ERROR - Requested texture \"" + str +
                           "\" not found!");
}

void fe_TextureManager::createTexDscSet() {
  uint32_t count = MAX_TEXTURES;
  vk::DescriptorSetVariableDescriptorCountAllocateInfo variableCount{
      .descriptorSetCount = 1, .pDescriptorCounts = &count};

  vk::DescriptorSetAllocateInfo allocInfo{.pNext = &variableCount,
                                          .descriptorPool = *texDscPool,
                                          .descriptorSetCount = 1,
                                          .pSetLayouts = &*texSetLayout};

  texDscSet = (*ctx.device).allocateDescriptorSets(allocInfo).front();

  std::vector<vk::DescriptorImageInfo> imageInfos;
  std::vector<vk::WriteDescriptorSet> writes;
  imageInfos.reserve(textures.size());
  writes.reserve(textures.size());
  for (int i = 0; i < textures.size(); i++) {
    vk::DescriptorImageInfo imageInfo = {
        .sampler = textures[i].sampler,
        .imageView = textures[i].view,
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};

    imageInfos.push_back(imageInfo);

    writes.push_back(
        {.dstSet = texDscSet,
         .dstBinding = 0,
         .dstArrayElement = static_cast<uint32_t>(i),
         .descriptorCount = 1,
         .descriptorType = vk::DescriptorType::eCombinedImageSampler,
         .pImageInfo = &imageInfos[i]});
  }
  ctx.device.updateDescriptorSets(writes, nullptr);
}

void fe_TextureManager::copyBufferToImage(vk::raii::CommandBuffer& cmd,
                                          const vk::raii::Buffer& buffer,
                                          vk::raii::Image& image,
                                          uint32_t width,
                                          uint32_t height) {
  vk::BufferImageCopy region{
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
      .imageOffset = {0, 0, 0},
      .imageExtent = {width, height, 1}};
  cmd.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal,
                        region);
}
