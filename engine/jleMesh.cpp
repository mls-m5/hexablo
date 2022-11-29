// Copyright (c) 2022. Johan Lind

#include "jleMesh.h"
#include "plog/Log.h"
#include "tiny_obj_loader.h"
#include <stdio.h>

#include "jleIncludeGL.h"

bool
jleMesh::loadFromFile(const std::string &path)
{
    return loadFromObj(path);
}

bool
jleMesh::loadFromObj(const std::string &path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool loaded = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());

    if (!err.empty()) {
        LOGE << err;
        return false;
    }

    const auto &vertices = attrib.vertices;
    const auto &normals = attrib.normals;
    const auto &texcoords = attrib.texcoords;

    std::vector<glm::vec3> in_vertices;
    std::vector<glm::vec3> in_normals;
    std::vector<glm::vec2> in_uvs;

    for (int i = 0; i < vertices.size(); i += 3) {
        in_vertices.push_back({vertices[i], vertices[i + 1], vertices[i + 2]});
    }

    for (int i = 0; i < normals.size(); i += 3) {
        in_normals.push_back({normals[i], normals[i + 1], normals[i + 2]});
    }

    for (int i = 0; i < texcoords.size(); i += 2) {
        in_uvs.push_back({texcoords[i], texcoords[i + 1]});
    }

    // If the OBJ file uses indexing for all attributes, then we simply go thru the
    // mesh's indicies, and then find the attributes in the correct order, and then
    // create new attribute arrays. We thus, do not use indexed rendering.
    if (shapes[0].mesh.indices[0].normal_index >= 0) {
        std::vector<glm::vec3> out_vertices;
        std::vector<glm::vec2> out_uvs;
        std::vector<glm::vec3> out_normals;

        for (auto &&shape : shapes) {
            for (unsigned int i = 0; i < shape.mesh.indices.size(); i++) {

                // Get the indices of its attributes
                int vertexIndex = shape.mesh.indices[i].vertex_index;
                if (vertexIndex != -1) {
                    glm::vec3 vertex = in_vertices[vertexIndex];
                    out_vertices.push_back(vertex);
                }

                int uvIndex = shape.mesh.indices[i].texcoord_index;
                if (uvIndex != -1) {
                    glm::vec2 uv = in_uvs[uvIndex];
                    out_uvs.push_back(uv);
                }

                int normalIndex = shape.mesh.indices[i].normal_index;
                if (normalIndex != -1) {
                    glm::vec3 normal = in_normals[normalIndex];
                    out_normals.push_back(normal);
                }
            }
        }

        makeMesh(out_vertices, out_normals, out_uvs, {});
        return true;

    } else { // Else we can use the vertex indices as they are, and thus use indexed rendering
        std::vector<unsigned int> vertexIndicies;
        for (auto &&shape : shapes) {
            for (int i = 0; i < shape.mesh.indices.size(); i++) {
                vertexIndicies.push_back(shape.mesh.indices[i].vertex_index);
            }
        }

        makeMesh(in_vertices, in_normals, in_uvs, vertexIndicies);
        return true;
    }
}

unsigned int
jleMesh::getVAO()
{
    return _vao;
}
unsigned int
jleMesh::getTrianglesCount()
{
    return _trianglesCount;
}

void
jleMesh::makeMesh(const std::vector<glm::vec3> &positions,
                  const std::vector<glm::vec3> &normals,
                  const std::vector<glm::vec2> &texCoords,
                  const std::vector<unsigned int> &indices)
{

    destroyOldBuffers();

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    if (!positions.empty()) {
        glGenBuffers(1, &_vbo_pos);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo_pos);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), &positions[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
        glEnableVertexAttribArray(0);
    }

    if (!normals.empty()) {
        glGenBuffers(1, &_vbo_normal);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo_normal);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
        glEnableVertexAttribArray(1);
    }

    if (!texCoords.empty()) {
        glGenBuffers(1, &_vbo_texcoords);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo_texcoords);
        glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec2), &texCoords[0], GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
        glEnableVertexAttribArray(2);
    }

    if (!indices.empty()) {
        glGenBuffers(1, &_ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    }

    glBindVertexArray(0);

    if (indices.size() > 0) {
        _trianglesCount = indices.size();
    } else {
        _trianglesCount = positions.size();
    }
}

jleMesh::~jleMesh() { destroyOldBuffers(); }

void
jleMesh::destroyOldBuffers()
{
    if (_vbo_pos) {
        glDeleteBuffers(1, &_vbo_pos);
        _vbo_pos = 0;
    }
    if (_vbo_normal) {
        glDeleteBuffers(1, &_vbo_normal);
        _vbo_normal = 0;
    }
    if (_vbo_texcoords) {
        glDeleteBuffers(1, &_vbo_texcoords);
        _vbo_texcoords = 0;
    }
    if (_ebo) {
        glDeleteBuffers(1, &_ebo);
        _ebo = 0;
    }
    if (_vao) {
        glDeleteVertexArrays(1, &_vao);
        _vao = 0;
    }
}
bool
jleMesh::usesIndexing()
{
    return _ebo > 0;
}