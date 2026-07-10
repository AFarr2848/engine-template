#include <cstdint>
#include <glm/glm.hpp>

struct Vertex {
  alignas(16) glm::vec3 pos;
  alignas(16) glm::vec3 normal;
  alignas(16) glm::vec2 uv;
};

struct PushConstants {
  uint64_t vertBufAddress;
};
