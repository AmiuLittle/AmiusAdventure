#pragma once
#ifndef PC_RENDER_PRIMITIVES
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <array>
#include <vector>

struct Vertex {
    glm::vec3 pos;
    glm::vec2 texCoord;
    glm::vec3 norm;

    static constexpr VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindDesc {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };

        return bindDesc;
    }

    static constexpr std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attrDescs {}; 
        attrDescs[0] = { // position
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, pos)
        };
        attrDescs[1] = { // UV coords
            .location = 1,
            .binding = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(Vertex, texCoord)
        };
        attrDescs[2] = { // normals
            .location = 2,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, norm)
        };
        return attrDescs;
    }
};

enum PrimitiveType {
    PRIMITIVE_TYPE_INDEXED,
    PRIMITIVE_TYPE_VERTEXES_ONLY
};

class Primitive {
public:
    PrimitiveType type;
    uint32_t vertexDataOffset;
    uint32_t vertexDataSize;
    uint32_t firstVertex;
    uint32_t vertexCount;
    uint32_t indexDataOffset;
    uint32_t indexDataSize;
    uint32_t firstIndex;
    uint32_t indexCount;
    Primitive(PrimitiveType type);
    ~Primitive();
};

struct Mesh {
    std::vector<Primitive> primitives;
};

struct ModelData {
    std::vector<Mesh> meshes;
};

#define PC_RENDER_PRIMITIVES
#endif // PC_RENDER_PRIMITIVES