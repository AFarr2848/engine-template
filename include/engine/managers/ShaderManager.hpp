#pragma once
#include <unordered_map>
class fe_VulkanContext;
class fe_TimingData;

class fe_ShaderManager {
 public:
  fe_ShaderManager(fe_VulkanContext& ctx) : ctx(ctx) {}

  /**
   * @brief Loads a compiled shader and puts it in the shader registry
   *
   * @param name The name of the new shader module
   * @param filePath The path to the compiled shader
   * @param stage The shader's stage
   */
  void loadShaderModule(const std::string& name,
                        const std::string& filePath,
                        vk::ShaderStageFlagBits stage);

  vk::ShaderEXT getShader(const std::string& name);

 private:
  fe_VulkanContext& ctx;
  std::unordered_map<std::string, vk::raii::ShaderEXT> shaderRegistry;
};
