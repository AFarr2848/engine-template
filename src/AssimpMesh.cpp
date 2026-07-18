#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <sys/types.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <iostream>
#include "engine/Shapes.hpp"

void fe_Mesh::loadModel() {
  Assimp::Importer import;
  const aiScene* scene =
      import.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_FlipUVs);
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
    return;
  }

  processNode(scene->mRootNode, scene, glm::mat4(1.0f));
}

void fe_Mesh::processNode(aiNode* node,
                          const aiScene* scene,
                          glm::mat4 parentTransform) {
  glm::mat4 nodeTransform =
      parentTransform * assimpToGlm(node->mTransformation);

  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh* m = scene->mMeshes[node->mMeshes[i]];
    processMesh(m, scene, nodeTransform);
  }

  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene, nodeTransform);
  }
}

void fe_Mesh::processMesh(aiMesh* mesh,
                          const aiScene* scene,
                          const glm::mat4 transform) {
  uint32_t vertexOffset = static_cast<uint32_t>(vertices.size());

  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex;
    glm::vec4 pos =
        transform * glm::vec4(mesh->mVertices[i].x, mesh->mVertices[i].y,
                              mesh->mVertices[i].z, 1.0f);
    vertex.pos = pos;

    if (mesh->HasNormals()) {
      glm::vec3 normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y,
                                   mesh->mNormals[i].z);
      vertex.normal = glm::normalize(glm::mat3(transform) * normal);
    }
    if (mesh->mTextureCoords[0]) {
      glm::vec2 vec = {
          mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y

      };

      vertex.uv = vec;

      /*
      // tangent
      vector.x = mesh->mTangents[i].x;
      vector.y = mesh->mTangents[i].y;
      vector.z = mesh->mTangents[i].z;
      vertex.Tangent = vector;
      // bitangent
      vector.x = mesh->mBitangents[i].x;
      vector.y = mesh->mBitangents[i].y;
      vector.z = mesh->mBitangents[i].z;
      vertex.Bitangent = vector;
      */
    } else
      vertex.uv = glm::vec2(0.0f, 0.0f);

    vertices.push_back(vertex);
  }

  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++)
      indices.push_back(face.mIndices[j] + vertexOffset);
  }
  /*
  // process materials
  aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
  // we assume a convention for sampler names in the shaders. Each diffuse
  texture should be named
  // as 'texture_diffuseN' where N is a sequential number ranging from 1 to
  MAX_SAMPLER_NUMBER.
  // Same applies to other texture as the following list summarizes:
  // diffuse: texture_diffuseN
  // specular: texture_specularN
  // normal: texture_normalN

  // 1. diffuse maps
  vector<Texture> diffuseMaps = loadMaterialTextures(material,
  aiTextureType_DIFFUSE, "texture_diffuse"); textures.insert(textures.end(),
  diffuseMaps.begin(), diffuseMaps.end());
  // 2. specular maps
  vector<Texture> specularMaps = loadMaterialTextures(material,
  aiTextureType_SPECULAR, "texture_specular"); textures.insert(textures.end(),
  specularMaps.begin(), specularMaps.end());
  // 3. normal maps
  std::vector<Texture> normalMaps = loadMaterialTextures(material,
  aiTextureType_HEIGHT, "texture_normal"); textures.insert(textures.end(),
  normalMaps.begin(), normalMaps.end());
  // 4. height maps
  std::vector<Texture> heightMaps = loadMaterialTextures(material,
  aiTextureType_AMBIENT, "texture_height"); textures.insert(textures.end(),
  heightMaps.begin(), heightMaps.end());

  */
}

glm::mat4 fe_Mesh::assimpToGlm(aiMatrix4x4 a) {
  glm::mat4 glm = glm::mat4(
      glm::vec4(a.a1, a.b1, a.c1, a.d1), glm::vec4(a.a2, a.b2, a.c2, a.d2),
      glm::vec4(a.a3, a.b3, a.c3, a.d3), glm::vec4(a.a4, a.b4, a.c4, a.d4));
  return glm;
}
