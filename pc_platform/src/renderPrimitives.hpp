#pragma once
#ifndef PC_RENDER_PRIMITIVES
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <array>

struct Vertex {
    glm::vec2 pos;
    glm::vec3 vertexColor;

    static constexpr VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindDesc {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };

        return bindDesc;
    }

    static constexpr std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attrDescs {}; 
        attrDescs[0] = {
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(Vertex, pos)
        };
        attrDescs[1] = {
            .location = 1,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, vertexColor)
        };
        return attrDescs;
    }
};

#define PC_RENDER_PRIMITIVES
#endif // PC_RENDER_PRIMITIVES