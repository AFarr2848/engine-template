#pragma once
#include <vector>

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;
constexpr int MAX_FRAMES_IN_FLIGHT = 2;
constexpr int MAX_LIGHTS = 64;
constexpr int MAX_POINT_LIGHTS = 32;
constexpr int MAX_DIR_LIGHTS = 16;
constexpr int MAX_SPOT_LIGHTS = 16;
constexpr int MAX_TRANSFORMS = 1024;
constexpr int MAX_TEXTURES = 1024;

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

inline const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const char*> requiredDeviceExtension = {
    vk::KHRSwapchainExtensionName, vk::KHRShaderDrawParametersExtensionName,
    vk::KHRSpirv14ExtensionName, vk::KHRSynchronization2ExtensionName,
    vk::KHRCreateRenderpass2ExtensionName

};
