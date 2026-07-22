#pragma once

#include <cstdint>
#include "engine/Structs.hpp"
class fe_VulkanContext;
class fe_TimingData;

class fe_TextureManager {
 public:
  fe_TextureManager(fe_VulkanContext& ctx, fe_TimingData& tim)
      : ctx(ctx), tim(tim) {};

  void addTextureFromColor(glm::vec3 color, std::string name);
  void loadTextures();

  uint32_t getTextureIndex(std::string str);

  vk::DescriptorSet texDscSet = nullptr;
  vk::raii::DescriptorSetLayout texSetLayout = nullptr;
  vk::raii::DescriptorPool texDscPool = nullptr;

 private:
  fe_VulkanContext& ctx;
  fe_TimingData& tim;

  std::vector<fe_Texture> textures;

  void createTexDscSetLayout();
  void createTexDscPool();
  void createTexDscSet();

  /**
   * @brief Adds a new texture to the descriptor set. The new texture must be
   * the last one in the textures map.
   *
   * @param tex The texture to add
   */
  void updateTexDscSet(fe_Texture& tex);
  void createTextureSampler(fe_Texture& tex);
  void createTextureImageView(fe_Texture& tex);
  void createTextureImage(fe_Texture& tex,
                          std::string path,
                          glm::vec3 defaultColor);

  void copyBufferToImage(vk::raii::CommandBuffer& cmd,
                         const vk::raii::Buffer& buffer,
                         vk::raii::Image& image,
                         uint32_t width,
                         uint32_t height);
};
