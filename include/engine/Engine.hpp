#pragma once

#include <memory>
class fe_Window;
class fe_VulkanContext;

class fe_Engine {
 public:
  void init();

 private:
  std::unique_ptr<fe_Window> win;
  std::unique_ptr<fe_VulkanContext> ctx;
};
