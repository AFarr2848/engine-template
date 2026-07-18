#include "engine/managers/ShaderManager.hpp"
#include <fstream>
#include "engine/Structs.hpp"
#include "engine/Timing.hpp"
#include "engine/VulkanContext.hpp"
#include "vulkan/vulkan.hpp"

void fe_ShaderManager::loadShaderModule(const std::string& name,
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

  // prolly bad to have 2 of these
  vk::PushConstantRange pcRange = {
      .stageFlags =
          vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
      .offset = 0,
      .size = sizeof(fe_PushConstants)

  };

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
                               : "fragmentMain"),
                 .pushConstantRangeCount = 1,
                 .pPushConstantRanges = &pcRange}));
}

vk::ShaderEXT fe_ShaderManager::getShader(const std::string& name) {
  return shaderRegistry.at(name);
}
