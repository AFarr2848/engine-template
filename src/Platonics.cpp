#include "engine/Shapes.hpp"
void fe_Icosphere::subdivide(int divisions) {
  for (int d = 0; d < divisions; d++) {
    std::unordered_map<fe_EdgePair, uint32_t> midpointCache;
    uint32_t size = indices.size();

    for (int j = 0; j < size; j += 3) {
      uint32_t i1 = indices[j];
      uint32_t i2 = indices[j + 1];
      uint32_t i3 = indices[j + 2];
      // between i1 and i2
      if (!midpointCache.contains(fe_EdgePair(i1, i2))) {
        vertices.push_back(
            {.pos = midpoint(vertices[i1].pos, vertices[i2].pos)});
        midpointCache.insert(
            {fe_EdgePair(i1, i2), static_cast<uint32_t>(vertices.size() - 1)});
      }

      // i1 and i3
      if (!midpointCache.contains(fe_EdgePair(i1, i3))) {
        vertices.push_back(
            {.pos = midpoint(vertices[i1].pos, vertices[i3].pos)});
        midpointCache.insert(
            {fe_EdgePair(i1, i3), static_cast<uint32_t>(vertices.size() - 1)});
      }
      // i2 and i3
      if (!midpointCache.contains(fe_EdgePair(i2, i3))) {
        vertices.push_back(
            {.pos = midpoint(vertices[i2].pos, vertices[i3].pos)});
        midpointCache.insert(
            {fe_EdgePair(i2, i3), static_cast<uint32_t>(vertices.size() - 1)});
      }

      indices.push_back(i1);
      indices.push_back(midpointCache.at(fe_EdgePair(i1, i2)));
      indices.push_back(midpointCache.at(fe_EdgePair(i1, i3)));

      indices.push_back(i2);
      indices.push_back(midpointCache.at(fe_EdgePair(i2, i3)));
      indices.push_back(midpointCache.at(fe_EdgePair(i1, i2)));

      indices.push_back(i3);
      indices.push_back(midpointCache.at(fe_EdgePair(i1, i3)));
      indices.push_back(midpointCache.at(fe_EdgePair(i2, i3)));

      indices[j] = midpointCache.at(fe_EdgePair(i1, i2));
      indices[j + 1] = midpointCache.at(fe_EdgePair(i2, i3));
      indices[j + 2] = midpointCache.at(fe_EdgePair(i1, i3));
    }
  }
  // project to unit sphere
  for (fe_Vertex& v : vertices) {
    v.pos = glm::normalize(v.pos);
  }
}

void fe_Icosphere::makeNormals() {
  for (fe_Vertex& v : vertices) {
    v.normal = normalize(v.pos);
  }
}

void fe_Icosphere::makeTexCoords() {
  float pi = std::numbers::pi;
  for (fe_Vertex& v : vertices) {
    v.uv = {atan2(v.pos.x, v.pos.z) / (2 * pi) + 0.5,
            -asin(v.pos.y) / pi + 0.5};
  }
}

void fe_Icosahedron::makeVerticesAndIndices() {
  const float PHI = std::numbers::phi;

  glm::vec3 positions[12] = {
      {-1, PHI, 0}, {1, PHI, 0}, {-1, -PHI, 0}, {1, -PHI, 0},
      {0, -1, PHI}, {0, 1, PHI}, {0, -1, -PHI}, {0, 1, -PHI},
      {PHI, 0, -1}, {PHI, 0, 1}, {-PHI, 0, -1}, {-PHI, 0, 1},
  };

  int faces[20][3] = {
      {0, 11, 5}, {0, 5, 1},  {0, 1, 7},   {0, 7, 10}, {0, 10, 11},
      {1, 5, 9},  {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
      {3, 9, 4},  {3, 4, 2},  {3, 2, 6},   {3, 6, 8},  {3, 8, 9},
      {4, 9, 5},  {2, 4, 11}, {6, 2, 10},  {8, 6, 7},  {9, 8, 1},
  };

  vertices.clear();
  indices.clear();

  for (int i = 0; i < 20; i++) {
    // Normalize positions to create a unit sphere surface
    glm::vec3 p0 = normalize(positions[faces[i][0]]);
    glm::vec3 p1 = normalize(positions[faces[i][1]]);
    glm::vec3 p2 = normalize(positions[faces[i][2]]);

    // Calculate normal pointing outwards[cite: 1]
    // Using p1-p0 and p2-p0 with correct cross order for CCW
    glm::vec3 normal = normalize(cross(p1 - p0, p2 - p0));

    // Simple planar UVs per triangle[cite: 1]
    glm::vec2 uv0 = {0.0f, 0.0f};
    glm::vec2 uv1 = {1.0f, 0.0f};
    glm::vec2 uv2 = {0.5f, 1.0f};

    uint32_t baseIndex = static_cast<uint32_t>(vertices.size());

    vertices.push_back({p0, normal, uv0});
    vertices.push_back({p1, normal, uv1});
    vertices.push_back({p2, normal, uv2});

    indices.push_back(baseIndex + 0);
    indices.push_back(baseIndex + 1);
    indices.push_back(baseIndex + 2);
  }
}
