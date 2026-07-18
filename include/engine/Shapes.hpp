#pragma once
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <sys/types.h>
#include <assimp/Importer.hpp>
#include <glm/common.hpp>
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <numbers>
#include <vector>
#include "engine/Structs.hpp"

static glm::vec3 midpoint(glm::vec3 vec1, glm::vec3 vec2) {
  glm::vec3 out = glm::vec3(0.0f);
  out.x = (vec1.x + vec2.x) / 2;
  out.y = (vec1.y + vec2.y) / 2;
  out.z = (vec1.z + vec2.z) / 2;
  return out;
}

class fe_Shape {
 public:
  std::vector<fe_Vertex> vertices = std::vector<fe_Vertex>();
  std::vector<uint32_t> indices = std::vector<uint32_t>();
  uint32_t transformIndex;

  glm::vec3 pos = glm::vec3(0.0f);
  glm::vec3 rotation = glm::vec3(0.0f);
  glm::vec3 scale = glm::vec3(1.0f);

  glm::vec3 vel = glm::vec3(0.0f);
  glm::vec3 rotVel = glm::vec3(0.0f);
  glm::vec3 scaleVel = glm::vec3(0.0f);
};

class fe_Mesh : public fe_Shape {
 public:
  fe_Mesh(std::string modelPath) {
    this->modelPath = modelPath;
    loadModel();
  }

 private:
  std::string modelPath;
  void loadModel();

  void processMesh(aiMesh* mesh,
                   const aiScene* scene,
                   const glm::mat4 parentTransform);
  void processNode(aiNode* node, const aiScene* scene, glm::mat4 transform);
  glm::mat4 assimpToGlm(aiMatrix4x4 a);
};

class fe_Icosahedron : public fe_Shape {
 public:
  fe_Icosahedron() { makeVerticesAndIndices(); }

 private:
  void makeVerticesAndIndices();
};

class fe_Icosphere : public fe_Shape {
 public:
  fe_Icosphere(int divisions) {
    float phi = std::numbers::phi;
    float a = 1.0f;
    float b = 1.0f / phi;
    vertices = {
        {{0, b, -a}, {0, 0, 0}, {0, 0}},  {{b, a, 0}, {0, 0, 0}, {0, 0}},
        {{-b, a, 0}, {0, 0, 0}, {0, 0}},  {{0, b, a}, {0, 0, 0}, {0, 0}},
        {{0, -b, a}, {0, 0, 0}, {0, 0}},  {{-a, 0, b}, {0, 0, 0}, {0, 0}},
        {{0, -b, -a}, {0, 0, 0}, {0, 0}}, {{a, 0, -b}, {0, 0, 0}, {0, 0}},
        {{a, 0, b}, {0, 0, 0}, {0, 0}},   {{-a, 0, -b}, {0, 0, 0}, {0, 0}},
        {{b, -a, 0}, {0, 0, 0}, {0, 0}},  {{-b, -a, 0}, {0, 0, 0}, {0, 0}},

    };

    indices = {// Top cap
               0, 2, 1, 0, 9, 2, 0, 6, 9, 0, 7, 6, 0, 1, 7,

               // Upper middle band
               1, 8, 7, 1, 3, 8, 1, 2, 3, 2, 5, 3, 2, 9, 5,

               // Lower middle band
               9, 11, 5, 9, 6, 11, 6, 10, 11, 6, 7, 10, 7, 8, 10,

               // Bottom cap
               3, 4, 8, 3, 5, 4, 5, 11, 4, 11, 10, 4, 10, 8, 4};

    subdivide(divisions);
    makeNormals();
    makeTexCoords();
  }
  void subdivide(int);
  void makeNormals();
  void makeTexCoords();
};

class fe_Cube : public fe_Shape {
 public:
  fe_Cube() {
    vertices = {
        // Front face (Z+)
        {{-0.5f, -0.5f, 0.5f}, {0, 0, 1}, {0, 0}},
        {{0.5f, -0.5f, 0.5f}, {0, 0, 1}, {1, 0}},
        {{0.5f, 0.5f, 0.5f}, {0, 0, 1}, {1, 1}},
        {{-0.5f, 0.5f, 0.5f}, {0, 0, 1}, {0, 1}},

        // Back face (Z-)
        {{0.5f, -0.5f, -0.5f}, {0, 0, -1}, {0, 0}},
        {{-0.5f, -0.5f, -0.5f}, {0, 0, -1}, {1, 0}},
        {{-0.5f, 0.5f, -0.5f}, {0, 0, -1}, {1, 1}},
        {{0.5f, 0.5f, -0.5f}, {0, 0, -1}, {0, 1}},

        // Left face (X-)
        {{-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0, 0}},
        {{-0.5f, -0.5f, 0.5f}, {-1, 0, 0}, {1, 0}},
        {{-0.5f, 0.5f, 0.5f}, {-1, 0, 0}, {1, 1}},
        {{-0.5f, 0.5f, -0.5f}, {-1, 0, 0}, {0, 1}},

        // Right face (X+)
        {{0.5f, -0.5f, 0.5f}, {1, 0, 0}, {0, 0}},
        {{0.5f, -0.5f, -0.5f}, {1, 0, 0}, {1, 0}},
        {{0.5f, 0.5f, -0.5f}, {1, 0, 0}, {1, 1}},
        {{0.5f, 0.5f, 0.5f}, {1, 0, 0}, {0, 1}},

        // Top face (Y+)
        {{-0.5f, 0.5f, 0.5f}, {0, 1, 0}, {0, 0}},
        {{0.5f, 0.5f, 0.5f}, {0, 1, 0}, {1, 0}},
        {{0.5f, 0.5f, -0.5f}, {0, 1, 0}, {1, 1}},
        {{-0.5f, 0.5f, -0.5f}, {0, 1, 0}, {0, 1}},

        // Bottom face (Y-)
        {{-0.5f, -0.5f, -0.5f}, {0, -1, 0}, {0, 0}},
        {{0.5f, -0.5f, -0.5f}, {0, -1, 0}, {1, 0}},
        {{0.5f, -0.5f, 0.5f}, {0, -1, 0}, {1, 1}},
        {{-0.5f, -0.5f, 0.5f}, {0, -1, 0}, {0, 1}},
    };

    indices = {// Front
               0, 1, 2, 0, 2, 3,
               // Back
               4, 5, 6, 4, 6, 7,
               // Left
               8, 9, 10, 8, 10, 11,
               // Right
               12, 13, 14, 12, 14, 15,
               // Top
               16, 17, 18, 16, 18, 19,
               // Bottom
               20, 21, 22, 20, 22, 23

    };
    /*
    for (Vertex &v : vertices) {
      glm::vec4 p = transform * glm::vec4(v.pos, 1.0);
      v.pos = p;
    }
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));

    for (Vertex &v : vertices) {
      v.normal = glm::normalize(normalMatrix * v.normal);
    }
    */
  };
};

// for icosphere creation
class fe_EdgePair {
 public:
  uint32_t i1;
  uint32_t i2;

  bool operator==(const fe_EdgePair& other) const {
    return (i1 == other.i1 && i2 == other.i2) ||
           (i1 == other.i2 && i2 == other.i1);
  }
};

namespace std {
template <>
struct hash<fe_EdgePair> {
  size_t operator()(const fe_EdgePair& ep) const {
    size_t h1 = hash<uint32_t>{}(ep.i1);
    size_t h2 = hash<uint32_t>{}(ep.i2);
    return h1 ^ (h2 << 1);
  }
};
}  // namespace std
