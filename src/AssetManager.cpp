#include "engine/AssetManager.hpp"
#include <fstream>
#include <vulkan/vulkan_raii.hpp>
#include "engine/VulkanContext.hpp"
#include "vulkan/vulkan.hpp"

void fe_AssetManager::loadShaderModule(const std::string& name,
                                       const std::string& filePath,
                                       vk::ShaderStageFlagBits stage) {
  std::ifstream file(filePath, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open shader file: " + filePath);
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
  file.seekg(0);
  file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
  file.close();

  shaderRegistry.emplace(
      name, ctx.device.createShaderEXT(
                {.stage = stage,
                 .nextStage = (stage == vk::ShaderStageFlagBits::eVertex
                                   ? vk::ShaderStageFlagBits::eFragment
                                   : vk::ShaderStageFlags{}),
                 .codeType = vk::ShaderCodeTypeEXT::eSpirv,
                 .codeSize = buffer.size() * sizeof(uint32_t),
                 .pCode = buffer.data(),
                 .pName = (stage == vk::ShaderStageFlagBits::eVertex
                               ? "vertexMain"
                               : "fragmentMain")}));
}

vk::ShaderEXT fe_AssetManager::getShader(const std::string& name) {
  return shaderRegistry.at(name);
}
