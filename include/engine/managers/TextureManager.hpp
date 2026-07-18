#pragma once
#include "engine/Structs.hpp"

class TextureManager {
 public:
  TextureManager() {};

  uint32_t addTexture(fe_Texture texture);
  uint32_t addTexture(fe_Texture texture);

 private:
  std::vector<fe_Texture> textures;
};
